Import('pyenv env static_cpptcl tclStubLib_obj')

env_tcl2python = pyenv.Clone()
env_py3 = pyenv.Clone()

py3_obj = env_py3.Object(['py3.cpp',
                      'tcl_globals.cpp'])

env_py3.AppendUnique(CPPPATH=env['pybind11_includes'])

py3_static = env_py3.StaticLibrary('py3_static', source = py3_obj)

sources = ['tclandpython.cpp',
           'module_native.cpp',
           'module_ctcl.cpp',
           'module_cpptcl.cpp',
           'module_tcl.cpp']
tclpython_obj = py3_obj + env_tcl2python.Object(sources)

env_tcl2python.AppendUnique(CPPPATH= env['pybind11_includes'] + env['polyhook_includes'] + env['asmjit_includes'] + env['cpptcl_includes'],
    # CPPDEFINES = {'ASMJIT_BUILD_X86' : ''},
    LIBPATH = env['asmjit_lib_path'] ,
    LIBS = [static_cpptcl, tclStubLib_obj, 'ntdll'] + env['polyhook'] + ['asmjit'] + ['capstone']
)

tclpython_dll = env_tcl2python.SharedLibrary('tclandpython',
    source = tclpython_obj,
)

env_tcl2python.Install('#/bin', tclpython_dll)

Export('tclpython_obj')
Return('tclpython_dll py3_static')
