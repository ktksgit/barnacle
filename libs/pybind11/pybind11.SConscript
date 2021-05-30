import os

Import('env')

pybind11_includes = File('pybind11.h').srcnode().get_abspath()
pybind11_includes = os.path.split(pybind11_includes)[0]
env['pybind11_includes'] = [ pybind11_includes ]
