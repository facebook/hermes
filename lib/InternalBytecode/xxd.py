#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

"""
This module exists so that a Windows build can have access to functionality
like xxd -i
"""

import argparse
from os import path


# This is about 80 characters long (4 characters per byte + comma + space)
BYTES_PER_LINE = 12


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    args = parser.parse_args()

    # Ensure the file exists before writing out anything
    if not path.exists(args.file):
        raise Exception(f'File "{args.file}" doesn\'t exist')

    with open(args.file, "rb") as f:
        # Could read in chunks instead for extra performance, but this script
        # isn't meant to be used on gigantic files.
        file_as_bytes = f.read()

    # Make groups of bytes that fit in a single line
    lines = [
        file_as_bytes[i : i + BYTES_PER_LINE]
        for i in range(0, len(file_as_bytes), BYTES_PER_LINE)
    ]
    print(",\n".join(", ".join("0x{:02x}".format(b) for b in l) for l in lines))


if __name__ == "__main__":
    main()
