#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import sys


""" Reads a file from stdin, and outputs to stdout JavaScript source that
    constructs a variable containing that file, whose name is given in argv[1].

For example, if the input is:

  print("hello world");

Then `stringize.py MYVAR` will be

  var MYVAR = 'print("hello world");'

"""

if len(sys.argv) != 2:
    print("Usage: stringize.py <VARNAME>")
    sys.exit(-1)
name = sys.argv[1]

lines = sys.stdin.readlines()
for idx, val in enumerate(lines):
    line = val.strip("\r\n")
    line = line.replace("\\", "\\\\").replace("'", "\\'")
    maybe_newline = "\\n" if idx + 1 < len(lines) else ""
    if idx == 0:
        print("var %s = '%s%s';" % (name, line, maybe_newline))
    else:
        print("%s += '%s%s';" % (name, line, maybe_newline))
