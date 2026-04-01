#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Convert perf report --stdio output to structured JSON.

Parses the textual output of `perf report --stdio` and extracts per-function
profiling information including function name, module, sample count, and
percentage of total samples.

Usage:
    perf report --stdio | python3 profile_to_json.py [--top N] [--filter MODULE] [--json]
    python3 profile_to_json.py --input perf_report.txt [--top N] [--filter MODULE] [--json]

Examples:
    perf report --stdio | python3 profile_to_json.py --top 20 --json
    python3 profile_to_json.py --input report.txt --filter libhermes --top 10
"""

import argparse
import json
import re
import sys
from typing import Dict, List, Optional


def parse_perf_report(lines: List[str]) -> List[Dict]:
    """Parse perf report --stdio output into structured records.

    Expects lines in the format produced by `perf report --stdio`, e.g.:
        # Overhead  Samples  Command  Shared Object  Symbol
        12.34%      1234     hermes   libhermes.so   [.] someFunction
         5.67%       567     hermes   [kernel]       [k] page_fault

    Returns a list of dicts with keys: function, module, samples, percentage.
    """
    results = []
    # Pattern matches lines like:
    #   12.34%  1234  command  module  [.] function
    # The percentage may or may not have leading spaces.
    # Some versions omit sample counts; we handle both.
    pattern_with_samples = re.compile(
        r"^\s*(\d+\.\d+)%\s+"  # percentage
        r"(\d+)\s+"  # sample count
        r"(\S+)\s+"  # command
        r"(\S+)\s+"  # shared object / module
        r"\[.\]\s+"  # symbol type indicator [.] or [k] etc.
        r"(.+?)\s*$"  # function name
    )
    pattern_no_samples = re.compile(
        r"^\s*(\d+\.\d+)%\s+"  # percentage
        r"(\S+)\s+"  # command
        r"(\S+)\s+"  # shared object / module
        r"\[.\]\s+"  # symbol type indicator
        r"(.+?)\s*$"  # function name
    )

    for line in lines:
        line = line.rstrip("\n")
        # Skip comment and empty lines.
        if not line or line.startswith("#"):
            continue

        m = pattern_with_samples.match(line)
        if m:
            results.append(
                {
                    "function": m.group(5),
                    "module": m.group(4),
                    "samples": int(m.group(2)),
                    "percentage": float(m.group(1)),
                }
            )
            continue

        m = pattern_no_samples.match(line)
        if m:
            results.append(
                {
                    "function": m.group(4),
                    "module": m.group(3),
                    "samples": 0,
                    "percentage": float(m.group(1)),
                }
            )

    return results


def filter_results(
    results: List[Dict],
    module_filter: Optional[str] = None,
    top_n: Optional[int] = None,
) -> List[Dict]:
    """Apply optional module filter and top-N limit."""
    if module_filter:
        results = [r for r in results if module_filter.lower() in r["module"].lower()]
    # Sort by percentage descending.
    results.sort(key=lambda r: r["percentage"], reverse=True)
    if top_n is not None and top_n > 0:
        results = results[:top_n]
    return results


def format_human(results: List[Dict]) -> str:
    """Format results as a human-readable table."""
    if not results:
        return "No matching entries found."

    lines = []
    lines.append(f"{'%':>8s}  {'Samples':>8s}  {'Module':<30s}  {'Function'}")
    lines.append("-" * 80)
    for r in results:
        lines.append(
            f"{r['percentage']:7.2f}%  {r['samples']:>8d}  "
            f"{r['module']:<30s}  {r['function']}"
        )
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert perf report --stdio output to structured JSON.",
        epilog="Reads from stdin if --input is not specified.",
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        default=None,
        help="Path to perf report --stdio output file (default: stdin)",
    )
    parser.add_argument(
        "--top",
        "-n",
        type=int,
        default=None,
        help="Show only the top N entries by percentage",
    )
    parser.add_argument(
        "--filter",
        "-f",
        type=str,
        default=None,
        help="Filter results to entries whose module contains this substring",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable table)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r") as f:
            lines = f.readlines()
    else:
        lines = sys.stdin.readlines()

    results = parse_perf_report(lines)
    results = filter_results(results, module_filter=args.filter, top_n=args.top)

    if args.json:
        print(json.dumps(results))
    else:
        print(format_human(results))


if __name__ == "__main__":
    main()
