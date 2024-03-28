#!/usr/bin/env python3

def demangle_name(mangled_name):
    if not mangled_name.startswith("_JF"):
        raise ValueError("Invalid mangled name format")

    mangled_name = mangled_name[3:]
    is_static = False

    if mangled_name.startswith("C"):
        is_static = True
        mangled_name = mangled_name[1:]

    parts = []
    while mangled_name:
        if mangled_name[0] == "E":
            mangled_name = mangled_name[1:]
            break

        length = int(mangled_name[0])
        part = mangled_name[1:length + 1]
        parts.append(part)
        mangled_name = mangled_name[length + 1:]

    canonical_name = ".".join(parts)

    return_type, mangled_name = demangle_type(mangled_name)

    param_types = []
    while mangled_name:
        param_type, mangled_name = demangle_type(mangled_name)
        param_types.append(param_type)

    function_name = f"{canonical_name}({', '.join(param_types)})"
    if is_static:
        function_name = "static " + function_name

    return f"{return_type} {function_name}"

def demangle_type(name):
    type_char = name[0]
    if type_char == "B":
        return "boolean", name[1:]
    elif type_char == "c":
        return "char", name[1:]
    elif type_char == "s":
        return "short", name[1:]
    elif type_char == "i":
        return "int", name[1:]
    elif type_char == "b":
        return "byte", name[1:]
    elif type_char == "S":
        return "String", name[1:]
    elif type_char == "O":
        return "Object", name[1:]
    elif type_char == "v":
        return "void", name[1:]
    elif type_char == "A":
        array_type, remaining = demangle_type(name[1])
        return f"{array_type}[]", remaining
    elif type_char == "R":
        parts = []
        remaining = name[1:]
        while remaining:
            if remaining[0] == "E":
                remaining = remaining[1:]
                break

            length = int(remaining[0])
            part = remaining[1:length + 1]
            parts.append(part)
            remaining = remaining[length + 1:]

        canonical_name = ".".join(parts)
        return canonical_name, remaining

    raise ValueError(f"Invalid type: {name}")

print(demangle_name(input('> ')))
