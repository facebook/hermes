#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
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
import io
from os import path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    with io.open(args.files[0], "w", encoding="utf-8") as o:

        o.write("({")
        for arg in args.files[1:]:
            # Ensure the file exists before writing out anything
            if not path.exists(arg):
                raise Exception('File "{}" doesn\'t exist'.format(arg))

            # Extracts the filename out of the full path and excludes the ".js" extension
            o.write(
                '"{filename}" : (function(exports, require, module, internalBinding, __filename, __dirname) {{'.format(
                    filename=arg[
                        arg.index("nodelib") + len("nodelib") + 1 : -len(".js")
                    ].replace("\\", "/")
                )
            )
            with io.open(arg, "r", encoding="utf-8") as f:
                o.write(f.read())
            o.write("}),")

        o.write("});")


if __name__ == "__main__":
    main()
