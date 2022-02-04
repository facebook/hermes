#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os.path
import re
import sys


""" Reads a file from two files, and check if they are equal ignoring
full path and column number.
"""

if len(sys.argv) != 3:
    print("Usage: stacktrace_diff.py <trace1> <trace2>")
    sys.exit(-1)
base_trace = sys.argv[1]
compare_trace = sys.argv[2]

base_lines = open(base_trace).readlines()
compare_lines = open(compare_trace).readlines()

if len(base_lines) != len(compare_lines):
    sys.stderr.write("Number of lines mismatch\n")
    sys.exit(1)

for idx, base_line in enumerate(base_lines):
    compare_line = compare_lines[idx]
    if idx == 0:
        if base_line != compare_line:
            sys.stderr.write("Exception message mismatch\n")
            sys.exit(1)
    else:
        pattern = r"\s*at (\w+) \(([^\\]*\.js):(\d+):\d+\)\s*"
        base_match = re.match(pattern, base_line)
        if not base_match:
            sys.stderr.write(
                "base line {0} does not match expected format\n".format(base_line)
            )
            sys.exit(1)

        compare_match = re.match(pattern, compare_line)
        if not compare_match:
            sys.stderr.write("compare line does not match expected format\n")
            sys.exit(1)

        if base_match.group(1) != compare_match.group(1):
            sys.stderr.write("function name does not match\n")
            sys.exit(1)

        base_file_name = os.path.basename(base_match.group(2))
        compare_file_name = os.path.basename(compare_match.group(2))
        if base_file_name != compare_file_name:
            sys.stderr.write("File name does not match\n")
            sys.exit(1)

        if base_match.group(3) != compare_match.group(3):
            sys.stderr.write("line number does not match\n")
            sys.exit(1)
