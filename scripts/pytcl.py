__doc__ = """
Convenient access to tcl from Python
:class:`Tcl` is the main helper. Create an object of 
:class:`Tcl` and use Tcl.<tcl procedure name> to call a TCL procedure
Use :func:`Tcl.globals` to list TCL global variables
"""

import sys

import _tcl
import cpptcl

TCL_GLOBAL_ONLY = 1
TCL_LEAVE_ERR_MSG = 0x200


class CallProc:
    def __init__(self, name, autocast_list=True):
        self._name = name
        self._autocast_list = autocast_list

    def __call__(self, *args):
        if self._name:
            result = cpptcl.eval(self._name, *args)
            t = _tcl.detectEquivalentType(result)
            if not self._autocast_list and t == _tcl.Type.PyList:
                t = _tcl.Type.PyUnicode
            return _tcl.castValue(t, result)

        raise Exception(f"Cannot call {self._name} because it is not a proc")


class Globals:
    def __init__(self, interp):
        self._interp = interp

    def __getattr__(self, name):
        try:
            result = self._interp.getVar(name, TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG)
        except RuntimeError as exception:
            if 'variable is array' in str(exception):
                # print(exception, file=sys.stderr)
                return None
            if 'no such variable' in str(exception):
                # print(exception, file=sys.stderr)
                return None
            raise exception

        t = _tcl.detectEquivalentType(result.object_)
        return _tcl.castValue(t, result.object_)

    def __setattr__(self, name, value):
        if name == '_interp':
            super().__setattr__(name, value)
            return

        tcl_value = cpptcl.object_(value)
        self._interp.setVar(name, tcl_value, TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG)


class Tcl:
    CLASS_COMMANDS = ['proc', 'globals']

    def __init__(self):
        self.__cache_commands('')
    
    def __cache_commands(self, asked_command):
        '''
        Caching commands enables autocomplete at runtime in any Python editor
        '''
        if asked_command in Tcl.CLASS_COMMANDS:
            return

        command_list = cpptcl.eval('info', 'commands')
        t = _tcl.detectEquivalentType(command_list)
        command_list = _tcl.castValue(t, command_list)
        for my_command in Tcl.CLASS_COMMANDS:
            if my_command in command_list:
                command_list.remove(my_command)
        
        for command in command_list:
            setattr(self, command, CallProc(command))

    @classmethod
    def proc(cls, *args, **kwargs):
        return _tcl.createTclToPythonFunction(*args, **kwargs)

    @property
    def globals(self):
        return Globals(cpptcl.get_current_interpreter())

    def __getattr__(self, name):
        self.__cache_commands(name)
        return CallProc(name)

tcl = Tcl()
