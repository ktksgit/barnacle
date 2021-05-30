from ctypes import POINTER, c_int
import ctypes

import _native # tclandpython.dll
from _native.asmjit.Type import Id # tclandpython.dll

import cpptcl # tclandpython.dll
import abi

# Uncomment the two line bellow to debug this code
# print("Waiting for Visual Python debugger to attach")
# import ptvsd; ptvsd.enable_attach(); ptvsd.wait_for_attach()

ABI = abi.detect_abi()

TCL_OK = 0
TCL_ERROR = 1


def _create_native_dispatch(my_abi: abi.Abi):
    return _native.NativeFunction(my_abi[0], my_abi[1], my_abi[2], my_abi[3])

class TclCache_get:
    def __init__(self):
        func = _create_native_dispatch(ABI[type(self).__name__])
        arg_count = abi.arg_count(ABI[type(self).__name__])

        if arg_count == 2:
            self._func = lambda _, b, c, _2: func(b, c)
        else:
            self._func = func

    def __call__ (self, tcl_cache: int, code_out, filename: str, run_preprocessor):
        filename_as_bytes = ctypes.create_string_buffer(filename.encode("ascii"))
        raw_Tcl_Obj = ctypes.c_int(0)
        self._func(tcl_cache, ctypes.addressof(raw_Tcl_Obj), ctypes.addressof(filename_as_bytes), run_preprocessor)
        code = cpptcl.object_.reinterpret_cast(raw_Tcl_Obj.value)
        code_out.swap(code)

TclCache_get = TclCache_get()
g_tclCache = ctypes.cast(ABI['g_tclCache'][0], ctypes.POINTER(ctypes.c_int))
g_tclCache: int = g_tclCache[0]


def sTclObject_loadTclFile(code_out, filename: str, run_preprocessor, crc_flags):
    if g_tclCache:
        TclCache_get(g_tclCache, code_out, filename, run_preprocessor)
    else:
        sTclObject_preprocessFile(code_out, filename, run_preprocessor, crc_flags)
    return code_out


class sTclObject_preprocessFile:
    def __init__(self):
        func = _create_native_dispatch(ABI[type(self).__name__])
        arg_count = abi.arg_count(ABI[type(self).__name__])

        if arg_count == 2:
            self._func = lambda a, b, _1, _2: func(a, b)
        else:
            self._func = func

    def __call__ (self, code_out, filename: str, run_preprocessor, crc_flags):
        filename_as_bytes = ctypes.create_string_buffer(filename.encode("ascii"))
        raw_Tcl_Obj = ctypes.c_int(0)
        self._func(ctypes.addressof(raw_Tcl_Obj), ctypes.addressof(filename_as_bytes), run_preprocessor, crc_flags)
        code = cpptcl.object_.reinterpret_cast(raw_Tcl_Obj.value)
        code_out.swap(code)

sTclObject_preprocessFile = sTclObject_preprocessFile()

class BuiltinTclData(ctypes.Structure):
    _fields_ = [
        ("interp", ctypes.c_void_p),
        ("owning", ctypes.c_char),
        ("long_error_desc", ctypes.c_void_p),
        ("short_error_desc", ctypes.c_void_p),
    ]
 
class StringRef(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("refCount", ctypes.c_int),
        ("buffer_size", ctypes.c_int),
        ("length", ctypes.c_int),
    ]

class ValueReference:
    def __init__(self, value=None):
        self._value = None
    
    @property
    def value(self):
        return self._value
        
    @value.setter
    def value(self, value):
        self._value = value
        

class CObj_tclEval:
    def __init__(self):
        self._func = _create_native_dispatch(ABI[type(self).__name__])

    def __call__ (self, issf, code_in):
        return self._func(ctypes.addressof(issf), cpptcl.object_.addressof(code_in))

CObj_tclEval = CObj_tclEval()


class BuiltinTclData_ctor_Tcl_Interp:
    def __init__(self):
        self._func = _create_native_dispatch(ABI[type(self).__name__])

    def __call__ (self, issf, interp):
        return self._func(ctypes.addressof(issf), cpptcl.interpreter.reinterpret_cast(interp))

BuiltinTclData_ctor_Tcl_Interp = BuiltinTclData_ctor_Tcl_Interp()


class findFileLocation:
    def __init__(self):
        self._func = _create_native_dispatch(ABI[type(self).__name__])

    def __call__ (self, fileLocation: ValueReference, filename: str) -> str:
        filename_as_bytes = ctypes.create_string_buffer(filename.encode("ascii"))
        raw_string_ref = ctypes.c_int(0)
        self._func(ctypes.addressof(raw_string_ref), ctypes.addressof(filename_as_bytes))

        s = ctypes.cast(raw_string_ref.value, ctypes.POINTER(StringRef)) # TODO call destructor for StringRef this will leak memory
        if s[0].length > 0:
            char_str = ctypes.cast(raw_string_ref.value + 12, ctypes.c_char_p)
            fileLocation.value = char_str.value.decode('ascii')
        
        return fileLocation.value


findFileLocation = findFileLocation()


def func(clientdata, interp, objc, objv):
    i = cpptcl.interpreter.reinterpret_cast(interp)
    
    v = ctypes.cast(objv, POINTER(c_int))

    ARG_COUNT = 2
    if objc < ARG_COUNT:
        command = []
        for idx in range(objc):
            command.append(str(cpptcl.object_.reinterpret_cast(v[idx])))
        command = ' '.join(command)
        i.setResult(cpptcl.object_(f"invalid arg count: {command} ({ARG_COUNT} expected)"))
        return TCL_ERROR

    issf = BuiltinTclData()
    BuiltinTclData_ctor_Tcl_Interp(issf, i) # interpreter is not owned by us

    func_name = str(cpptcl.object_.reinterpret_cast(v[0]))
    filename = str(cpptcl.object_.reinterpret_cast(v[1]))
    original_filename = filename

    if not filename:
        i.setResult(cpptcl.object_(f"call filename was empty !"))
        return TCL_ERROR
    
    filename = filename.lower()

    if filename.startswith("scripts/"):
        filename = "data/" + filename
    elif filename.startswith("templates/"):
        filename = "data/" + filename

    code = cpptcl.object_()

    try:
        if not filename.startswith("data/templates"):
            sTclObject_loadTclFile(code, filename, 1, 0)
        else:
            sTclObject_preprocessFile(code, filename, 1, 0)
    except ValueError as exc:
        try:
            file_location = ValueReference('')
            findFileLocation(file_location, filename)
            filename = file_location.value.lower()

            if not filename.startswith("data/templates"):
                sTclObject_loadTclFile(code, filename, 1, 0)
            else:
                sTclObject_preprocessFile(code, filename, 1, 0)
        except ValueError as exc:
            i.setResult(cpptcl.object_(f"file not found.\n{original_filename}"))
            return TCL_OK # I don't know why OK is returned

    is_ok = CObj_tclEval(issf, code)
    if not is_ok:
        return TCL_ERROR
    # TODO call issf destructor
    return TCL_OK

mock = _native.createNativeToPythonFunction(ABI["script_call"][0], ABI["script_call"][1], ABI["script_call"][2], ABI["script_call"][3], func, False)

# This is the entry point to the native function via trampoline hook
mock_address = mock.getTrampolineAddress()
# This would call the native function
script_call = _native.NativeFunction(mock_address, ABI["script_call"][1], ABI["script_call"][2], ABI["script_call"][3])
