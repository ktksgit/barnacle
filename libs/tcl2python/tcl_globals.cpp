#include "tcl_globals.h"

namespace py = pybind11;

class TclPythonGlobals::Implementation
{
    std::stack<Tcl_Interp*> _tcl_interpreter;
    std::unordered_map<Tcl_Interp*, pybind11::object> _py_globals_per_interpreter;

public:
    void push(Tcl_Interp * interp)
    {
        _tcl_interpreter.push(interp);
    }

    Tcl_Interp *top()
    {
        return _tcl_interpreter.top();
    }

    Tcl_Interp *pop()
    {
        auto interp = _tcl_interpreter.top();
        _tcl_interpreter.pop();
        return interp;
    }

    py::object getGlobals()
    {
        auto interp = _tcl_interpreter.top();
        auto global = _py_globals_per_interpreter.find(interp);
        if(global != _py_globals_per_interpreter.end()) {
            return global->second;
        }

        py::object new_global = py::dict{};
        new_global["__builtins__"] = py::reinterpret_borrow<py::object>(PyEval_GetBuiltins());
        _py_globals_per_interpreter.insert({interp, new_global});
        return new_global;
    }
};

TclPythonGlobals::TclPythonGlobals() :
    _p(std::make_unique<Implementation>())
{

}

TclPythonGlobals::~TclPythonGlobals() = default;

void TclPythonGlobals::push(Tcl_Interp * interp)
{
    _p->push(interp);
}

Tcl_Interp *TclPythonGlobals::top()
{
    return _p->top();
}

Tcl_Interp *TclPythonGlobals::pop()
{
    return _p->pop();
}

py::object TclPythonGlobals::getGlobals()
{
    return _p->getGlobals();
}
