from inspect import signature, Parameter  # Python >= 3.3
from functools import wraps

def callback_prototype(prototype):
    protosig = signature(prototype)
    positional, keyword = [], []
    for name, param in protosig.parameters.items():
        if param.kind in (Parameter.VAR_POSITIONAL, Parameter.VAR_KEYWORD):
            raise TypeError("*args/**kwargs not supported in prototypes")

        if (param.default is not Parameter.empty) \
            or (param.kind == Parameter.KEYWORD_ONLY):
            keyword.append(name)
        else:
            positional.append(name)
        
    kwargs = dict.fromkeys(keyword)
    def adapt(callback):
        """Introspect and prepare a third party callback."""
        sig = signature(callback)
        try:
            sig.bind(*positional, **kwargs)
            return callback
        except TypeError:
            pass
        
        unmatched_pos = positional[:]
        unmatched_kw = kwargs.copy()
        unrecognised = []
        for name, param in sig.parameters.items():
            if param.kind == Parameter.POSITIONAL_ONLY:
                if len(unmatched_pos) > 0:
                    unmatched_pos.pop(0)
                else:
                    unrecognised.append(name)
            elif param.kind == Parameter.POSITIONAL_OR_KEYWORD:
                if (param.default is not Parameter.empty) and (name in unmatched_kw):
                    unmatched_kw.pop(name)
                elif len(unmatched_pos) > 0:
                    unmatched_pos.pop(0)    
                else:
                    unrecognised.append(name)
            elif param.kind == Parameter.VAR_POSITIONAL:
                unmatched_pos = []
            elif param.kind == Parameter.KEYWORD_ONLY:
                if name in unmatched_kw:
                    unmatched_kw.pop(name)
                else:
                    unrecognised.append(name)
            else:  # VAR_KEYWORD
                unmatched_kw = {}
        
        if unrecognised:
            raise TypeError("Function {!r} had unmatched arguments: {}".format(callback, unrecognised))

        n_positional = len(positional) - len(unmatched_pos)

        @wraps(callback)
        def adapted(*args, **kwargs):
            """Wrapper for third party callbacks that discards excess arguments"""
            args = args[:n_positional]
            for name in unmatched_kw:
                kwargs.pop(name)
            return callback(*args, **kwargs)
        
        return adapted

    prototype.adapt = adapt
    return prototype
