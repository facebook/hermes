#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Hotness-annotated disassembly from perf annotate output.

Parses `perf annotate --stdio` output for assembly instructions,
extracting per-instruction sample percentages and marking hot instructions.

Usage:
    perf annotate --stdio | python3 annotate_disasm.py [--function FUNC] [--json]
    python3 annotate_disasm.py --input annotate.txt --function myFunc --json

Examples:
    perf annotate --stdio | python3 annotate_disasm.py --function interpret --json
    python3 annotate_disasm.py -i ann.txt --function doCall
"""

import argparse
import json
import re
import sys
from typing import Dict, List, Optional


# Threshold (percentage) above which an instruction is considered "hot".
HOT_THRESHOLD = 1.0


def parse_disasm_annotations(
    lines: List[str], function_filter: Optional[str] = None
) -> List[Dict]:
    """Parse perf annotate --stdio output for assembly instructions.

    perf annotate produces output like:
        Percent | Source code & Disassembly of binary
        --------+-----------------------------------
                : function_name():
          0.50  :   4005a0: mov    %rdi,%rbx
         12.30  :   4005a3: callq  0x400480 <malloc@plt>
                :   4005a8: test   %rax,%rax

    Returns list of dicts:
        {address, instruction, mnemonic, percentage, hot, function}
    """
    results: List[Dict] = []
    current_function: Optional[str] = None
    in_target = function_filter is None

    # Matches a function header line, e.g. "  someFunction():"
    func_header = re.compile(r"^\s*(\S[^:]*(?:\([^)]*\))?)\s*:\s*$")

    # Matches annotated assembly lines.
    # Format: "  pct  :  addr: instruction" or "       :  addr: instruction"
    asm_line = re.compile(
        r"^\s*(\d+\.\d+)?\s*:\s*"  # optional percentage
        r"([0-9a-fA-F]+):\s+"  # hex address
        r"(.+?)\s*$"  # instruction text
    )

    for raw in lines:
        raw = raw.rstrip("\n")

        # Detect function headers.
        m_func = func_header.match(raw)
        if m_func:
            candidate = m_func.group(1).strip()
            # Skip "Percent" header lines.
            if candidate.lower().startswith("percent"):
                continue
            current_function = candidate
            if function_filter:
                in_target = function_filter.lower() in current_function.lower()
            continue

        if not in_target:
            continue

        m = asm_line.match(raw)
        if m:
            pct_str = m.group(1)
            address = m.group(2)
            instruction = m.group(3)
            pct = float(pct_str) if pct_str else 0.0

            # Extract mnemonic (first word of instruction).
            mnemonic = instruction.split()[0] if instruction.split() else ""

            results.append(
                {
                    "address": f"0x{address}",
                    "instruction": instruction,
                    "mnemonic": mnemonic,
                    "percentage": pct,
                    "hot": pct >= HOT_THRESHOLD,
                    "function": current_function or "<unknown>",
                }
            )

    return results


def format_human(results: List[Dict]) -> str:
    """Format disassembly with hotness markers."""
    if not results:
        return "No disassembly entries found."

    lines: List[str] = []
    prev_func: Optional[str] = None
    for r in results:
        if r["function"] != prev_func:
            if prev_func is not None:
                lines.append("")
            lines.append(f"=== {r['function']} ===")
            prev_func = r["function"]

        marker = ""
        if r["hot"]:
            marker = f">>> {r['percentage']:5.1f}% "
        elif r["percentage"] > 0:
            marker = f"    {r['percentage']:5.2f}% "
        else:
            marker = "           "

        lines.append(f"{marker} {r['address']}:  {r['instruction']}")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Produce hotness-annotated disassembly from perf annotate output."
        ),
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
        help="Output results as JSON (default: annotated disassembly)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r") as f:
            lines = f.readlines()
    else:
        lines = sys.stdin.readlines()

    results = parse_disasm_annotations(lines, function_filter=args.function)

    if args.json:
        print(json.dumps(results))
    else:
        print(format_human(results))


if __name__ == "__main__":
    main()
