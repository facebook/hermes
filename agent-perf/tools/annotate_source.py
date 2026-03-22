#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Overlay line-level profile data on source code.

Parses the output of `perf annotate --stdio` and maps sample counts back to
source lines.  In human-readable mode (default) hot lines are prefixed with
[HOT X.X%].  In JSON mode the output is a list of per-line records.

Usage:
    perf annotate --stdio | python3 annotate_source.py [--function FUNC] [--json]
    python3 annotate_source.py --input annotate.txt --function myFunc

Examples:
    perf annotate --stdio | python3 annotate_source.py --function interpret --json
    python3 annotate_source.py -i ann.txt --function doCall
"""

import argparse
import json
import re
import sys
from typing import Dict, List, Optional


def parse_perf_annotate(
    lines: List[str], function_filter: Optional[str] = None
) -> List[Dict]:
    """Parse perf annotate --stdio output, extracting per-source-line samples.

    The annotate output interleaves source lines with percentage annotations:
        : int foo(int x) {
        0.50 :     return x + 1;
        :     }

    When --function is given only the matching function section is returned.

    Returns list of dicts: {file, line_number, source, percentage, hot}.
    """
    results: List[Dict] = []
    current_function: Optional[str] = None
    current_file: Optional[str] = None
    in_target = function_filter is None  # If no filter, include everything.

    # Matches the function header produced by perf annotate.
    func_header = re.compile(r"^(\S.*):$")
    # Matches annotated source lines.  The percentage may be absent (cold line).
    # Format: "  percentage : source" or "         : source"
    annotated_line = re.compile(r"^\s*(\d+\.\d+)?\s*:\s*(\d+)?\s*:(.*)$")
    # Alternate simpler format: "  pct : source" without explicit line numbers.
    simple_annotated = re.compile(r"^\s*(\d+\.\d+)?\s*:\s(.*)$")
    # File path line (appears as comment like "// file.cpp").
    file_marker = re.compile(r"^\s*:\s*(/\S+\.\w+):$")

    line_counter = 0

    for raw in lines:
        raw = raw.rstrip("\n")

        # Detect function headers.
        m_func = func_header.match(raw)
        if m_func and not raw.startswith(" "):
            current_function = m_func.group(1).strip()
            if function_filter:
                in_target = function_filter.lower() in current_function.lower()
            line_counter = 0
            continue

        if not in_target:
            continue

        # Detect file markers.
        m_file = file_marker.match(raw)
        if m_file:
            current_file = m_file.group(1)
            continue

        # Try to parse annotated source line with explicit line number.
        m = annotated_line.match(raw)
        if m:
            pct_str = m.group(1)
            line_no_str = m.group(2)
            source = m.group(3)
            pct = float(pct_str) if pct_str else 0.0
            line_no = int(line_no_str) if line_no_str else None
            if line_no is None:
                line_counter += 1
                line_no = line_counter
            results.append(
                {
                    "file": current_file or "<unknown>",
                    "line_number": line_no,
                    "source": source,
                    "percentage": pct,
                    "hot": pct >= 1.0,
                }
            )
            continue

        # Fallback: simpler annotated format without line numbers.
        m2 = simple_annotated.match(raw)
        if m2:
            pct_str = m2.group(1)
            source = m2.group(2)
            pct = float(pct_str) if pct_str else 0.0
            line_counter += 1
            results.append(
                {
                    "file": current_file or "<unknown>",
                    "line_number": line_counter,
                    "source": source,
                    "percentage": pct,
                    "hot": pct >= 1.0,
                }
            )

    return results


def format_human(results: List[Dict]) -> str:
    """Format results as annotated source with [HOT] markers on hot lines."""
    if not results:
        return "No annotated lines found."

    lines: List[str] = []
    prev_file: Optional[str] = None
    for r in results:
        if r["file"] != prev_file:
            if prev_file is not None:
                lines.append("")
            lines.append(f"--- {r['file']} ---")
            prev_file = r["file"]

        prefix = ""
        if r["hot"]:
            prefix = f"[HOT {r['percentage']:5.1f}%] "
        elif r["percentage"] > 0:
            prefix = f"[    {r['percentage']:5.2f}%] "
        else:
            prefix = "             "

        lines.append(f"{prefix}{r['line_number']:5d}: {r['source']}")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Overlay line-level perf profile data on source code.",
        epilog="Reads from stdin if --input is not specified.",
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        default=None,
        help="Path to perf annotate --stdio output file (default: stdin)",
    )
    parser.add_argument(
        "--function",
        "-f",
        type=str,
        default=None,
        help="Filter to a specific function (substring match)",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: annotated source)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r") as f:
            lines = f.readlines()
    else:
        lines = sys.stdin.readlines()

    results = parse_perf_annotate(lines, function_filter=args.function)

    if args.json:
        print(json.dumps(results))
    else:
        print(format_human(results))


if __name__ == "__main__":
    main()
