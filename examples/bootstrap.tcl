set info_script [ info script ]
if { $info_script == "" } {
    error "Use \"source bootstrap.tcl\""
}

if { [catch {puts "Writing stdout to console"} fid] } {
    set stdout [open stdout.txt w]
} else {
    set stdout stdout
}

set bootstrap_path [ file dirname  $info_script ]
set package_bin_path $bootstrap_path/../bin
set python_path $package_bin_path/python36
set jupyter_qtconsole $python_path/scripts/jupyter-qtconsole.exe
puts $stdout "bootstrap_path $bootstrap_path"
puts $stdout "package_bin_path $package_bin_path"
puts $stdout "python_path $python_path"

set env(PATH) "$package_bin_path;$python_path;$env(PATH);"
set env(PYTHONHOME) $python_path

load tclandpython.dll

set py [PythonInterpreter]
$py exec "
import sys

# This makes import from the example directory possible
sys.path.append('$bootstrap_path')
"

# $py import pytcl
# $py import cpptcl

# Use exclusively first:
# $py import gui.console
# $py gui.console.main()

# or second:
# $py import ipy_kernel
# $py eval ipy_kernel.main('$jupyter_qtconsole')
