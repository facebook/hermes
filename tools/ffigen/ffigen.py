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
}

# Initialize a dictionary to hold the dynamic mappings
dynamic_type_mapping = {}

# Get the filename from the command line arguments
if len(sys.argv) < 2:
    print("Usage: ffigen.py <filename>")
    sys.exit(1)
filename = sys.argv[1]

# Read XML tree either from the file or by running castxml
if filename.endswith(".xml"):
    root = read_xml_from_file(filename)
else:
    root = read_xml_from_castxml(filename)

# Find all types and their IDs (Typedef and FundamentalType)
for typedef in root.findall(".//Typedef"):
    type_id = typedef.get("id")
    type_name = typedef.get("name")
    if type_name in simple_type_mapping:
        dynamic_type_mapping[type_id] = simple_type_mapping[type_name]

for fundamental_type in root.findall(".//FundamentalType"):
    type_id = fundamental_type.get("id")
    type_name = fundamental_type.get("name")
    if type_name in simple_type_mapping:
        dynamic_type_mapping[type_id] = simple_type_mapping[type_name]

# Any pointer type will be mapped to c_ptr
for pointer in root.findall(".//PointerType"):
    type_id = pointer.get("id")
    dynamic_type_mapping[type_id] = "c_ptr"

# Generate JS declarations
for func in root.findall(".//Function"):
    arg_count = 0
    name = func.get("name")
    return_type = dynamic_type_mapping.get(func.get("returns"), "UNKNOWN_TYPE")
    args = []
    for arg in func.findall(".//Argument"):
        arg_count += 1
        arg_name = arg.get("name")
        arg_type = dynamic_type_mapping.get(arg.get("type"), "UNKNOWN_TYPE")
        if arg_name:
            args.append(f"{arg_name}: {arg_type}")
        else:
            args.append(f"_{arg_count}: {arg_type}")
    args_str = ", ".join(args)
    js_declaration = (
        "const _"
        + name
        + " = $SHBuiltin.extern_c({}, function "
        + name
        + "("
        + args_str
        + "): "
        + return_type
        + " { throw 0; });"
    )
    print(js_declaration)

# Generate JS constants from the ordered dictionary
if len(constants_dict):
    print()
for name, value in constants_dict.items():
    print(f"const _{name} = {value};")
