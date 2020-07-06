Import('*')

env = pyenv.Clone()

pybind11_includes = Dir('#/libs/pybind11')

py3_obj = env.Object(['py3.cpp',
                      'tcl_globals.cpp'])
sources = ['tclandpython.cpp',
           'module_native.cpp',
           'module_ctcl.cpp',
           'module_cpptcl.cpp',
           'module_tcl.cpp']
tclpython_cpp = py3_obj + env.Object(sources)

env.AppendUnique(CPPPATH=[cpptcl_includes, pybind11_includes],
    LIBS = [static_cpptcl, tclStubLib_obj, 'ntdll']
)

tclpython_dll = env.SharedLibrary('tclandpython',
    source = tclpython_cpp,
)

env.Install('#/bin', tclpython_dll)

Return('tclpython_dll py3_obj pybind11_includes')
