import os

Import('pyenv env')

env_cpptcl = pyenv.Clone()

sources = File('cpptcl/cpptcl.cpp')

cpptcl_includes = sources.srcnode().get_abspath()
cpptcl_includes = os.path.split(cpptcl_includes)[0]
env['cpptcl_includes'] = [ cpptcl_includes ]
 
env_cpptcl.Append(
    CPPPATH=env['cpptcl_includes'] + 
             env['pybind11_includes'],
)

static_cpptcl = env_cpptcl.StaticLibrary(target = 'cpptcl',
            source = sources,
)

cpptclnostubs_obj = env_cpptcl.Object(target='cpptclnostubs',
            source = sources, 
            CPPDEFINES = {'CPPTCL_NO_TCL_STUBS' : ''},
)
static_cpptcl_no_stubs = env_cpptcl.StaticLibrary(cpptclnostubs_obj)


Return('static_cpptcl static_cpptcl_no_stubs')
