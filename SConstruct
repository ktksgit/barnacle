import os

AddOption('--nomingw',
          dest='mingw',
          action='store_false',
          default=True,
          help='mingw build (defaults to gcc)')

AddOption('--nodebug',
          dest='nodebug',
          action='store_true',
          default=False,
          help='perform release build')

AddOption('--releasedebug',
          dest='releasedebug',
          action='store_true',
          default=False,
          help='perform debug release build')

AddOption('--python-base-path',
          dest='python_base_path',
          help=r'Python root directory (Like D:\Python38)')

AddOption('--python-lib',
          dest='python_lib',
          help=r'Python library to link against (e.g. "python38")')


vars = Variables()
vars.Add(
    EnumVariable(
        "platform",
        "Target platform",
        "",
        allowed_values=("linux", "windows"),
    )
)

is_mingw = GetOption('mingw')
nodebug = GetOption('nodebug')
releasedebug = GetOption('releasedebug')
python_base_path = GetOption('python_base_path')
python_lib = GetOption('python_lib')

if not os.path.exists(python_base_path):
    Exit(f"{python_base_path} not found")

if is_mingw == True:
    env = Environment(
        variables=vars,
        tools = ['mingw']
    )
    env["is_mingw"] = True
else:
    env = Environment(
        variables=vars,
        TARGET_ARCH='x86'
    )
    env["is_mingw"] = False
    print ('Visual Studio build')
    env.AppendUnique(CXXFLAGS =['/EHsc'])

if env["platform"] == "windows":
    env["windows"] = True
    windows = True
    python_include_path = os.path.join(python_base_path, 'include')
    capstone_include_path = os.path.join(python_base_path, 'Lib/site-packages/capstone/include')
    capstone_lib_path = os.path.join(python_base_path, 'Lib/site-packages/capstone/lib')
    Install(Dir('#/bin'), capstone_lib_path + '/capstone.dll')
    if not os.path.exists(capstone_include_path):
        Exit(f"Capstone not found at {capstone_lib_path}: pip install capstone")
else:
    env["windows"] = False
    windows = False
    python_include_path = "/usr/include/python3.6m"

# custom methods
def pre_process(env, source):
    env = env.Clone()
    env.Replace(OBJSUFFIX = '.E')
    env.AppendUnique(CCFLAGS = '-E')
    return env.Object(source)
env.AddMethod(pre_process, 'PreProcess')

if is_mingw:
    env.AppendUnique(CXXFLAGS=[#'-m32',
                                '-fPIC',
                               "-std=c++17"
    ])
    env.AppendUnique(CFLAGS=[# '-m32',
                            '-fPIC',
                             ])
    if releasedebug:
        env.AppendUnique(CXXFLAGS=['-g', '-O3'])
        env.AppendUnique(CFLAGS=['-g'])
    elif nodebug:
        env.AppendUnique(CXXFLAGS=['-O3'])
    else:
        env.AppendUnique(CXXFLAGS=['-g'])
        env.AppendUnique(CFLAGS=['-g'])
else:
    env.AppendUnique(CXXFLAGS=['/std:c++17']
    )
    if nodebug or releasedebug:
        env.AppendUnique(CPPFLAGS =['/W3',  '/MD', '/Od'],#, '/Gs'],
                         LINKFLAGS=['/RELEASE']
        )
    else:
        env.AppendUnique(CPPFLAGS =['/DEBUG', '/W3', '/MDd', '/Od'],
                         LINKFLAGS=['/DEBUG']
        )
        env['CCPDBFLAGS'] = '/Zi /Fd${TARGET}.pdb'
        env['PDB'] = '${TARGET.base}.pdb'

Export('env')

# distinguish between release/release_debug/debug builds
if releasedebug:
    variant_dir = "object/release_debug"
elif nodebug:
    variant_dir = "object/release"
else:
    variant_dir = "object/debug"

VariantDir(f'{variant_dir}/libs', 'libs', duplicate=0)

env.Tool('compile_commands')

# Build tcl 
(tcl_includes,
tcl_lib_name,
tcl_bin_install_dir,
tclStubLib_obj)= env.SConscript('libs/tcl8.3.2/SConscript_main.py',
               exports='nodebug is_mingw windows'
)

env.AppendUnique(CPPPATH = [capstone_include_path,
                            tcl_includes],
                # CPPDEFINES = {'__thiscall' : ''},
                LIBS = [tcl_lib_name],
                LIBPATH = [tcl_bin_install_dir, capstone_lib_path])

pyenv = env.Clone()
pyenv.AppendUnique(CPPPATH=[python_include_path],
                LIBS = [python_lib],
                LIBPATH = [os.path.join(python_base_path, 'libs')]
)
Export('pyenv')

(tclpython_dll,
static_cpptcl,
static_cpptcl_no_stubs,
py3_static) = pyenv.SConscript(f'{variant_dir}/libs/libs.SConscript',
            exports='tclStubLib_obj',
)

all_targets = [
    tclpython_dll,
    static_cpptcl,
    static_cpptcl_no_stubs,
    py3_static
]

env.CompileCommands('object', all_targets)

if is_mingw and env["windows"]:
    install_dir = Dir('#/bin').abspath

    path = os.environ['PATH']
    path = [x for x in path.split(';') if 'mingw32' in x][0]

    libstdcpp_6_dll = os.path.join(path, 'libstdc++-6.dll')
    env.Install(install_dir, libstdcpp_6_dll)


    if any([x in path for x in ['7.2.0','7.3.0', '8.1.0']]):
        
        libgcc_s_dw2_1_dll = os.path.join(path, 'libgcc_s_dw2-1.dll')
        env.Install(install_dir, libgcc_s_dw2_1_dll)
        
        libwinpthread_1_dll = os.path.join(path, 'libwinpthread-1.dll')
        env.Install(install_dir, libwinpthread_1_dll)
        
    if any([x in path for x in ['4.8.5']]):
        
        libgcc_s_sjlj_1_dll = os.path.join(path, 'libgcc_s_sjlj-1.dll')
        env.Install(install_dir, libgcc_s_sjlj_1_dll)
