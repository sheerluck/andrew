def builtin_symbolic_function_list():
    from sage.symbolic.expression import symbol_table
    from sage.symbolic.function import BuiltinFunction
    fdict = symbol_table['functions']
    return [c for c in sorted(fdict.values(), key=str)
            if isinstance(c, BuiltinFunction)]


for c in builtin_symbolic_function_list():
    argl = [2.5 for n in range(c.number_of_arguments())]
    print(c, argl)
    try:
        print(CC(c(*argl)))   
    except:
        print("~~~~~~~~~~~~")
    print()  
