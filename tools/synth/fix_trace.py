#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# @noautodeps

import argparse
import os
import re
import sys

# A trace is a JSON object containing properties followed by a large array of
# records. During recording, traces are written to disk by emitting the
# opening brace of the object, the properties, and the opening bracket of the
# records array. Records are then streamed to the trace as they are
# generated.
# When the trace is terminated abruptly, the closing bracket and brace will
# be missing, and the text of the records array may be terminated at an
# arbitrary point. In this case, the JSON is malformed, and the records may
# end with a non-empty stack.
# This function copies lines of the JSON up to the last record with an empty
# stack, then emits the terminating bracket and brace for the array and
# object.


def fix_trace(input_file_path: str, output_file_path: str) -> None:
    enter_types = {
        "CallToNativeRecord",
        "GetPropertyNativeRecord",
        "GetNativePropertyNamesRecord",
        "SetPropertyNativeRecord",
    }
    exit_types = {
        "ReturnFromNativeRecord",
        "GetPropertyNativeReturnRecord",
        "GetNativePropertyNamesReturnRecord",
        "SetPropertyNativeReturnRecord",
    }

    records_begin = re.compile(r'^\s+"trace"\s*:\s*\[\s*$')
    record_end = re.compile(r"^\s+},\s*$")
    record_end_comma = re.compile(r",\s*$")
    type_field = re.compile(r'^\s+"type"\s*:\s*"([a-zA-Z0-9]+)",?\s*$')

    # Track whether we've reached the records array
    in_records = False

    # Accumulate a record, line by line
    current_record: list[str] = []
    current_record_type: str | None = None

    # Track the stack depth, buffering records until the stack empties
    depth = 1
    buffered_records: list[str] = []

    with open(input_file_path, "r") as infile, open(output_file_path, "w") as outfile:
        for line in infile:
            line = line.rstrip("\n")

            if not in_records:
                outfile.write(line + "\n")

                if records_begin.search(line):
                    # The record array has begun
                    in_records = True
            else:
                type_match = type_field.search(line)
                if type_match and type_match.group(1):
                    # Type field
                    current_record_type = type_match.group(1)
                    current_record.append(line)
                elif record_end.search(line):
                    # The record has ended

                    # Insert the trailing comma before the previous record
                    if buffered_records:
                        buffered_records[-1] += ","

                    # Remove trailing comma from the current record, even if
                    # whitespace follows the comma.
                    current_record.append(record_end_comma.sub("", line))

                    # Buffer the record
                    buffered_records.extend(current_record)
                    current_record.clear()

                    # Update the stack depth
                    if current_record_type in enter_types:
                        depth += 1
                    elif current_record_type in exit_types:
                        depth -= 1
                    current_record_type = None

                    # Dump buffered records whenever the stack is empty
                    if depth == 1:
                        # Don't emit the last line yet, as we don't know
                        # whether it requires a comma.
                        last_line = buffered_records.pop()
                        for buffered_line in buffered_records:
                            outfile.write(buffered_line + "\n")
                        buffered_records.clear()
                        buffered_records.append(last_line)
                else:
                    current_record.append(line)

        if buffered_records:
            outfile.write("    }\n")

        # Close the records array and containing object.
        outfile.write("  ]\n}\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Fix a truncated SynthTrace JSON file by closing it "
        "at the last record with an empty stack."
    )
    parser.add_argument("input", help="Path to the truncated trace file")
    parser.add_argument("output", help="Path to write the fixed trace file")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Error: input file does not exist: {args.input}", file=sys.stderr)
        sys.exit(1)

    fix_trace(args.input, args.output)
