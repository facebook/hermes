# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Test expectation generation.

Use this script for regenerating the expectations in tests that use the
%FileCheckOrRegen lit substitution.
"""

import argparse
import os
import re
import sys

_ES5_FUNCTION = "function"
_ES6_CONSTRUCTOR = "constructor"
_ES6_ARROW = "arrow"
_ES6_ARROW_FUNCTION = "arrow function"
_ES6_METHOD = "method"

_DUMP_FUNCTION_START_MARKERS = [
    _ES5_FUNCTION,
    _ES6_CONSTRUCTOR,
    _ES6_ARROW,
    _ES6_ARROW_FUNCTION,
    _ES6_METHOD,
]

_DUMP_FUNCTION_END_MARKER = "function_end"

_SOURCE_LOCATION_PREFIX_MARKER = "source location: ["
_SOURCE_LOCATION_RANGE_SEP = " ... "
_SOURCE_LOCATION_SUFFIX_MARKER = ")"

_INST_LOCATION_PREFIX = "; "

_LOCATION_SEP = ":"

_AUTO_GENERATED_MARKER = (
    "// Auto-generated content below. Please do not modify manually."
)


class SourceLocation:
    """Represents a source location in the compiler output.

    The hermes compiler sometimes outputs source locations (when the
    --dump-source-locations flag is set). Those locations potentially
    include absolute paths that should not appear in the expectations.
    Therefore, this class decodes a source location coming from the
    compiler, and it provides helpers to ensure the auto-generated
    expectations work properly (e.g., /full/path/to/file.js becomes
    {{.*}}file.js).
    """

    def __init__(self, loc):
        column_index = loc.rfind(_LOCATION_SEP, 0)
        line_index = loc.rfind(_LOCATION_SEP, 0, column_index)
        if line_index == -1:
            raise Exception(f"Unrecognized location format {loc}")
        self.filename = os.path.basename(loc[:line_index])
        self.line = int(loc[line_index + 1 : column_index])
        self.column = int(loc[column_index + 1 :])

    def to_check_payload(self):
        """Returns a string that can be used in lit expectations."""
        return f"{{{{.*}}}}{self}"

    def __lt__(self, other):
        assert self.filename == other.filename, f"{self} vs {other}"
        return (self.line < other.line) or (
            self.line == other.line and self.column < other.column
        )

    def __eq__(self, other):
        return (
            self.filename == other.filename
            and self.line == other.line
            and self.column == other.column
        )

    def __le__(self, other):
        return self == other or self < other

    def __str__(self):
        return f"{self.filename}{_LOCATION_SEP}{self.line}{_LOCATION_SEP}{self.column}"


class CompilerOutput:
    """Manages the compiler output.

    This class is used as a "container" for the compiler output from which
    the test expectations are generated.
    """

    def __init__(self, js_file, check_prefix):
        self.check_prefix = check_prefix
        self.js_file_location = os.path.join(os.path.dirname(js_file), "")
        self.emit_check_next = False
        self.lines = []

    def add_line(self, line):
        if not line:
            self.emit_check_next = False
            if self.lines and self.lines[-1]:
                self.lines.append("")
            return

        if self.emit_check_next:
            self.lines.append(f"// {self.check_prefix}-NEXT:")
        else:
            self.lines.append(f"// {self.check_prefix}:")
            self.emit_check_next = True

        if self.js_file_location:
            start = line.find(self.js_file_location)
            while start != -1:
                line = (
                    line[:start] + "{{.*}}" + line[start + len(self.js_file_location) :]
                )
                start = line.find(self.js_file_location)

        self.lines[-1] += f"{line}"


def _parse_command_line():
    """Parses a FileCheck-like command line.

    This function parses the scripts command line argument, and exposes some
    options that are also present in FileCheck. This way, whenever the golden
    outputs need to be updated, all that's needed is to replace the command
    used in the %GoldenCheck lit substitution.
    """

    p = argparse.ArgumentParser()

    p.add_argument("js_file")

    # The prefix to be checked for (the --check-prefix command line option in
    # FileCheck) is expected to be part of the golden output file.
    p.add_argument("--check-prefix", "-check-prefix", default="CHECK")

    # Sometimes the golden output tests use the following options, so they are
    # added here so this script can be used as a drop in replacement for the
    # FileCheck utility.
    p.add_argument(
        "--match-full-lines",
        "-match-full-lines",
        default=False,
        action="store_true",
    )

    return p.parse_args()


def main(args):
    print(f"Generating {args.js_file} ...")

    # First step is parsing the compiler output and building the
    # intermediate representation -- i.e., the [Function] list. It
    # is important to exhaust stdin first in case the compiler output
    # has been piped to this script.
    compiler_output = CompilerOutput(args.js_file, args.check_prefix)
    for line in sys.stdin:
        compiler_output.add_line(line.rstrip())

    while compiler_output.lines and not compiler_output.lines[-1]:
        compiler_output.lines.pop()

    # Now it is time to parse the input .js file and append it to the functions.
    js_lines = []
    with open(args.js_file, "r") as jsf:
        for line in jsf:
            if not re.match(f"\\s*//\\s*{args.check_prefix}", line):
                line = line.rstrip()
                if line or (js_lines and js_lines[-1]):
                    js_lines.append(line)

    # And now drop empty lines at the end of the js file
    while not js_lines[-1]:
        js_lines.pop()

    # Now update the test file.
    with open(args.js_file, "w") as jsf:
        has_auto_generated_marker = False
        for line in js_lines:
            print(line, file=jsf)
            has_auto_generated_marker = (
                has_auto_generated_marker or line == _AUTO_GENERATED_MARKER
            )
        if not has_auto_generated_marker:
            print(f"\n{_AUTO_GENERATED_MARKER}", file=jsf)
        print(file=jsf)
        for line in compiler_output.lines:
            print(line, file=jsf)


if __name__ == "__main__":
    main(_parse_command_line())
