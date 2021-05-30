set bootstrap_path [ file dirname  [ info script ] ]
set package_bin_path $bootstrap_path/../bin
set package_scripts_path $bootstrap_path/../scripts
set python_path [lindex $argv 0]

set env(PATH) "$package_bin_path;$python_path;$env(PATH);"
# Sometimes env(PATH) contains non-accessable paths
# set env(PATH) "$package_bin_path;$python_path;"
set env(PYTHONHOME) $python_path
set env(PYTHONPATH) "$python_path/lib;$python_path/dlls"

load tclandpython.dll

set py [PythonInterpreter]
$py import sys
$py import site
$py eval sys.path.append('$bootstrap_path')
$py eval sys.path.append('$package_bin_path')
$py eval sys.path.append('$package_scripts_path')
$py eval site.addsitedir('$python_path/lib/site-packages')

$py eval {print('sys.path:\n', '\n'.join(sys.path), '\n') }

$py import pytcl
$py import cpptcl

set actual [$py eval {"%d.%d.%d" % sys.version_info[0:3]}]
set copy $actual
set actual [$py eval 5]


$py exec {
def func(text: str):
    print(type(text))
    print(text)
def multiply(value, value2: float):
    return value * value2
def return_py_list(element):
    return [element]
def call_variable_positional(func_name, *args):
    function = pytcl.CallProc(func_name)
    res = function(*args)
    print(res)
    return res
}
$py eval {pytcl.Tcl.proc("name", func)}
$py eval {pytcl.Tcl.proc("multiply", multiply)}
$py eval {pytcl.Tcl.proc("return_py_list", return_py_list)}
$py eval {pytcl.Tcl.proc("call_variable_positional", call_variable_positional)}

set result [call_variable_positional string trimleft "       text to be trimmed"]
puts $result

name adsf
set result [multiply $actual 5.1]
puts $result

proc call_ipython {} {
    global py python_path
    $py import ex01.ipy_kernel
    $py eval ex01.ipy_kernel.main('$python_path/python.exe')
}
call_ipython

puts "Test cpptcl.object_.addressof"
$py exec {
import cpptcl
import ctypes
o = cpptcl.object_("text")
pp_Tcl_Obj = cpptcl.object_.addressof(o)
LPLP_Tcl_Obj = ctypes.POINTER(ctypes.c_void_p)
p = ctypes.cast(pp_Tcl_Obj, LPLP_Tcl_Obj)
o2 = cpptcl.object_.reinterpret_cast(p[0])
print(str(o2))
}

puts "Test raw access with ctypes"
$py exec {
import ctypes

tcl83d = ctypes.CDLL('tcl83d')
t_interp = cpptcl.interpreter.reinterpret_cast(cpptcl.get_current_interpreter())
LP_Tcl_Interp = ctypes.POINTER(ctypes.c_void_p)
p = ctypes.cast(t_interp, LP_Tcl_Interp)
command = ctypes.create_string_buffer(b'set x Text')
tcl83d.Tcl_Eval(p, command)
result = cpptcl.result(cpptcl.get_current_interpreter().get())
}

puts $x

proc tclInit {} {

}

interp create sub_child
interp eval sub_child {
    load tclandpython.dll
    set py [PythonInterpreter]
    $py import sys
    $py import pytcl

    set actual [$py eval {"%d.%d.%d" % sys.version_info[0:3]}]
    set copy $actual
    set actual [$py eval 5]

    $py exec {
def func(text: str):
    print(type(text))
    print(text)
def multiply(value, value2: float):
    return value * value2
    }
    $py eval {pytcl.Tcl.proc("name", func)}
    $py eval {pytcl.Tcl.proc("multiply", multiply)}

    name adsf
    set result [multiply $actual 5.1]
    puts $result
}

puts "Finish"
