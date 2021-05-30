# This file will bootstrap the current directory
# It adds python path to the environment variable PATH
# It adds this SDKs bin path, namely ../bin, to the environment variable PATH
#
# A python interpreter is created
# It adds this path to Python's sys.path
# It adds this SDKs scripts path, namely ../scripts, to Python's sys.path
# 
# How to use
# ----------
# Add line
# source <sdk_path>/examples/bootstrap.tcl
# to the very beginning of <diggles_path>/data/systeminit.tcl
# where
# <sdk_path> is to be replaced with the path where you unpacked the SDK
# <diggles_path> is to be replaced with the path where you're Diggles installation (Diggles.exe) is located

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
set package_scripts_path $bootstrap_path/../scripts


if { [ info exists python_path] } {
    # Switch to user supplied python path
} else {
    # Use self-contained python 3.6 32-bit installation
    set python_path $package_bin_path/python36
}

puts $stdout "bootstrap_path $bootstrap_path"
puts $stdout "package_bin_path $package_bin_path"
puts $stdout "package_scripts_path $package_scripts_path"
puts $stdout "python_path $python_path"

set env(PATH) "$package_bin_path;$python_path;"
set env(PYTHONHOME) $python_path
set env(PYTHONPATH) "$python_path/lib;$python_path/dlls;"

load tclandpython.dll

set py [PythonInterpreter]
$py exec "
import sys
import site

# This makes import from the example directory possible
sys.path.append('$bootstrap_path')
sys.path.append('$package_scripts_path')
site.addsitedir('$python_path/lib/site-packages')
"
