set bootstrap_path [ file dirname  [ info script ] ]
set package_bin_path $bootstrap_path/../bin
set python_path $package_bin_path/python36
set jupyter_qtconsole $python_path/scripts/jupyter-qtconsole.exe

set env(PATH) "$package_bin_path;$python_path;$env(PATH);"
set env(PYTHONHOME) $python_path

load tclandpython.dll

set py [PythonInterpreter]
$py import sys
$py eval sys.path.append('$bootstrap_path')
$py eval sys.path.append('$package_bin_path')

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
    global py jupyter_qtconsole
    $py import ipy_kernel
    $py eval ipy_kernel.main('$jupyter_qtconsole')
}
call_ipython

$py exec {
import tcl83d
import ctypes
t_interp = cpptcl.get_current_interpreter_raw()
LP_Tcl_Interp = ctypes.POINTER(tcl83d.Tcl_Interp)
p = ctypes.cast(t_interp, LP_Tcl_Interp)
tcl83d.Tcl_Eval(p, 'set x Text')
}

puts $x

interp create sub_child
interp eval sub_child {
    load tclandpython.dll
    set py [PythonInterpreter]
    $py import sys
    $py import tcl

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
    $py eval {tcl.Tcl.proc("name", func)}
    $py eval {tcl.Tcl.proc("multiply", multiply)}

    name adsf
    set result [multiply $actual 5.1]
    puts $result
}

puts "Finish"
