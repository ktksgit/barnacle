#!/usr/bin/env python

Import('*')

from os.path import join

baselibs   = [] # ['kernel32', 'advapi32', 'user32']
winlibs    = baselibs + [] # ['gdi32', 'comdlg32', 'winspool']

guilibs    = winlibs
conlibs    = baselibs
guilibsdll = winlibs
conlibsdll = baselibs
'''
tclPipe_dll = env.SharedLibrary(target = TCLPIPEDLLNAME,
				source=stub16_obj,
                LIBS = guilibs
)

tclDDE_dll = env.SharedLibrary(target=TCLDDEDLLNAME,
				source=tclWinDde_obj + tclStubLib_obj,
                LIBS = conlibs,
)

tclReg_dll = env.SharedLibrary(target=TCLREGDLLNAME,
				source=tclWinReg_obj + tclStubLib_obj,
                LIBS = conlibs,
)
env.Install(BIN_INSTALL_DIR, [tclPipe_dll, tclDDE_dll, tclReg_dll])
'''
env_export = env.Clone()
env_export.AppendUnique(CPPPATH = TCL_INCLUDES,
                        CPPDEFINES = ['BUILD_tcl'] + TCL_DEFINES
)


TCLDLL = env_export.SharedLibrary(target=TCLDLLNAME,
				source=TCLOBJS,
                LIBS = guilibsdll,
)

test_exe = env_export.Program(target=TCLTEST,
				source = TCLTESTOBJS,
                LIBS = conlibsdll + [TCLDLLNAME],
                LIBPATH = [BIN_INSTALL_DIR, Dir('#/lib').abspath],
)

sh_exe = env_export.Program(target=TCLSH,
				source = TCLSHOBJS,
                LIBS = conlibsdll + [TCLDLLNAME],
                LIBPATH = [BIN_INSTALL_DIR, Dir('#/lib').abspath],
)

print ('read')
env.Install(BIN_INSTALL_DIR, [TCLDLL, test_exe, sh_exe])
