/*
 * py3_misc.h
 *
 */

#ifndef INCLUDE_PY3_MISC_H_
#define INCLUDE_PY3_MISC_H_

namespace pybind11
{
class object;
}

struct Tcl_Obj;

Tcl_Obj* pythonToTclObject(pybind11::object& obj);



#endif /* INCLUDE_PY3_MISC_H_ */
