#!/usr/bin/python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import subprocess
import sys


objdump = "/usr/bin/objdump"
symbolizer = ""
hermes = ""


def loadNativeCallStats():
    """
    Parse the native stats dumped by Hermes from stdin. Return a tuple of
    (data_rows, name_of_base_function)
    """
    rows = []
    baseOffsetLinePrefix = "Native call base offset "
    baseName = ""
    for l in sys.stdin:
        if l.startswith(baseOffsetLinePrefix):
            baseName = l.rstrip()[len(baseOffsetLinePrefix) :]
            # print("found base name (%s)" % baseOffsetName)
            continue

        toks = l.split()
        if (len(toks) != 5) or (toks[4] != "NativeCall"):
            continue
        rows.append(toks)

    if not baseName:
        print("Could not extract base offset function")
        exit(1)
    return (rows, baseName)


def findOffset(name):
    """Find the offset of the specified mangled function name"""
    s = subprocess.Popen([objdump, "-t", hermes], stdout=subprocess.PIPE)

    offset = 0
    for l in s.stdout:
        if l.find(name) != -1:
            offset = int(l.split()[0], 16)

    s.wait()

    if not offset:
        print("Could not find offset of function %s in %s" % (name, hermes))
        exit(1)

    return offset


def symbolicate(rows, baseOffset):
    """Symbolicate and print the updated table, which is modified inplace"""
    s = subprocess.Popen(
        [symbolizer, "--obj=" + hermes], stdout=subprocess.PIPE, stdin=subprocess.PIPE
    )

    for toks in rows:
        addr = int(toks[3], 16)
        s.stdin.write(hex(addr + baseOffset) + "\n")

    s.stdin.close()

    index = 0
    skipCnt = 0
    for l in s.stdout:
        # The symbolizer outputs three lines per function. We only care about the
        # first line.
        skipCnt += 1
        if (skipCnt % 3) != 1:
            continue
        toks = rows[index]
        rows[index] = (toks[0], toks[1], toks[2], l.rstrip())
        index += 1

    s.wait()

    print("Count   Total   Per-call Function")
    print("====== ======== ======== ============")
    for tok in rows:
        print("%6s %8s %8s %s" % (tok[0], tok[1], tok[2], tok[3]))


def main(argv):
    global symbolizer, hermes, objdump
    if len(argv) < 3 or len(argv) > 4:
        print(
            "Syntax: %s path-to-llvm-symbolizer path-to-hermes [path-to-objdump]"
            % argv[0]
        )
        exit(1)

    symbolizer = argv[1]
    hermes = argv[2]
    if len(argv) == 4:
        objdump = argv[3]

    rows, baseOffsetName = loadNativeCallStats()
    baseOffset = findOffset(baseOffsetName)
    symbolicate(rows, baseOffset)


main(sys.argv)
