import os

import win32


def get_image_base(exe_name = 'Diggles'):
    pid = os.getpid()

    handle_snapshot = win32.CreateToolhelp32Snapshot(win32.TH32CS_SNAPMODULE, pid)

    module = win32.Module32First(handle_snapshot)

    while module:
        if exe_name in module.szExePath.decode('ascii'):
            return module.modBaseAddr
        module = win32.Module32Next(handle_snapshot, module)

    return None
