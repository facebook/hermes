#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from collections import OrderedDict

# Initialize a global ordered dictionary to hold the constants
constants_dict = OrderedDict()


def read_xml_from_castxml(filename):
    # Read the file and look for #define statements
    with open(filename, "r") as f:
        for line in f:
            match = re.match(r"\s*#\s*define\s+(\w+)\s+(\d+)\s*(?:/\*|//|$)", line)
            if match:
                name, number = match.groups()
                constants_dict[name] = number

    # Run castxml
    xml_output = subprocess.run(
        ["castxml", "--castxml-gccxml", "-o", "-", filename],
        capture_output=True,
        text=True,
    ).stdout

    return ET.fromstring(xml_output)


def read_xml_from_file(filename):
    return ET.parse(filename).getroot()


# A simple mapping of common C types to their Static Hermes equivalents.
simple_type_mapping = {
    "char": "c_char",
    "signed char": "c_schar",
    "unsigned char": "c_uchar",
    "short int": "c_short",
    "short unsigned int": "c_ushort",
    "int": "c_int",
    "unsigned int": "c_uint",
    "long int": "c_long",
    "long unsigned int": "c_ulong",
    "long long int": "c_longlong",
    "long long unsigned int": "c_ulonglong",
    "bool": "c_bool",
    "float": "c_float",
    "void": "void",
    "double": "c_double",
}

# Mapping from the ID for a type to its XML element.
id_to_element = {}

# Mapping from the ID for a type to the corresponding type name in SH.
def to_sh_name(id):
    elem = id_to_element[id]

    if elem.tag == "FundamentalType":
        return simple_type_mapping.get(elem.get("name"))
    if elem.tag == "Enumeration":
        return "c_int"
    if elem.tag == "PointerType":
        return "c_ptr"
    if elem.tag == "ArrayType":
        return "c_ptr"

    # Structs and unions are converted to be passed as pointers.
    if elem.tag == "Struct":
        return "c_ptr"
    if elem.tag == "Union":
        return "c_ptr"

    return "UNKNOWN_TYPE"


# Mapping from the ID for a type to the corresponding type name in C.
def to_c_name(id):
    elem = id_to_element[id]

    if elem.tag == "FundamentalType":
        return elem.get("name")
    if elem.tag == "Enumeration":
        return elem.get("name")
    if elem.tag == "PointerType":
        return to_c_name(elem.get("type")) + "*"
    if elem.tag == "FunctionType":
        return "void"
    if elem.tag == "Struct":
        return "struct " + elem.get("name")
    if elem.tag == "Union":
        return "union " + elem.get("name")
    if elem.tag == "ArrayType":
        return to_c_name(elem.get("type")) + "*"

    return "UNKNOWN_TYPE"


# Whether a type needs to be wrapped and passed by pointer.
def need_cwrap(id):
    return id_to_element[id].tag == "Struct" or id_to_element[id].tag == "Union"


# Get the filename from the command line arguments
if len(sys.argv) < 3:
    print("Usage: ffigen.py <cwrap|js> <filename> [filter filenames]")
    print(
        "Last argument optionally provides an allow-list of files from which to export types and functions."
    )
    print("Example: ffi_gen.py js foo.h foo.h,bar.h")
    sys.exit(1)
mode = sys.argv[1]
filename = sys.argv[2]

# Read XML tree either from the file or by running castxml
if filename.endswith(".xml"):
    root = read_xml_from_file(filename)
else:
    root = read_xml_from_castxml(filename)

# Set of file IDs from which to export structs and functions.
allowed_file_ids = set()

# If no filter was provided, include all files.
if len(sys.argv) == 3:
    for file in root.findall(".//File"):
        allowed_file_ids.add(file.get("id"))
else:
    filter_names = sys.argv[3].split(",")
    for file in root.findall(".//File"):
        for substr in filter_names:
            if substr in file.get("name"):
                allowed_file_ids.add(file.get("id"))


# Collect all of the type declarations.
for tag in [
    "Struct",
    "Enumeration",
    "FundamentalType",
    "PointerType",
    "Union",
    "ArrayType",
    "CvQualifiedType",
    "FunctionType",
]:
    for elem in root.findall(".//" + tag):
        id_to_element[elem.get("id")] = elem

# Collect typedefs and associate their id with the underlying type.
for typedef in root.findall(".//Typedef"):
    type_id = typedef.get("id")
    id_to_element[type_id] = id_to_element[typedef.get("type")]

# Map CvQualifiedTypes to their underlying type. We don't support qualifiers yet.
for cv in root.findall(".//CvQualifiedType"):
    id_to_element[cv.get("id")] = id_to_element[cv.get("type")]

