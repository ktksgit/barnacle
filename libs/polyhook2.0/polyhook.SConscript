import os

Import('env')

sources = Glob('polyhook2/**/*.cpp', source=True)
sources = [
    "sources/ADetour.cpp",
    "sources/CapstoneDisassembler.cpp",
    "sources/ILCallback.cpp",
    "sources/MemProtector.cpp",
    "sources/PageAllocator.cpp",
    "sources/PyCallback.cpp",
    "sources/x86Detour.cpp",
]

polyhook_includes = File('CMakeLists.txt').srcnode().get_abspath()
polyhook_includes = os.path.split(polyhook_includes)[0]
env['polyhook_includes'] = polyhook_includes

polyhook = env.StaticLibrary(target = 'polyhook',
    source = sources,
    # CPPDEFINES = {'NO_PAGE_ALLOCATOR' : ''},
    CPPPATH = ['$polyhook_includes', '$asmjit_includes', '$CPPPATH']
)

env['polyhook'] = polyhook

# env.Install(Dir('#/bin'), polyhook)
