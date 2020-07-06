#!/usr/bin/env python

Import('*')

from os.path import join

baselibs   = ['kernel32', 'advapi32', 'user32']
winlibs    = baselibs + ['gdi32', 'comdlg32', 'winspool']

guilibs    = winlibs
conlibs    = baselibs
guilibsdll = winlibs
conlibsdll = baselibs

env_static = env.Clone()
env_static.AppendUnique(CPPPATH = TCL_INCLUDES,
                        CPPDEFINES = ['STATIC_BUILD', 'BUILD_tcl'] + TCL_DEFINES,
)

compile_static_exe = False
if compile_static_exe:
    tclStatic_lib = env_static.StaticLibrary(target=TCLLIB,
                   source=TCLOBJS,
                   LIBS = guilibsdll,
    )
    env.Install(LIB_INSTALL_DIR, tclStatic_lib)

else:
    test_exe = env_static.Program(target=TCLTEST+'static',
                    source = TCLTESTOBJS + TCLOBJS,
                    LIBS = conlibsdll + guilibsdll,
    )
    
    env.Install(LIB_INSTALL_DIR, test_exe)
