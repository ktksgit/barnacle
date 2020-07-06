
#include "tcl_globals.h"
#include "embed.h"
#include "cpptcl.h"

#include "py3.h"

namespace py = pybind11;

namespace
{

Tcl::interpreter get_current_interpreter() {
    Tcl::interpreter interp{TclPythonGlobals::getInstance().top(), false};
    return interp;
}

int get_current_interpreter_raw() {
    return reinterpret_cast<int>(TclPythonGlobals::getInstance().top());
}

void register_tcl_interpreter_class(py::module &m)
{
    py::class_<Tcl::interpreter>(m, "interpreter")
        .def(py::init<::Tcl_Interp *, bool>(), py::arg("arg0"), py::arg("owner") = false)
        .def_static("getDefault", (Tcl::interpreter * (*)()) & Tcl::interpreter::getDefault, " ")
        .def("make_safe", (void (Tcl::interpreter::*)()) & Tcl::interpreter::make_safe, " ")
        .def("get", (Tcl_Interp * (Tcl::interpreter::*)() const) & Tcl::interpreter::get, " ")
        //  (Tcl::result(Tcl::interpreter::*)(std::string const &)) 
        .def("eval",[] (Tcl::interpreter& self, std::string const & script) -> Tcl::object {
            Tcl::result res = self.eval(script);
            return res;
        }, " ", py::arg("script"))
        .def("eval", [] (Tcl::interpreter& self, Tcl::object const & script) -> Tcl::object {
            Tcl::result res = self.eval(script);
            return res;
        }, " ", py::arg("obj"))
        .def("getVar", (Tcl::result(Tcl::interpreter::*)(std::string const &, int)) & Tcl::interpreter::getVar, " ", py::arg("scalarTclVariable"), py::arg("flags"))
        .def("getVar", (Tcl::result(Tcl::interpreter::*)(std::string const &, std::string const &)) & Tcl::interpreter::getVar, " ", py::arg("arrayTclVariable"), py::arg("arrayIndex"))
        .def("setVar", (Tcl::result(Tcl::interpreter::*)(std::string const &, Tcl::object const & scalarTclVariable, int)) & Tcl::interpreter::setVar,
            " ", py::arg("arrayTclVariable"), py::arg("scalarTclVariable"), py::arg("flags"))
        .def("exists", (bool (Tcl::interpreter::*)(std::string const &)) & Tcl::interpreter::exists, " ", py::arg("scalarTclVariable"))
        .def("exists", (bool (Tcl::interpreter::*)(::std::string const &, ::std::string const &)) & Tcl::interpreter::exists, " ", py::arg("arrayTclVariable"), py::arg("arrayIndex"))
        .def("create_alias", (void (Tcl::interpreter::*)(::std::string const &, Tcl::interpreter &, ::std::string const &)) & Tcl::interpreter::create_alias, " ", py::arg("cmd"), py::arg("targetInterp"), py::arg("targetCmd"))
        .def("pkg_provide", (void (Tcl::interpreter::*)(::std::string const &, ::std::string const &)) & Tcl::interpreter::pkg_provide, " ", py::arg("name"), py::arg("version"))
        .def("create_namespace", (void (Tcl::interpreter::*)(::std::string const &)) & Tcl::interpreter::create_namespace, " ", py::arg("name"))
        .def_static("clear_definitions", (void (*)(::Tcl_Interp *)) & Tcl::interpreter::clear_definitions, " ", py::arg("arg0"));
}

void register_tcl_object_class(py::module &m)
{
    py::class_<Tcl::object>(m, "object_")
        .def(py::init<>())
        .def(py::init([](py::object& py_object) {
            Tcl_Obj* tcl_obj = pythonToTclObject(py_object);
            return Tcl::object{tcl_obj, false};
        }))
        .def(py::init<::Tcl_Obj *, bool>(), py::arg("o"), py::arg("shared") = false)
        .def(py::init<::Tcl::object const &, bool>(), py::arg("other"), py::arg("shared") = false)
        .def("assign", [](Tcl::object &self, py::object& py_object) {
            Tcl_Obj* tcl_obj = pythonToTclObject(py_object);
            self.assign(tcl_obj);
        })
        .def("swap", 
            (::Tcl::object & (Tcl::object::*)(::Tcl::object &)) & Tcl::object::swap," ", py::arg("other"))
        .def("get_bool",
            (bool (Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::get<bool>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("get_double",
            (double (Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::get<double>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("get_int",
            (int (Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::get<int>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("get_long",
            (long int (Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::get<long int>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("get_char",
            (char const *(Tcl::object::*)(Tcl::interpreter &)const) & Tcl::object::get<char const *>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("get_string",
            (std::string (Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::get<std::string>," ", py::arg("i") = Tcl::interpreter::defaultInterpreter)
        .def("assign",
            (::Tcl::object & (Tcl::object::*)(char const *, ::size_t)) & Tcl::object::assign," ", py::arg("buf"), py::arg("size"))
        .def("resize",
            (::Tcl::object & (Tcl::object::*)(size_t)) & Tcl::object::resize," ", py::arg("size"))
        .def("get_bytes",[](const Tcl::object &obj) {
            size_t size;
            auto s = obj.get(size);  // Not valid UTF-8
            return py::make_tuple(s, py::int_(size)); // Return the data without transcoding
        } ," ")
        .def("size",
            (::size_t(Tcl::object::*)(Tcl::interpreter &) const) & Tcl::object::size," ", py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("at",
            (::Tcl::object(Tcl::object::*)(::size_t, Tcl::interpreter &) const) & Tcl::object::at," ", py::arg("index"), py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("append",
            (::Tcl::object & (Tcl::object::*)(::Tcl::object const &, Tcl::interpreter &)) & Tcl::object::append," ", py::arg("o"), py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("append_list",
            (::Tcl::object & (Tcl::object::*)(::Tcl::object const &, Tcl::interpreter &)) & Tcl::object::append_list," ", py::arg("o"), py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("replace",
            (::Tcl::object & (Tcl::object::*)(::size_t, ::size_t, ::Tcl::object const &, Tcl::interpreter &)) & Tcl::object::replace," ", py::arg("index"), py::arg("count"), py::arg("o"), py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("replace_list",
            (::Tcl::object & (Tcl::object::*)(::size_t, ::size_t, ::Tcl::object const &, Tcl::interpreter &)) & Tcl::object::replace_list," ", py::arg("index"), py::arg("count"), py::arg("o"), py::arg("i") = *Tcl::interpreter::defaultInterpreter)
        .def("get_object",
            (::Tcl_Obj * (Tcl::object::*)() const) & Tcl::object::get_object," ")
        .def("set_interp",
            (void (Tcl::object::*)(::Tcl_Interp *)) & Tcl::object::set_interp," ", py::arg("interp"))
        .def("get_interp",
            (::Tcl_Interp * (Tcl::object::*)() const) & Tcl::object::get_interp," ")
        .def("exists",
            (bool const (Tcl::object::*)(::std::string) const) & Tcl::object::exists," ", py::arg("idx"))
        .def("bind",
            (void (Tcl::object::*)(::std::string const &)) & Tcl::object::bind," ", py::arg("variableName"))
        .def("bind",
            (void (Tcl::object::*)(::std::string const &, ::std::string const &)) & Tcl::object::bind," ", py::arg("variableName"), py::arg("indexName"))
        .def("__str__",
            (::std::string(Tcl::object::*)() const) & Tcl::object::asString," ")
        .def("__int__",
            (long int (Tcl::object::*)() const) & Tcl::object::asLong," ")
        .def("__float__",
            (double (Tcl::object::*)() const) & Tcl::object::asDouble," ");
}

void register_tcl_result_class(py::module &m){
    py::class_<Tcl::result>(m, "result")
        .def_property_readonly("object_", &Tcl::result::operator Tcl::object)
        .def("__str__", &Tcl::result::operator std::string)
        .def("__float__", &Tcl::result::operator double)
        .def("__int__", &Tcl::result::operator long)
        ;
}

}

PYBIND11_EMBEDDED_MODULE(cpptcl, m) {
    register_tcl_interpreter_class(m);
    register_tcl_object_class(m);
    register_tcl_result_class(m);
    m.def("get_current_interpreter_raw", get_current_interpreter_raw);
    m.def("get_current_interpreter", get_current_interpreter);
}
