#ifndef PY_H
#define PY_H

#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

struct Tcl_ObjType;
struct Tcl_Obj;

namespace pybind11
{
    class object;
}

Tcl_ObjType* getTclPyObjectInstance();
Tcl_Obj* pythonToTclObject(pybind11::object&);

#endif
