#include "py3.h"

#include <sstream>

#include "tcl.h"
#include "pybind11.h"


namespace py = pybind11;
using namespace py::literals;

class TclPyObject : public Tcl_ObjType {
private:

	static void Tcl_freeIntRepProc(Tcl_Obj *objPtr) {
		PyObject* pyObj = static_cast<PyObject*>(objPtr->internalRep.otherValuePtr);
		py::object obj = pybind11::reinterpret_steal<py::object>(pyObj);
	}

	static void Tcl_dupIntRepProc(Tcl_Obj *srcPtr, Tcl_Obj *dupPtr) {
		dupPtr->typePtr = &getInstance();

		PyObject* pyObj = static_cast<PyObject*>(srcPtr->internalRep.otherValuePtr);
		py::object obj = py::reinterpret_borrow<py::object>(pyObj);

		obj.inc_ref();
		dupPtr->internalRep.otherValuePtr = obj.ptr();
	}

	static void Tcl_updateStringProc(Tcl_Obj *objPtr) {
		// TODO: Could call __str__ method of python object
		std::ostringstream o;
		o << "py" << objPtr->internalRep.otherValuePtr;

		objPtr->length = o.str().size();
		objPtr->bytes = (char*) ckalloc(static_cast<unsigned int>(objPtr->length + 1));
		memcpy(objPtr->bytes, o.str().data(), objPtr->length);
		objPtr->bytes[objPtr->length] = '\0';
	}

	static int Tcl_setFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr) {
		return TCL_ERROR;
	}
public:
	static TclPyObject& getInstance() {
		static TclPyObject singleton;
		return singleton;
	}

	static Tcl_Obj* newObj(pybind11::object &obj) {
		Tcl_Obj *objPtr = Tcl_NewObj();
		objPtr->bytes = nullptr;
		objPtr->typePtr = &getInstance();

		objPtr->internalRep.otherValuePtr = obj.release().ptr();
		return objPtr;
	}

	static TclPyObject* getElem(Tcl_Interp *interp, Tcl_Obj *const objPtr) {
		int status;

		if ((status = Tcl_ConvertToType(interp, objPtr, &getInstance()))
				!= TCL_OK)
			return nullptr;
		return reinterpret_cast<TclPyObject*>(objPtr->internalRep.otherValuePtr);
	}

	TclPyObject() {
		name = const_cast<char*>("TclPyObject");
		freeIntRepProc = &Tcl_freeIntRepProc;
		dupIntRepProc = &Tcl_dupIntRepProc;
		updateStringProc = &Tcl_updateStringProc;
		setFromAnyProc = &Tcl_setFromAnyProc;
	}
};

Tcl_ObjType* getTclPyObjectInstance() 
{
	return &TclPyObject::getInstance();
}

Tcl_Obj* pythonToTclObject(py::object& obj)
{
	Tcl_Obj* tclObj = nullptr;
	PyObject* p = obj.ptr();
	if (PyFloat_CheckExact(p))
	{
		py::float_ s = py::cast<py::float_>(obj);
		tclObj =  Tcl_NewDoubleObj(static_cast<double>(s));
	}
	else if (PyLong_CheckExact(p))
	{
		py::int_ s = py::cast<py::int_>(obj);
		tclObj = Tcl_NewLongObj(s);
	}
	else if (PyUnicode_CheckExact(p))
	{
		std::string s = py::str(obj);
		tclObj = Tcl_NewStringObj(s.c_str(), s.length());
	}
	else
	{
	    tclObj = TclPyObject::newObj(obj);
	}

	// Tcl_IncrRefCount(tclObj);
	return tclObj;
}

bool isTypeOfTclPyObject(Tcl_Obj* tcl_obj)
{
	return (tcl_obj->typePtr == &TclPyObject::getInstance());
}
