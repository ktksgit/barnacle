#ifndef POLYHOOK_2_0_PYCALLBACK_HPP
#define POLYHOOK_2_0_PYCALLBACK_HPP

#include "ILCallback.hpp"

#include <iostream>
#include <vector>

namespace PLH
{

    class PyCallback : public ILCallback
    {
    public:
        using tUserCallback = void (*)(const uint32_t unique_id, const Parameters *params, const uint8_t count, const ReturnValue *ret);

        PyCallback() = default;
        ~PyCallback() = default;

        uint64_t getJitFunc(const uint32_t unique_id, const std::string &retType,
                            const std::vector<std::string> &paramTypes, const tUserCallback callback,
                            std::string callConv /* = ""*/);

        // Construct a callback given the typedef as a string. Types are any valid C/C++ data type (basic types), and pointers to
        // anything are just a uintptr_t. Calling convention is defaulted to whatever is typical for the compiler you use, you can override with
        // stdcall, fastcall, or cdecl (cdecl is default on x86). On x64 those map to the same thing.
        uint64_t getJitFunc(const uint32_t unique_id, const asmjit::FuncSignature &sig, const asmjit::Environment::Arch arch,
                            const tUserCallback callback, bool use_trampoline = false);

        uint64_t getJitFunc(const uint32_t native_address, const asmjit::FuncSignature &sig, const asmjit::Environment::Arch arch);

        static asmjit::CallConv::Id getCallConv(const std::string &conv);
    };

} // namespace PLH

#endif // POLYHOOK_2_0_PYCALLBACK_HPP
