# import ptvsd; ptvsd.enable_attach(); ptvsd.wait_for_attach()

# import pydevd
# pydevd.settrace()

import os
import sys
import tempfile
import threading
import time

import gui.console

import IPython

connection_file = os.path.join(
    tempfile.gettempdir(),
    'connection-{:d}.json'.format(os.getpid()))


def runner(jupyter_qtconsole):
    while not os.path.exists(connection_file):
        time.sleep(1)
    print(f"Starting  {connection_file}")
    os.system(f"cmd /c {jupyter_qtconsole} --existing {connection_file}")
    print("runner started")


def main(jupyter_qtconsole):
    gui.console.Runner.close()

    t = threading.Thread(
        target=runner,
        args=[jupyter_qtconsole])
    t.start()

    IPython.embed_kernel(
            local_ns=sys._getframe(1).f_locals,
            connection_file=connection_file,
        )
    t.join()
