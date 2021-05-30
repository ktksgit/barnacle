#include "module_tcl.h"

#include "tcl_globals.h"
#include "embed.h"
#include "cpptcl.h"

#include <functional>
#include <iostream>

namespace py = pybind11;

namespace
{

enum class Type
{
    PyUnicode,
    PyLong,
    PyFloat,
    PyBool,
    PyList,
    TclPyObject,
    Autodetect,
    VAR_POSITIONAL,
};

Type detectEquivalentType(Tcl::object&);

py::object castValue(Type type, Tcl::object& tcl_obj)
{
	if (tcl_obj.get_interp() == nullptr) {
        throw Tcl::tcl_error{"castValue requires a Tcl::object with defined Tcl::interpreter, but the interpreter of Tcl::object is null"};
    }
    Tcl::interpreter interp{tcl_obj.get_interp(), false};
    
    switch (type)
    {
    case Type::PyLong:
    {
        int integer = tcl_obj.get<int>(interp);
        py::int_ o{integer};
        return o;
    }
    break;
    case Type::PyUnicode:
    {
        std::string str = tcl_obj.get<std::string>(interp);
        py::str o{str};
        return o;
    }
    break;
    case Type::PyFloat:
    {
        double value = tcl_obj.get<double>(interp);
        py::float_ o{value};
        return o;
    }
    break;
    case Type::PyBool:
    {
        bool value = tcl_obj.get<bool>(interp);
        py::bool_ o{value};
        return o;
    }
    break;
    case Type::PyList:
    {
        py::list list; 
        auto list_length = tcl_obj.size(interp);
        for(size_t i = 0; i < list_length; i++)
        {
            auto elem = tcl_obj.at(i, interp);
            elem.set_interp(interp.get());
            auto elem_type = detectEquivalentType(elem);

            py::object py_elem = castValue(elem_type, elem);
            list.append(py_elem);
        }

        return list;
    }
    break;
    case Type::TclPyObject:
    {
        Tcl_Obj* raw_tcl_obj = tcl_obj.get_object();

        if (!isTypeOfTclPyObject(raw_tcl_obj)) {
            auto name = raw_tcl_obj->typePtr->name;

            std::ostringstream error_message;
            error_message << "Expected PyObject as argument";
            error_message << "but got " << tcl_obj.get<std::string>(interp);

            throw Tcl::tcl_error{error_message.str()};
        }

        PyObject* pyObj = static_cast<PyObject*>(raw_tcl_obj->internalRep.otherValuePtr);
        pybind11::object pyBindObj = pybind11::reinterpret_borrow<pybind11::object>(pyObj);

        return pyBindObj;
    }
    break;
    }
}

Type detectEquivalentType(Tcl::object& tcl_obj) {
    auto tcl_type = tcl_obj.get_object()->typePtr;

    // Always say tcl object is of type string
    Type type = Type::PyUnicode;
    if (!tcl_type)
    {
        return type;
    }

    if (isTypeOfTclPyObject(tcl_obj.get_object()))
    {
        type = Type::TclPyObject;
    } 
    else if (std::strcmp(tcl_type->name, "int") == 0)
    {
        type = Type::PyLong;
    }
    else if (std::strcmp(tcl_type->name, "double") == 0)
    {
        type = Type::PyFloat;
    }
    else if (std::strcmp(tcl_type->name, "boolean") == 0)
    {
        type = Type::PyBool;
    }
    else if (std::strcmp(tcl_type->name, "string") == 0)
    {
        type = Type::PyUnicode;
    }
    else if (std::strcmp(tcl_type->name, "list") == 0)
    {
        type = Type::PyList;
    }
    else
    {
        std::cout << "Cannot detect type of " << tcl_type->name << std::endl;
    }

    return type;
}

/**
 * python function
 */
void createTclToPythonFunction(std::string name, py::function function)
{
    py::module inspect = py::module::import("inspect");
    py::object signature = inspect.attr("signature")(function);
    py::dict parameters = signature.attr("parameters");

    std::vector<Type> types;

    for (const auto &param : parameters)
    {
        py::object type = param.second.attr("annotation");
        py::object kind = param.second.attr("kind");
        std::string var_name = py::str(param.first);

        if (kind.is(param.second.attr("VAR_POSITIONAL")))
        {
            types.push_back(Type::VAR_POSITIONAL);
        }
        else if (type.is(py::reinterpret_borrow<py::object>(reinterpret_cast<PyObject *>(&PyUnicode_Type))))
        {
            types.push_back(Type::PyUnicode);
        }
        else if (type.is(py::reinterpret_borrow<py::object>(reinterpret_cast<PyObject *>(&PyLong_Type))))
        {
            types.push_back(Type::PyLong);
        }
        else if (type.is(py::reinterpret_borrow<py::object>(reinterpret_cast<PyObject *>(&PyFloat_Type))))
        {
            types.push_back(Type::PyFloat);
        }
        else
        {
            types.push_back(Type::Autodetect);
        }
    }

    auto func = [function, types = std::move(types)](Tcl::object const &argv) -> py::object {
        py::gil_scoped_acquire gil{};
        Tcl::interpreter interp{argv.get_interp(), false};

        TclPythonGlobals::AutoStack handle_interpreter{interp.get()};
        size_t argc = argv.size(interp);

        py::list arguments;

        for (size_t indx = 0; indx != argc; ++indx)
        {
            Tcl::object tcl_obj{argv.at(indx, interp)};
            tcl_obj.set_interp(interp.get());

            Type type;
            if (types.size() <= indx)
            {
                if (types[types.size() - 1] == Type::VAR_POSITIONAL)
                {
                    type = Type::VAR_POSITIONAL;
                }
                else
                {
                    throw Tcl::tcl_error{"More arguments given, but python function expects " + std::to_string(argc)};
                }
            }
            else
            {
                type = types[indx];
            }

            if (type == Type::Autodetect || type == Type::VAR_POSITIONAL)
            {
                type = detectEquivalentType(tcl_obj);
            }

            auto pyObj = castValue(type, tcl_obj);
            arguments.append(pyObj);
        }

        py::object result = function(*arguments);

        return result;
    };

    auto interp = TclPythonGlobals::getInstance().top();
    Tcl::interpreter i{interp};
    i.def<>(name, func, Tcl::variadic());

    // TODO: return raw address
}



} // namespace

PYBIND11_EMBEDDED_MODULE(_tcl, m) {
    py::enum_<Type>(m, "Type")
        .value("PyUnicode", Type::PyUnicode)
        .value("PyLong", Type::PyLong)
        .value("PyFloat", Type::PyFloat)
        .value("PyBool", Type::PyBool)
        .value("PyList", Type::PyList)
        .value("TclPyObject", Type::TclPyObject)
        .value("Autodetect", Type::Autodetect)
        .value("VAR_POSITIONAL", Type::VAR_POSITIONAL);

    m.def("createTclToPythonFunction", createTclToPythonFunction);
    m.def("detectEquivalentType", detectEquivalentType);
    m.def("castValue", [] (Type type, Tcl::object& tcl_obj) -> py::object {
        return castValue(type, tcl_obj);
    });
}
