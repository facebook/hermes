#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import print_function

import os
import re
import sys


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


baseName = ""
headingLines = []
sourceLines = {}
lastSourceLine = 0
errors = []


def parseErrorLine(line):
    m = re.match(r".*:(\d+):(\d+): (.*)", line)
    if not m:
        return None
    return (int(m.group(1)), int(m.group(2)), m.group(3))


def parseSource(lines):
    global lastSourceLine
    i = 0
    # Collect the commented out lines at the beginning.
    while i < len(lines):
        if not re.match(r"^\s*//|^$|^\/\*|^ \*", lines[i]):
            break
        headingLines.append(lines[i])
        i += 1
    # Collect the source, exlcuding // CHECK: and similar lines.
    while i < len(lines):
        if not re.match(r"^\s*//\s*(\S+):", lines[i]):
            if lines[i].strip():
                sourceLines[i + 1] = lines[i]
                lastSourceLine = i + 1
        i += 1


def parseErrors(lines):
    i = 0
    lastError = None
    while i < len(lines):
        errDesc = parseErrorLine(lines[i])
        if errDesc:
            if lastError:
                errors.append(lastError)
            lastError = (errDesc, [])
        else:
            if lastError:
                lastError[1].append(lines[i])
            else:
                eprint("ignored error line", lines[i])
        i += 1
    if lastError:
        errors.append(lastError)
    list.sort(errors, key=lambda error: error[0][0])


def generateOutput(f):
    lineNo = 0
    for hl in headingLines:
        print(hl, file=f)
        lineNo += 1

    prevErrorLine = lineNo

    errorIndex = 0
    while errorIndex < len(errors):
        origErrorLine = errors[errorIndex][0][0]

        # Detect all errors on the same line
        nextErrorIndex = errorIndex + 1
        while (
            nextErrorIndex < len(errors)
            and errors[nextErrorIndex][0][0] == origErrorLine
        ):
            nextErrorIndex += 1

        # Output all source up to and including the line of the error, but skip empty lines
        # in the beginning and end.

        # Skip empty lines in the beginning.
        fromLine = prevErrorLine + 1
        prevErrorLine = origErrorLine
        while fromLine < origErrorLine and not sourceLines.get(fromLine, ""):
            fromLine += 1
        # Skip empty lines at the end.
        toLine = origErrorLine
        while toLine > fromLine and not sourceLines.get(toLine, ""):
            toLine -= 1

        # Output a separator line, but not the first time.
        if errorIndex:
            print("", file=f)
            lineNo += 1

        # Output the source lines.
        for i in range(fromLine, toLine + 1):
            print(sourceLines.get(i, ""), file=f)
            lineNo += 1
        newErrorLine = lineNo

        while errorIndex < nextErrorIndex:
            error = errors[errorIndex]
            errorIndex += 1
            print(
                "//CHECK: {{.*}}%s:%d:%d: %s"
                % (baseName, newErrorLine, error[0][1], error[0][2]),
                file=f,
            )
            lineNo += 1
            for eline in error[1]:
                print("//CHECK-NEXT:", eline, file=f)
                lineNo += 1

    # Output the rest of the source.
    fromLine = prevErrorLine + 1
    while fromLine < lastSourceLine and not sourceLines.get(fromLine, ""):
        fromLine += 1
    if fromLine <= lastSourceLine:
        print("", file=f)
        lineNo += 1
    for i in range(fromLine, lastSourceLine + 1):
        print(sourceLines.get(i, ""), file=f)
        lineNo += 1


def printHelp():
    print("syntax: updateErrorTest.py [options] input-file")
    print("  -h  help")
    print("  -i  replace the input file inplace")


def main(argv):
    argI = 1

    inputName = ""
    inplace = False
    while argI < len(argv):
        if argv[argI] == "-h":
            printHelp()
            sys.exit()
        elif argv[argI] == "-i":
            inplace = True
        elif argv[argI].startswith("-"):
            eprint("Invalid option '%s'. -h for help." % argv[argI])
            sys.exit(1)
        else:
            if inputName:
                eprint("More than one input-file specified")
                sys.exit(1)
            inputName = argv[argI]
        argI += 1

    if not inputName:
        eprint("Input-file not specified. -h for help.")
        sys.exit(1)

    global baseName
    baseName = os.path.basename(inputName)

    # Read the source file
    with open(inputName, "r") as f:
        source = [x.strip("\n") for x in f.readlines()]

    # Read the error output
    errorOutput = [x.strip("\n") for x in sys.stdin.readlines()]

    parseSource(source)
    parseErrors(errorOutput)

    eprint("Scanned %d source lines and %d errors" % (len(sourceLines), len(errors)))

    if inplace:
        with open(inputName, "w") as f:
            generateOutput(f)
    else:
        generateOutput(sys.stdout)


if __name__ == "__main__":
    main(sys.argv)
