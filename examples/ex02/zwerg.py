# import ptvsd; ptvsd.enable_attach(); ptvsd.wait_for_attach()

import pytcl
tcl = pytcl.Tcl()


class SetCapture():
    def __init__(self, globls):
        self._globals = globls

    def __call__(self, name1, name2, op):
        print(tcl.get_objname('this'), name1, '=', getattr(self._globals, name1))


class _Detour():
    def __init__(self, name, detail_name):
        self._name = name
        self._detail_name = detail_name

    def __call__(self, *args):
        func = pytcl.CallProc(self._detail_name, autocast_list=False)
        p_args = [str(x) for x in args]

        res = func(*args)
        print(tcl.get_objname('this'), f'{self._name} ' + ', '.join(p_args), '->', res)
        return res


def trace_all_globals():
    globs = tcl.globals
    all_globals = tcl.info('globals')
    for var_name in all_globals:
        tracer_proc_name = f'::traceing::set::{var_name}'
        tcl.proc(tracer_proc_name, SetCapture(globs))
        tcl.trace('variable', var_name, 'w', tracer_proc_name)


def untrace_all_globals():
    all_globals = tcl.info('globals')
    for name in all_globals:
        for tracer in tcl.trace('vinfo', name):
            ops, command = tracer
            tcl.trace('vdelete', name, ops, command)


def listen_to_all_procedures():
    all_procedures = tcl.info('procs')
    for proc_name in all_procedures:
        details_proc_name = f'::details::{proc_name}'
        tcl.rename(proc_name, details_proc_name)
        tcl.proc(proc_name, _Detour(proc_name, details_proc_name))

    trace_all_globals()
