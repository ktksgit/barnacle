#include <cstdlib>
#include <cstring>

#include "py3.h"

#include <Python.h>
#include "cpptcl.h"

#include <iostream>
#include "embed.h"
#include "tcl_globals.h"

namespace py = pybind11;
using namespace py::literals;

namespace py = pybind11;

class PythonInterpreterSingleton {
	PythonInterpreterSingleton() {
		py::initialize_interpreter();
	}
public:
	~PythonInterpreterSingleton() {
		py::finalize_interpreter();
	}

	PythonInterpreterSingleton(PythonInterpreterSingleton const&) = delete;
	void operator=(PythonInterpreterSingleton const&) = delete;

	static PythonInterpreterSingleton& getInstance() {
		static PythonInterpreterSingleton instance;
		return instance;
	}
private:
};

struct PythonInterpreter {
public:
	PythonInterpreter() {
		PythonInterpreterSingleton::getInstance();

		// Inject something 
		py::module::import("sys").attr("argv") = py::make_tuple("insert anything stupid", "");
		/// importi("tcl", Tcl::interpreter::getDefault()->get());
	}

	~PythonInterpreter() = default;

	void exec(const char *str, const Tcl::object &dummy) {
		auto interp = dummy.get_interp();
		TclPythonGlobals::AutoStack handle_interpreter{interp};

		auto globals = TclPythonGlobals::getInstance().getGlobals();
		py::exec(str, globals);
	}

	PyObject* eval(const char *str, const Tcl::object &dummy) {
		auto interp = dummy.get_interp();
		TclPythonGlobals::AutoStack handle_interpreter{interp};

		auto globals = TclPythonGlobals::getInstance().getGlobals();

		py::object obj = py::eval(str, globals);
		return obj.release().ptr();
	}

	std::string  str(py::object str, const Tcl::object &dummy) {
		auto interp = dummy.get_interp();
		TclPythonGlobals::AutoStack handle_interpreter{interp};

		return py::str(str);
	}

	void import(const char*str, const Tcl::object &dummy) {
		auto interp = dummy.get_interp();

		importi(str, interp);
	}
private:
	void importi(const char*str, Tcl_Interp* interp) {
		TclPythonGlobals::AutoStack handle_interpreter{interp};
		auto globals = TclPythonGlobals::getInstance().getGlobals();

		py::module imported = py::module::import(str);
		globals[str] = imported;
	}
};

CPPTCL_MODULE(Tclandpython, interp) {
	Tcl_RegisterObjType(getTclPyObjectInstance());

    interp.class_<PythonInterpreter>("PythonInterpreter")
         .def("exec", &PythonInterpreter::exec, Tcl::variadic())
         .def("eval", &PythonInterpreter::eval, Tcl::variadic())
         .def("str", &PythonInterpreter::str, Tcl::variadic())
         .def("import", &PythonInterpreter::import, Tcl::variadic());
}