if mode == "js":
    # Generate JS declarations
    for func in root.findall(".//Function"):
        # Skip functions that are not in the allow-list.
        if func.get("file") not in allowed_file_ids:
            continue

        name = func.get("name")
        args = []

        # If the return value is a struct or union, return void and add an out
        # parameter.
        return_cwrap = need_cwrap(func.get("returns"))
        if return_cwrap:
            return_type = "void"
            args.append(f"_out: c_ptr")
        else:
            return_type = to_sh_name(func.get("returns"))

        # If the return value or any argument needs wrapping, then generate
        # a wrapper.
        do_cwrap = return_cwrap

        for arg in func.findall(".//Argument"):
            if need_cwrap(arg.get("type")):
                do_cwrap = True

            # Emit each parameter in the SH signature.
            arg_name = arg.get("name")
            arg_type = to_sh_name(arg.get("type"))
            if arg_name:
                args.append(f"_{arg_name}: {arg_type}")
            else:
                args.append(f"_{len(args)}: {arg_type}")

        js_declaration = (
            "const _"
            + name
            + " = $SHBuiltin.extern_c({}, function "
            + name
            + ("_cwrap" if do_cwrap else "")
            + "("
            + ", ".join(args)
            + "): "
            + return_type
            + " { throw 0; });"
        )
        print(js_declaration)

    # Generate struct and union sizes.
    for type in ["Struct", "Union"]:
        for elem in root.findall(".//" + type):
            if elem.get("file") not in allowed_file_ids:
                continue
            name = elem.get("name")
            size = elem.get("size")
            if name and size:
                size = int(size) // 8
                print(f"const _sizeof_{name} = {size};")

    # Generate struct accessors
    for field in root.findall(".//Field"):
        if field.get("file") not in allowed_file_ids:
            continue
        field_name = field.get("name")
        field_type_id = field.get("type")
        field_type_elem = id_to_element[field_type_id]

        struct_name = id_to_element[field.get("context")].get("name")
        sh_type = to_sh_name(field_type_id)

        offset = int(field.get("offset")) // 8

        # If the field is a struct, union, or array, return a pointer to
        # it and do not emit a setter.
        if (
            field_type_elem.tag == "Array"
            or field_type_elem.tag == "Union"
            or field_type_elem.tag == "Struct"
        ):
            print(f"function get_{struct_name}_{field_name}(s: c_ptr): {sh_type} {{")
            print(f'  "inline";\n  return _sh_ptr_add(s, {offset});\n}}')
        else:
            print(f"function get_{struct_name}_{field_name}(s: c_ptr): {sh_type} {{")
            print(f'  "inline";\n  return _sh_ptr_read_{sh_type}(s, {offset});\n}}')

            print(
                f"function set_{struct_name}_{field_name}(s: c_ptr, v: {sh_type}): void {{"
            )
            print(f'  "inline";\n  _sh_ptr_write_{sh_type}(s, {offset}, v);\n}}')

    # Generate JS constants from the ordered dictionary
    if len(constants_dict):
        print()
    for name, value in constants_dict.items():
        print(f"const _{name} = {value};")

    # Generate JS constants from enum values.
    for enum in root.findall(".//Enumeration"):
        if elem.get("file") not in allowed_file_ids:
            continue
        for enum_value in enum.findall(".//EnumValue"):
            enum_value_name = enum_value.get("name")
            enum_value_value = enum_value.get("init")
            print(f"const _{enum_value_name} = {enum_value_value};")
elif mode == "cwrap":
    print("#include <stdbool.h>\n")

    # Generate C declarations
    for func in root.findall(".//Function"):
        return_id = func.get("returns")

        # C declaration is only needed if the function either accepts or returns a
        # struct by value.
        do_cwrap = need_cwrap(return_id)
        for arg in func.findall(".//Argument"):
            if need_cwrap(arg.get("type")):
                do_cwrap = True
        if not do_cwrap:
            continue

        func_name = func.get("name")

        # The parameters that the wrapper accepts.
        params = []

        # The body of the generated wrapper.
        body = ""

        # If the return type requires wrapping, add an out parameter and
        # populate it with the result of the call.
        if need_cwrap(return_id):
            return_type = "void"
            params.append(to_c_name(return_id) + f"* a0")
            body += "*a0 = "
        else:
            return_type = to_c_name(return_id)
            body += "return "

        body += func_name + "("

        # The arguments to the call to the wrapped function.
        args = []
        for arg in func.findall(".//Argument"):
            n = len(params)
            # If the parameter type requires wrapping, accept it as a pointer
            # and dereference it.
            if need_cwrap(arg.get("type")):
                params.append(to_c_name(arg.get("type")) + f"* a{n}")
                args.append(f"*a{n}")
            else:
                params.append(to_c_name(arg.get("type")) + f" a{n}")
                args.append(f"a{n}")

        body += ", ".join(args)
        body += ");\n"

        print(
            f"{return_type} {func_name}_cwrap({', '.join(params)}){{\n  " + body + "}"
        )
