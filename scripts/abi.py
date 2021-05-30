#%%

import process
import win32
import ctypes
from typing import Type, NewType, List, Tuple, Dict

#%% Native code
from _native.asmjit.Type import Id # tclandpython.dll

executable = win32.GetModuleFileName()
image_base = process.get_image_base(executable)

#%% Data types and private functions

Address = NewType('Address', int)
Abi = NewType('Abi', Tuple[Address, str, Id, List[Id]])

def BASE(address: int) -> Address:
    B_4 = 0x400000
    if address < B_4:
        raise ValueError(f"{hex(address)} shall be bigger than {hex(B_4)}")

    return Address(address - 0x400000)


def arg_count(abi: Abi) -> int:
    return len(abi[3])


ABI_1_0_844_59495 = {
    "findFileLocation": (BASE(0x46A150), Id.kIdUIntPtr, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "CObj_tclEval": (BASE(0x483710), Id.kIdI8,  "thiscall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "script_call": (BASE(0x592560), Id.kIdI32, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdI32, Id.kIdUIntPtr]),
    "sTclObject_loadTclFile": (BASE(0x6A46CD), Id.kIdI32, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdI32, Id.kIdI32]),
    "sTclObject_preprocessFile": (BASE(0x6A4708), Id.kIdVoid, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdI32, Id.kIdI32]),
    "BuiltinTclData_ctor_Tcl_Interp": (BASE(0x6A5C06), Id.kIdVoid, "thiscall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "TclCache_get": (BASE(0x6A538F), Id.kIdVoid, "thiscall", [Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdI32]),
    "g_tclCache": (BASE(0xECAE80), Id.kIdUIntPtr, None, None),     
}



ABI_2_1_1_10 = {
    "findFileLocation": (BASE(0x4402C0), Id.kIdUIntPtr, "fastcall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "CObj_tclEval": (BASE(0x555A00), Id.kIdI8,  "thiscall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "script_call": (BASE(0x6313A0), Id.kIdI32, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr, Id.kIdI32, Id.kIdUIntPtr]),
    "sTclObject_preprocessFile": (BASE(0x6BB140), Id.kIdVoid, "fastcall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "BuiltinTclData_ctor_Tcl_Interp": (BASE(0x6BC170), Id.kIdVoid, "thiscall", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "TclCache_get": (BASE(0x6BBB00), Id.kIdVoid, "cdecl", [Id.kIdUIntPtr, Id.kIdUIntPtr]),
    "g_tclCache": (BASE(0xA42DD0), Id.kIdUIntPtr, None, None),     
}


def _prepare_abi(original_abi: Dict[str, Abi]) -> Dict[str, Abi]:
    abi = {}
    for key, value in original_abi.items():
        abi[key] = (Address(int(value[0]) + image_base),) + value[1:]
    return abi


def detect_abi():
    """
    Returns detected ABI
    """
    autoversion_build = ctypes.cast(image_base + 0x78236C - 0x400000, ctypes.c_char_p)

    if autoversion_build.value:
        autoversion_build = autoversion_build.value.decode("ascii")
        if autoversion_build == '$AUTOVERSION_BUILD 844   ':
            return _prepare_abi(ABI_1_0_844_59495)

    version_string = ctypes.cast(image_base + 0x722148 - 0x400000, ctypes.c_char_p)

    if version_string.value:
        version_string = version_string.value.decode("ascii")
        if version_string == '2.1.1.10':
            return _prepare_abi(ABI_2_1_1_10)

    return None
