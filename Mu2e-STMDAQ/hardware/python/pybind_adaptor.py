import runpy, sys

def run_script(script, args):
    args = list(args)
    sys.argv = [script] + [str(a) for a in args]

    runpy.run_path(script, run_name="__main__")
    return None

def run_and_get(script, args, var="result"):
    # Ensure argv is a list of strings
    args = list(args)
    sys.argv = [script] + [str(a) for a in args]

    # if var="":
    #     runpy.run_path(script, run_name="__main__")
    #     return None
    # else:
    g = runpy.run_path(script, run_name="__main__")
    
    if var not in g:
        # Fail loudly: script didn't define it
        raise KeyError(
            f"{script} did not define global '{var}'. "
            f"Defined globals include: {', '.join(sorted(g.keys())[:30])} ..."
        )
    
    val = g[var]
    if val is None:
        raise ValueError(f"{script} set '{var}' to None")
    
    return val
