#!/usr/bin/env python

# Requirements
# sudo apt install libxt-dev
# sudo apt install gcc-multilib g++-multilib

from os.path import join

Import('*')

ROOT = Dir('.').path
INSTALLDIR  = Dir('.').abspath

#THREADDEFINES = 'TCL_THREADS=1'
THREADDEFINES = []

#DEBUGDEFINES = 'TCL_MEM_DEBUG TCL_COMPILE_DEBUG TCL_COMPILE_STATS'
#DEBUGDEFINES = 'USE_TCLALLOC'
DEBUGDEFINES = []

NAMEPREFIX = 'tcl'
STUBPREFIX = ''.join(DEBUGDEFINES) + 'stub'
DOTVERSION = '8.3'
VERSION = '83'

BINROOT     = ROOT
if nodebug:
    TMPDIRNAME  = 'Release'
    DBGX        = ''
else:
    TMPDIRNAME  = 'Debug'
    DBGX        = 'd'

TMPDIR      = TMPDIRNAME
OUTDIR      = join(ROOT,TMPDIRNAME)

TCLLIB      = NAMEPREFIX + VERSION + DBGX
TCLDLLNAME  = NAMEPREFIX + VERSION + DBGX
TCLDLL      = join(OUTDIR, TCLDLLNAME)

if windows:
    PLATFORMDIR      = join(INSTALLDIR, 'win')
    platform = 'win'
else:
    PLATFORMDIR      = join(INSTALLDIR, 'unix')
    platform = 'unix'

GENERICDIR  = join(INSTALLDIR, 'generic')

TCLSTUBLIBNAME  = STUBPREFIX + VERSION + DBGX
TCLPLUGINLIB    = join(OUTDIR,NAMEPREFIX) + VERSION + 'p' + DBGX + '.lib'
TCLPLUGINDLLNAME= NAMEPREFIX + VERSION + 'p' + DBGX
TCLSH       = join(OUTDIR, NAMEPREFIX) + 'sh' + VERSION + 'p' + DBGX
TCLSHP      = join(OUTDIR, NAMEPREFIX) + 'shp' + VERSION + 'p' + DBGX
TCLPIPEDLLNAME  = NAMEPREFIX + 'pip' + VERSION + DBGX
TCLREGDLLNAME   = NAMEPREFIX + 'reg' + VERSION + DBGX
TCLDDEDLLNAME   = NAMEPREFIX + 'dde' + VERSION + DBGX
TCLTEST     = NAMEPREFIX + 'test'
CAT32       = 'cat32'

LIB_INSTALL_DIR = join(INSTALLDIR, 'lib')
BIN_INSTALL_DIR = join(INSTALLDIR, 'bin')
SCRIPT_INSTALL_DIR  = join(INSTALLDIR, 'lib\tcl') + DOTVERSION
INCLUDE_INSTALL_DIR = join(INSTALLDIR, 'include')


TCL_INCLUDES = [PLATFORMDIR] + [GENERICDIR]
TCL_DEFINES = DEBUGDEFINES + THREADDEFINES

if is_mingw:
    TCL_DEFINES +=  ['HAVE_NO_SEH']
if not windows:
    TCL_DEFINES +=  ['HAVE_UNISTD_H', 'NO_UNION_WAIT',
        'TIME_WITH_SYS_TIME',
        'HAVE_TM_ZONE',
        'HAVE_ST_BLKSIZE',
        'TCL_LIBRARY=\\"/home/mserver/Downloads/tcl8.3.2/library/\\"',
        f'TCL_PACKAGE_PATH=\\"{LIB_INSTALL_DIR}\\"'
    ]

if is_mingw and 0:
    Return('TCL_INCLUDES TCLDLLNAME BIN_INSTALL_DIR')


if windows:
    strftime_obj = env.SConscript('compat/Sconscript',
                exports='env TCL_DEFINES TCL_INCLUDES ',
                variant_dir=join(TMPDIR, 'compat'),
                duplicate=0
    )
else:
    strftime_obj = []


(tclStubLib_obj,
tcl_objs_gen,
tcl_test_objs_gen)= env.SConscript('generic/SConscript',
               exports='env TCL_DEFINES GENERICDIR '
                        'TCL_INCLUDES TCLSTUBLIBNAME ',
               variant_dir=join(TMPDIR, 'generic/dynamic'),
               duplicate=0
)

(tcl_objs,
tcl_test_objs,
TCLSHOBJS)=env.SConscript(f'{platform}/SConscript',
               exports='env TCL_DEFINES PLATFORMDIR '
                        'TCL_INCLUDES '
                        'is_mingw',
               variant_dir=join(TMPDIR, f'{platform}/dynamic'),
               duplicate=0
)

TCLOBJS = tcl_objs_gen + tcl_objs + strftime_obj + tclStubLib_obj
TCLTESTOBJS = tcl_test_objs_gen + tcl_test_objs

env.SConscript('SConscript_dll.py',
               exports='env '
                        'tclStubLib_obj '
                        'TCLREGDLLNAME TCLDDEDLLNAME TCLPIPEDLLNAME TCLDLLNAME TCLSH '
                        'TCLTEST TCLOBJS TCLTESTOBJS TCLSHOBJS '
                        'TCL_INCLUDES TCL_DEFINES '
						'LIB_INSTALL_DIR BIN_INSTALL_DIR ',
               variant_dir=join(TMPDIR, 'dynamic'),
               duplicate=0
)

## static library

(tclStubLib_obj,
tcl_objs_gen,
tcl_test_objs_gen)= env.SConscript('generic/SConscript',
               exports='env TCL_DEFINES GENERICDIR '
                        'TCL_INCLUDES TCLSTUBLIBNAME ',
               variant_dir=join(TMPDIR, 'generic/static'),
               duplicate=0
)

(tcl_objs,
tcl_test_objs,
TCLSHOBJS)=env.SConscript(f'{platform}/SConscript',
               exports='env TCL_DEFINES PLATFORMDIR '
                       'TCL_INCLUDES '
                       'is_mingw ',
               variant_dir=join(TMPDIR, f'{platform}/static'),
               duplicate=0
)

TCLOBJS = tcl_objs_gen + tcl_objs + strftime_obj + tclStubLib_obj
TCLTESTOBJS = tcl_test_objs_gen + tcl_test_objs

if not is_mingw:
    env.SConscript('SConscript_staticlib.py',
               exports='env '
                        'TCLREGDLLNAME TCLDDEDLLNAME TCLPIPEDLLNAME TCLDLLNAME TCLLIB '
                        'TCLTEST TCLOBJS TCLTESTOBJS tclStubLib_obj '
                        'TCL_INCLUDES TCL_DEFINES '
						'LIB_INSTALL_DIR BIN_INSTALL_DIR ',
               variant_dir=join(TMPDIR, 'static'),
               duplicate=0
    )

Return('TCL_INCLUDES TCLDLLNAME BIN_INSTALL_DIR tclStubLib_obj')
