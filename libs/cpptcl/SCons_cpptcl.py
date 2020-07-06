import os

Import('*')

lenv = pyenv.Clone()

cpptcl_cpp = File('cpptcl/cpptcl.cpp')
cpptcl_includes = cpptcl_cpp.srcnode().get_abspath()
cpptcl_includes = os.path.split(cpptcl_includes)[0]
 
lenv.Append(CPPPATH=[cpptcl_includes,
                    Dir('#/libs/pybind11')],
)

static_cpptcl = lenv.StaticLibrary('cpptcl', cpptcl_cpp)

cpptclnostubs_obj = lenv.Object(target='cpptclnostubs', source=cpptcl_cpp, 
             CPPDEFINES = {'CPPTCL_NO_TCL_STUBS' : ''},
)
static_cpptcl_no_stubs = lenv.StaticLibrary(cpptclnostubs_obj)


Return('static_cpptcl static_cpptcl_no_stubs cpptcl_includes')
