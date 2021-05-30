#include "embed.h"
#include "stl.h" // py::object -> std::vector conversion

#include <iostream>
#include <thread>

// #include "asmjit/asmjit.h"
#include "polyhook2/Detour/PyCallback.hpp"
#include "polyhook2/Detour/x86Detour.hpp"
#include "polyhook2/CapstoneDisassembler.hpp"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <Tlhelp32.h>
#include <winternl.h>

namespace py = pybind11;

namespace {

static const constexpr auto ANTI_DEBUG_THREAD_ENTRY = 0xf15cfd;

std::string getErrorAsStr(DWORD error)
{
    LPSTR msg_buffer;
    auto buffer_length = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        reinterpret_cast<LPSTR>(&msg_buffer),
        0, nullptr);
    if (buffer_length)
    {
        std::string result{msg_buffer, msg_buffer + buffer_length};

        LocalFree(msg_buffer);

        return result;
    }

    return {};
}

void killAntiDebugThread() {
    DWORD process_id = GetCurrentProcessId();
    auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id);

    if(snapshot_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    auto thread_entry = THREADENTRY32{};
    thread_entry.dwSize = sizeof(THREADENTRY32);
    auto success = Thread32First(snapshot_handle, &thread_entry);
    if (!success) {
        auto err = GetLastError();
        if (err == ERROR_NO_MORE_FILES) {
            throw std::runtime_error(getErrorAsStr(err));
        }
    }

    std::vector<DWORD> threads;
    while (true) {
        if (thread_entry.th32OwnerProcessID == process_id) {
            auto thread_id = thread_entry.th32ThreadID;
            threads.push_back(thread_id);
        }

        auto success = Thread32Next(snapshot_handle, &thread_entry);
        if (!success) {
            auto err = GetLastError();
            if (err == ERROR_NO_MORE_FILES) {
                break;
            }

            std::cout << getErrorAsStr(err) << std::endl;
        }
    }
    
    CloseHandle(snapshot_handle);

#ifndef _MSC_VER
    // ThreadQuerySetWin32StartAddress not available for each compiler
    for(auto thread_id : threads) {
        HANDLE thread_handle = OpenThread(THREAD_ALL_ACCESS, false, thread_id);
        PVOID thread_info;
        ULONG return_length;
        NtQueryInformationThread(thread_handle, ThreadQuerySetWin32StartAddress, &thread_info, sizeof(thread_info), &return_length);

        if (reinterpret_cast<int>(thread_info) == ANTI_DEBUG_THREAD_ENTRY) {
            std::cout << "Killing antidebug thread " << std::endl;
            TerminateThread(thread_handle, 0);
        }
        CloseHandle(thread_handle);
    }
#endif
}

void waitForDebuggerAndBreak() {
    while(!IsDebuggerPresent()) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
#ifdef _MSC_VER
    __debugbreak();
#else
    __asm__("int $3");
#endif
}

struct PyFunctionWrap {
    PyFunctionWrap(py::function& func) {
        _func = func;
    }

    PLH::PyCallback callback;
    py::function _func;
    std::vector<asmjit::Type::Id> _args;
    asmjit::Type::Id _return_type;
    std::shared_ptr<PLH::IHook> _detour;
};

class ConversionException : public std::exception {
    const char* const _msg = "";

public:
    ConversionException(const char* msg):
        _msg(msg)
    {}

    const char* what() const noexcept override final {
        return _msg;
    }
};

} // namespace

