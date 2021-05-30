import os

Import('env pyenv tclStubLib_obj')

# Build asmjit dll library
env.SConscript('asmjit/src/asmjit.SConscript')

# Build polyhook static library
env.SConscript('polyhook2.0/polyhook.SConscript')

# Header only
env.SConscript('pybind11/pybind11.SConscript')

# Build cpptcl static library
(static_cpptcl, static_cpptcl_no_stubs) = pyenv.SConscript('cpptcl/cpptcl.SConscript')

# Build tcl extensions
tclpython_dll, py3_static = pyenv.SConscript('tcl2python/tcl2python.SConscript',
                exports='pyenv static_cpptcl tclStubLib_obj',
                duplicate=0,
)

Return('tclpython_dll static_cpptcl static_cpptcl_no_stubs py3_static')

