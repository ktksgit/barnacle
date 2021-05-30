cd ../bin/python36
python.exe -m pip install --upgrade --force-reinstall pip
cd scripts
rem Workaround for https://github.com/ipython/ipython/issues/12740
pip install pip install jedi==0.17.2
pip install qtconsole
pip install pyqt5
pip install ptvsd
