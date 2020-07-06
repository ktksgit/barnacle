# DIGGLES SDK

## How to build

It is only tested to build with Python 3.6 32bit
So
* install Python 32 bit or use the one from the zip
* install scons
* Run the build `scons platform=windows -j8 --nodebug --python-base-path=D:/Python36-x86 --python-lib=python36`


## How to use pip

Run `bin/python36/python.exe -m pip install --upgrade --force-reinstall pip`
Now you should be able to run `bin/python36/scripts/pip3.exe`
