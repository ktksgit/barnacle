#print("Waiting for Visual Python debugger to attach")
# import ptvsd; ptvsd.enable_attach(); ptvsd.wait_for_attach()

# import pydevd
# pydevd.settrace()

import os
import sys
import subprocess
import tempfile
import threading
import time

from pathlib import Path

import IPython

connection_file = Path(tempfile.gettempdir()) / 'connection-{:d}.json'.format(os.getpid())
jupyter_qtconsole_exe = Path(__file__).parent / 'jupyter' / 'qt_console.py'


def clean_up_environment_path():
    os_path = []
    for p in os.environ['PATH'].split(';'):
        if Path(p).exists() and len(p):
            os_path.append(str(p))
        else:
            print("Removed from PATH:", str(p))
    os.environ['PATH'] = ';'.join(os_path) + ';'


def runner(python_exe, timeout=10):
    print(f"Waiting {timeout} sec for {connection_file} to be created")
    count_down = timeout
    while count_down >= 1 and not connection_file.exists():
        time.sleep(1)
        count_down -= 1

    if not connection_file.exists():
        print(f"{connection_file} does not exist after {timeout} seconds")
        return

    count_down = timeout
    while count_down >= 1 and len(connection_file.read_text()) < 2:
        time.sleep(1)
        count_down -= 1

    if len(connection_file.read_text()) < 2:
        print(f"{connection_file} is empty after {timeout} seconds")
        return

    # print(os.system(f"cmd.exe /c {jupyter_qtconsole_exe} --existing {connection_file}"))
    command = [python_exe, str(jupyter_qtconsole_exe), "--existing", str(connection_file)]
    print('Starting:', ' '.join(command))
    proc = subprocess.Popen(
        args=command,
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        universal_newlines=True)
    
    # we need to read. read is a blocking call
    print("Process stderr\n", proc.stderr.read())
    print("Process stdout\n", proc.stdout.read())

    print(f"Closed {jupyter_qtconsole_exe}")


def main(python_exe, throw_on_error = False):
    clean_up_environment_path()

    t = threading.Thread(
        target=runner,
        args=[python_exe])
    t.start()

    print("Running IPython embeded kernel")
    try:
        IPython.embed_kernel(
            local_ns=sys._getframe(1).f_locals,
            connection_file=str(connection_file),
        )
    except Exception as exp:
        print("Unable to embed IPython. Exception occured:")
        print(exp)
        if throw_on_error:
            raise exp
    finally:
        if sys.stdout:
            sys.stdout.flush()
        t.join()


if __name__ == "__main__":
    main(sys.executable)
