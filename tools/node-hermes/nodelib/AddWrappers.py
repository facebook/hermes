#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""
The purpose of this script is to add the module wrappers around the
contents of each of the javascript files located in this directory,
for the purposes of being able to compile them down to bytecode with
the wrappers present.

This script is intended to be compliant with both py2 and py3.
"""

import argparse
from os import path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    print("({")
    for arg in args.files:
        # Ensure the file exists before writing out anything
        if not path.exists(arg):
            raise Exception('File "{}" doesn\'t exist'.format(arg))

        # Extracts the filename out of the full path and excludes the ".js" extension
        print(
            '"{filename}" : (function(exports, require, module, __filename, __dirname) {{'.format(
                filename=arg[
                    arg.index("nodelib") + len("nodelib") + 1 : -len(".js")
                ].replace("\\", "/")
            )
        )
        with open(arg, "r") as f:
            print(f.read())
        print("}),")

    print("});")


if __name__ == "__main__":
    main()
