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


def _filename_from_arg(wrapper_path):
    """Extracts the wrapper filename out of wrapper_path, stripping the .js extension."""

    wrapper_path = wrapper_path.replace("\\", "/")
    return wrapper_path[
        wrapper_path.index("/wrappers/") + len("/wrappers/") : -len(".js")
    ]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    with io.open(args.files[0], "w", encoding="utf-8") as o:

        o.write("({")
        for arg in args.files[1:]:
            arg = path.abspath(arg)

            # Ensure the file exists before writing out anything
            if not path.exists(arg):
                raise Exception('File "{}" doesn\'t exist'.format(arg))

            o.write(
                f'"{_filename_from_arg(arg)}" : (function(exports, require, module, internalBinding, __filename, __dirname) {{'
            )
            with io.open(arg, "r", encoding="utf-8") as f:
                o.write(f.read())
            o.write("}),")

        o.write("});")


if __name__ == "__main__":
    main()
