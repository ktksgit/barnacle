"""
Source: https://github.com/melver/scons-bare
License: Creative Commons Zero v1.0 Universal

SCons tool to emit compile_commands.json for files compiled in current
invocation.
Requires default CXXCOMSTR and CCOMSTR.
"""

import json
import os
import SCons


#module global list
_compile_commands = []

def make_strfunction(strfunction):
    def _strfunction(target, source, env, **kwargs):
        cwd = os.getcwd()
        cmd = strfunction(target, source, env, **kwargs)
        _compile_commands.append({
            'directory' : cwd,
            'command'   : cmd,
            'file'      : source[0].rstr()
            })
        return cmd
    return _strfunction


def write_compile_commands(target, source, env):
    with open(str(target[0]), 'w') as f:
        json.dump(_compile_commands, f, indent=2, sort_keys=True)


def compile_commands(env, outdir, depends):
    c_strfunction = make_strfunction(SCons.Defaults.CAction.strfunction)
    SCons.Defaults.CAction.strfunction = c_strfunction
    cxx_strfunction = make_strfunction(SCons.Defaults.CXXAction.strfunction)
    SCons.Defaults.CXXAction.strfunction = cxx_strfunction

    compile_commands_path = env.Dir(outdir).File("compile_commands.json")
    target = env.Command(target=compile_commands_path,
                         source=[],
                         action=env.Action(write_compile_commands,
                                           f"[CompileCommands] Writing {compile_commands_path}"))
    env.Depends(target, depends)
    env.AlwaysBuild(target)


def generate(env, **kwargs):
    env.AddMethod(compile_commands, "CompileCommands")


def exists(env):
    return True