NOINLINE void dispatch(const uint32_t unique_id, const PLH::PyCallback::Parameters *p, const uint8_t param_count, const PLH::PyCallback::ReturnValue *retVal)
{
    PyFunctionWrap *py_func_wrap = reinterpret_cast<PyFunctionWrap *>(unique_id);
    std::string function_name;
    std::string arguments_as_str;

    try
    {
        py::gil_scoped_acquire gil{};
        py::list arguments;
        namespace Type = asmjit::Type;

        function_name = py::str(py_func_wrap->_func);

        for (uint8_t indx = 0; indx != param_count; ++indx)
        {
            uint8_t type = py_func_wrap->_args[indx];

            switch (type)
            {
            case Type::Id::kIdUIntPtr:
            {
                uintptr_t value = p->getArg<uintptr_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdF32:
            {
                float value = p->getArg<float>(indx);
                py::float_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdF64:
            {
                double value = p->getArg<double>(indx);
                py::float_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdU32:
            {
                uint32_t value = p->getArg<uint32_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdI32:
            {
                int32_t value = p->getArg<int32_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdI8:
            {
                int8_t value = p->getArg<int8_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdU8:
            {
                uint8_t value = p->getArg<uint8_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdI64:
            {
                int64_t value = p->getArg<int64_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdU64:
            {
                uint64_t value = p->getArg<uint64_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdI16:
            {
                int16_t value = p->getArg<int16_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            break;
            case Type::Id::kIdU16:
            {
                uint16_t value = p->getArg<uint16_t>(indx);
                py::int_ py_obj{value};
                arguments.append(py_obj);
            }
            default:
                throw ConversionException{"Unsupported argument type"};
            }
        }
        
        arguments_as_str = py::str(arguments);

        py::object result = py_func_wrap->_func(*arguments);

        auto ret_type = py_func_wrap->_return_type;
        if (Type::isFloat32(ret_type))
        {
            float value = py::cast<float>(result);
            *((float *)retVal->getRetPtr()) = value;
        }
        else if (Type::isFloat64(ret_type))
        {
            double value = py::cast<double>(result);
            *((double *)retVal->getRetPtr()) = value;
        }
        else if (Type::isInt(ret_type))
        {
            uint64_t unsigned_integer = py::cast<uint64_t>(result);
            int64_t signed_integer = py::cast<int64_t>(result);

            if (Type::isInt8(ret_type))
            {
                *((int8_t *)retVal->getRetPtr()) = static_cast<int8_t>(signed_integer);
            }
            else if (Type::isInt16(ret_type))
            {
                *((int16_t *)retVal->getRetPtr()) = static_cast<int16_t>(signed_integer);
            }
            else if (Type::isInt32(ret_type))
            {
                *((int32_t *)retVal->getRetPtr()) = static_cast<int32_t>(signed_integer);
            }
            else
            {
                *((uint64_t *)retVal->getRetPtr()) = unsigned_integer;
            }
        }
        else
        {
            throw ConversionException{"Unsupported return type"};
        }
    }
    catch (std::exception &exc)
    {
#ifdef _MSC_VER
        throw exc;
#else
        std::cerr << "Exception occurred during dispatch of native function to python function\n" << 
             __FILE__ << " " << __LINE__ << " " << function_name << "(" << arguments_as_str << "\n" << 
             exc.what() << std::endl;
        return;
#endif
    }
    return;
}

namespace {

class Logger : public PLH::Logger
{
public:
    void log(std::string msg, PLH::ErrorLevel level) override {
        std::cout << "[" << static_cast<uint32_t>(level) << "] " << msg << std::endl;
    }
};

struct NativeFunction
{
    asmjit::Type::Id _ret_type;
    std::string _call_conv;
    std::vector<asmjit::Type::Id> _param_types;
    uint64_t _function_address = 0;
    PLH::PyCallback _callback;

    NativeFunction(const uint32_t native_address, const asmjit::Type::Id ret_type, const std::string &call_conv, const std::vector<asmjit::Type::Id> &param_types)
    {
        _ret_type = ret_type;
        _call_conv = call_conv;
        _param_types = param_types;

        asmjit::FuncSignature sig = {};

        const size_t param_count = param_types.size();

        std::vector<uint8_t> args_types;
        for (const auto &s : param_types)
        {
            args_types.push_back(static_cast<uint8_t>(s));
        }

        sig.init(PLH::PyCallback::getCallConv(call_conv),
                 asmjit::FuncSignature::kNoVarArgs,
                 ret_type,
                 args_types.data(),
                 static_cast<uint32_t>(param_types.size()));

        _function_address = _callback.getJitFunc(native_address, sig, asmjit::Environment::kArchHost);
    }

    py::object operator()(const py::args& args)
    {
        const NativeFunction &native_function = *this;

        const size_t param_count = native_function._param_types.size();
        if (args.size() != param_count)
        {
            throw std::invalid_argument("args.size() != param_types.size()");
        }

        std::vector<uint64_t> params;
        params.reserve(param_count);

        for (size_t i = 0; i < param_count; i++)
        {
            params.push_back(0);
        }

        namespace Type = asmjit::Type;
        for (uint8_t indx = 0; indx != param_count; ++indx)
        {
            Type::Id type = native_function._param_types[indx];
            PLH::PyCallback::Parameters *param = reinterpret_cast<PLH::PyCallback::Parameters *>(params.data());

            switch (type)
            {
            case Type::Id::kIdUIntPtr:
            {
                uintptr_t value = py::cast<py::int_>(args[indx]);
                param->setArg<uintptr_t>(indx, value);
            }
            break;
            case Type::Id::kIdF32:
            {
                float value = py::cast<py::float_>(args[indx]);
                param->setArg<float>(indx, value);
            }
            break;
            case Type::Id::kIdF64:
            {
                double value = py::cast<py::float_>(args[indx]);
                param->setArg<double>(indx, value);
            }
            break;
            case Type::Id::kIdU32:
            {
                uint32_t value = py::cast<py::int_>(args[indx]);
                param->setArg<uint32_t>(indx, value);
            }
            break;
            case Type::Id::kIdI32:
            {
                int32_t value = py::cast<py::int_>(args[indx]);
                param->setArg<int32_t>(indx, value);
            }
            break;
            case Type::Id::kIdI8:
            {
                int8_t value = py::cast<py::int_>(args[indx]);
                param->setArg<int8_t>(indx, value);
            }
            break;
            case Type::Id::kIdU8:
            {
                uint8_t value = py::cast<py::int_>(args[indx]);
                param->setArg<uint8_t>(indx, value);
            }
            break;
            case Type::Id::kIdI64:
            {
                int64_t value = py::cast<py::int_>(args[indx]);
                param->setArg<int64_t>(indx, value);
            }
            break;
            case Type::Id::kIdU64:
            {
                uint64_t value = py::cast<py::int_>(args[indx]);
                param->setArg<uint64_t>(indx, value);
            }
            break;
            case Type::Id::kIdI16:
            {
                int16_t value = py::cast<py::int_>(args[indx]);
                param->setArg<int16_t>(indx, value);
            }
            break;
            case Type::Id::kIdU16:
            {
                uint16_t value = py::cast<py::int_>(args[indx]);
                param->setArg<uint16_t>(indx, value);
            }
            default:
                throw ConversionException{"Unsupported argument type"};
            }
        }

        PLH::PyCallback::ReturnValue return_value;
        return_value.m_retVal = 0;

        using Func = void (*)(PLH::PyCallback::Parameters *, PLH::PyCallback::ReturnValue *);
        Func func = reinterpret_cast<Func>(native_function._function_address);
        {
            py::gil_scoped_release gil{};
            func(reinterpret_cast<PLH::PyCallback::Parameters *>(params.data()), &return_value);
        }

        if (native_function._ret_type == Type::Id::kIdVoid) {
            return py::none{};
        }

        py::object ret;

        if (Type::isFloat32(native_function._ret_type))
        {
            float value = *((float *)return_value.getRetPtr());
            ret = py::float_{value};
        }
        else if (Type::isFloat64(native_function._ret_type))
        {
            double value = *((double *)return_value.getRetPtr());
            ret = py::float_{value};
        }
        else if (Type::isInt(native_function._ret_type))
        {
            if (Type::isInt8(native_function._ret_type))
            {
                int8_t value = *((int8_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isUInt8(native_function._ret_type))
            {
                uint8_t value = *((uint8_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isInt16(native_function._ret_type))
            {
                int16_t value = *((int16_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isUInt16(native_function._ret_type))
            {
                uint16_t value = *((uint16_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isInt32(native_function._ret_type))
            {
                int32_t value = *((int32_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isUInt32(native_function._ret_type))
            {
                uint32_t value = *((uint32_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else if (Type::isInt64(native_function._ret_type))
            {
                int64_t value = *((int64_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
            else
            {
                uint64_t value = *((uint64_t *)return_value.getRetPtr());
                ret = py::int_{value};
            }
        }
        else
        {
            throw ConversionException{"Unsupported return type"};
        }

        return ret;
    }
};

/**
 * native function
 */
std::unique_ptr<PyFunctionWrap> createNativeToPythonFunction(uint64_t native_address, asmjit::Type::Id ret_type,
                                  const std::string &call_conv, const std::vector<asmjit::Type::Id> &param_types,
                                  py::function function, bool use_trampoline)
{
    std::unique_ptr<PyFunctionWrap> py_function_wrap = std::make_unique<PyFunctionWrap>(function);

    asmjit::FuncSignature sig = {};
    py_function_wrap->_args = param_types;
    py_function_wrap->_return_type = ret_type;

    std::vector<uint8_t> args;
    for (const auto &s : param_types)
    {
        args.push_back(static_cast<uint8_t>(s));
    }

    sig.init(PLH::PyCallback::getCallConv(call_conv),
        asmjit::FuncSignature::kNoVarArgs,
        ret_type,
        args.data(),
        static_cast<uint32_t>(args.size())
    );

    uint32_t unique_id = reinterpret_cast<uint32_t>(py_function_wrap.get());
    uint64_t jit_callback = py_function_wrap->callback.getJitFunc(unique_id,
        sig, asmjit::Environment::kArchHost, &dispatch, use_trampoline);

    // TODO move to static
    PLH::CapstoneDisassembler disassembler(PLH::Mode::x86);
    std::shared_ptr<PLH::IHook> detour = std::make_shared<PLH::x86Detour>(native_address, jit_callback, py_function_wrap->callback.getTrampolineHolder(), disassembler);

    detour->hook();
    py_function_wrap->_detour = std::move(detour);

    return py_function_wrap;
}

}

PYBIND11_EMBEDDED_MODULE(_native, m)
{
    auto asmjit = m.def_submodule("asmjit");
    auto Type = asmjit.def_submodule("Type");

    using asmjit::Type::Id;
    py::enum_<Id>(Type, "Id")
        .value("kIdVoid", Id::kIdVoid)
        .value("kIdF32", Id::kIdF32)
        .value("kIdF64", Id::kIdF64)
        .value("kIdI8", Id::kIdI8)
        .value("kIdU8", Id::kIdU8)
        .value("kIdI16", Id::kIdI16)
        .value("kIdU16", Id::kIdU16)
        .value("kIdI32", Id::kIdI32)
        .value("kIdU32", Id::kIdU32)
        .value("kIdI64", Id::kIdI64)
        .value("kIdU64", Id::kIdU64)
        .value("kIdUIntPtr", Id::kIdUIntPtr);

    py::class_<PyFunctionWrap>(m, "PyFunctionWrap")
        .def("getTrampolineAddress", [](PyFunctionWrap &self) -> uint64_t {
            uint64_t *holder = self.callback.getTrampolineHolder();
            return *holder;
        });

    py::class_<NativeFunction>(m, "NativeFunction")
        .def(py::init<const uint32_t, const asmjit::Type::Id, const std::string &, const std::vector<asmjit::Type::Id> &>(),
             py::arg("native_address"), py::arg("ret_type"), py::arg("call_conv"), py::arg("param_types"))
        .def("__call__", [] (NativeFunction& this_, const py::args& args) {
            return this_(args);
        });
    m.def("killAntiDebugThread", killAntiDebugThread);
    m.def("waitForDebuggerAndBreak", waitForDebuggerAndBreak);
    m.def("createNativeToPythonFunction", createNativeToPythonFunction);

    PLH::Log::registerLogger(std::make_shared<Logger>());
}
