#pragma once

#include "pybind11.h"

#include <memory>
#include <stack>
#include <unordered_map>

struct Tcl_Interp;

class TclPythonGlobals {
public:
    ~TclPythonGlobals();
    static TclPythonGlobals& getInstance()
    {
        static TclPythonGlobals instance;
        return instance;
    }

    class AutoStack {
        public:
        AutoStack(Tcl_Interp* interp) {
            TclPythonGlobals::getInstance().push(interp);
        }

        ~AutoStack() {
            TclPythonGlobals::getInstance().pop();
        }
    };

    void push(Tcl_Interp*);
    Tcl_Interp* pop();
    Tcl_Interp* top();
    pybind11::object getGlobals();

private:
    TclPythonGlobals();

    class Implementation;
    std::unique_ptr<Implementation> _p;
};
