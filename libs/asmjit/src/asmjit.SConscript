import os

Import('env')

sources = Glob('asmjit/**/*.cpp', source=True)


asmjit_includes = File('CMakeLists.txt').srcnode().get_abspath()
asmjit_includes = os.path.split(asmjit_includes)[0]
env['asmjit_includes'] = [ asmjit_includes ]

asmjit = env.SharedLibrary(target = 'asmjit',
            source = sources,
            CPPDEFINES = {'ASMJIT_BUILD_X86' : ''},
            CPPPATH = Dir('#/test')
)

env['asmjit_lib_path'] = os.path.dirname(asmjit[0].get_abspath())


env.Install(Dir('#/bin'), asmjit)
