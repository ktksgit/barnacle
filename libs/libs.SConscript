import os

Import('env pyenv tclStubLib_obj')

# Build asmjit dll library
env.SConscript('asmjit/src/SConstruct')

# Build polyhook static library
env.SConscript('polyhook2.0/polyhook.SConscript')

# Build cpptcl static library
(static_cpptcl, static_cpptcl_no_stubs, cpptcl_includes) = pyenv.SConscript('cpptcl/SCons_cpptcl.py')

# Build tcl extensions
tclpython_dll, py3_obj, pybind11_includes = pyenv.SConscript('tcl2python/tcl2python.SConscript',
                exports='pyenv static_cpptcl cpptcl_includes tclStubLib_obj',
                duplicate=0,
)

Return('tclpython_dll static_cpptcl static_cpptcl_no_stubs py3_obj cpptcl_includes pybind11_includes')

