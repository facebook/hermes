#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Register allocation quality analysis for Static Hermes.

Parses the output of `shermes -dump-ra` (IR with register assignments) and
reports per-function register allocation quality metrics:
    - Total register count used
    - Number of spills
    - Maximum register pressure (highest register index used)
    - Identification of functions with high spill counts

Usage:
    shermes -dump-ra input.js | python3 regalloc_report.py [--json]
    python3 regalloc_report.py --input dump_ra.txt [--json]

Examples:
    shermes -dump-ra input.js 2>&1 | python3 regalloc_report.py
    python3 regalloc_report.py --input ra_output.txt --json

IR format notes:
    The -dump-ra output contains IR with register annotations like:
        %r3 = LoadFrameInst ...
        %r0 = AddInst %r1, %r2
    Spills appear as StoreStackInst / LoadStackInst targeting spill slots.
    Functions are delimited by "function <name>(...)" headers.
"""

import argparse
import json
import re
import sys
from typing import Any, Dict, List, Optional


def parse_dump_ra(lines: List[str]) -> List[Dict[str, Any]]:
    """Parse shermes -dump-ra output into per-function register allocation info.

    Returns list of dicts:
        {
            name, start_line, end_line,
            registers_used, max_register, spill_count,
            instruction_count, register_set, spill_slots
        }
    """
    functions: List[Dict[str, Any]] = []

    # Function header pattern: "function <name>(<params>)"
    # or "function <name>#<id>(<params>)"
    func_header = re.compile(r"^(?:scope\s+)?function\s+(\S+?)(?:#\d+)?\s*\(")

    # Register usage pattern: %rN where N is the register number.
    reg_pattern = re.compile(r"%r(\d+)")

    # Spill patterns: StoreStackInst / LoadStackInst to spill slots.
    spill_store = re.compile(r"\bStoreStackInst\b")
    spill_load = re.compile(r"\bLoadStackInst\b")
    # Alternate spill markers: Spill / Reload annotations.
    spill_marker = re.compile(r"\b(Spill|Reload|SpillInst|ReloadInst)\b", re.IGNORECASE)
    # Mov to stack for spill.
    mov_spill = re.compile(r"\bMov(?:Inst)?\b.*%stack")

    current_func: Optional[str] = None
    current_start: int = 0
    registers: set = set()
    spills: int = 0
    instr_count: int = 0
    spill_slots: set = set()

    def flush_function(end_line: int) -> None:
        nonlocal current_func, registers, spills, instr_count, spill_slots
        if current_func is None:
            return
        max_reg = max(registers) if registers else 0
        functions.append(
            {
                "name": current_func,
                "start_line": current_start,
                "end_line": end_line,
                "registers_used": len(registers),
                "max_register": max_reg,
                "spill_count": spills,
                "instruction_count": instr_count,
                "register_set": sorted(registers),
                "spill_slots": sorted(spill_slots),
            }
        )
        current_func = None
        registers = set()
        spills = 0
        instr_count = 0
        spill_slots = set()

    for i, raw_line in enumerate(lines):
        line = raw_line.rstrip("\n")
        lineno = i + 1

        # Check for function header.
        m_func = func_header.match(line)
        if m_func:
            # Flush previous function.
            flush_function(lineno - 1)
            current_func = m_func.group(1)
            current_start = lineno
            continue

        if current_func is None:
            continue

        # Count instructions (lines with = that look like IR instructions).
        if "=" in line and ("Inst" in line or re.search(r"%r\d+\s*=", line)):
            instr_count += 1

        # Collect register numbers.
        for m in reg_pattern.finditer(line):
            registers.add(int(m.group(1)))

        # Detect spills.
        if spill_store.search(line) or spill_load.search(line):
            spills += 1
            # Try to extract spill slot identifier.
            slot_match = re.search(r"%stack(\d+)", line)
            if slot_match:
                spill_slots.add(int(slot_match.group(1)))

        if spill_marker.search(line):
            spills += 1

        if mov_spill.search(line):
            spills += 1
            slot_match = re.search(r"%stack(\d+)", line)
            if slot_match:
                spill_slots.add(int(slot_match.group(1)))

    # Flush last function.
    flush_function(len(lines))

    return functions


def build_report(functions: List[Dict]) -> Dict[str, Any]:
    """Build a summary report from per-function data."""
    if not functions:
        return {
            "total_functions": 0,
            "functions": [],
            "high_spill_functions": [],
            "summary": {},
        }

    total_regs = sum(f["registers_used"] for f in functions)
    total_spills = sum(f["spill_count"] for f in functions)
    max_pressure = max(f["max_register"] for f in functions) if functions else 0

    # Identify high-spill functions (spill_count > 10 or spill ratio > 20%).
    high_spill = []
    for f in functions:
        if f["spill_count"] > 10:
            high_spill.append(f)
        elif f["instruction_count"] > 0:
            ratio = f["spill_count"] / f["instruction_count"]
            if ratio > 0.2:
                high_spill.append(f)

    high_spill.sort(key=lambda x: -x["spill_count"])

    # Sort functions by max_register descending for the report.
    sorted_funcs = sorted(functions, key=lambda f: -f["max_register"])

    return {
        "total_functions": len(functions),
        "summary": {
            "total_registers_across_functions": total_regs,
            "total_spills_across_functions": total_spills,
            "max_register_pressure": max_pressure,
            "avg_registers_per_function": round(total_regs / len(functions), 1),
            "avg_spills_per_function": round(total_spills / len(functions), 1),
        },
        "functions": sorted_funcs,
        "high_spill_functions": high_spill,
    }


def format_human(report: Dict) -> str:
    """Format the report as a human-readable table."""
    lines: List[str] = []

    lines.append("=" * 70)
    lines.append("  Register Allocation Quality Report")
    lines.append("=" * 70)
    lines.append("")

    if report["total_functions"] == 0:
        lines.append("No functions found in input.")
        lines.append("Ensure you are passing shermes -dump-ra output.")
        return "\n".join(lines)

    s = report["summary"]
    lines.append(f"Total functions:           {report['total_functions']}")
    lines.append(f"Max register pressure:     {s['max_register_pressure']}")
    lines.append(f"Avg registers/function:    {s['avg_registers_per_function']}")
    lines.append(f"Avg spills/function:       {s['avg_spills_per_function']}")
    lines.append(f"Total spills:              {s['total_spills_across_functions']}")
    lines.append("")

    if report["high_spill_functions"]:
        lines.append(
            f"--- High Spill Functions ({len(report['high_spill_functions'])}) ---"
        )
        lines.append(
            f"  {'Function':<40s}  {'Regs':>5s}  {'MaxReg':>6s}  "
            f"{'Spills':>6s}  {'Instrs':>6s}"
        )
        lines.append("  " + "-" * 70)
        for f in report["high_spill_functions"]:
            lines.append(
                f"  {f['name']:<40s}  "
                f"{f['registers_used']:>5d}  "
                f"{f['max_register']:>6d}  "
                f"{f['spill_count']:>6d}  "
                f"{f['instruction_count']:>6d}"
            )
        lines.append("")

    lines.append("--- All Functions (by max register, top 30) ---")
    lines.append(
        f"  {'Function':<40s}  {'Regs':>5s}  {'MaxReg':>6s}  "
        f"{'Spills':>6s}  {'Instrs':>6s}"
    )
    lines.append("  " + "-" * 70)
    for f in report["functions"][:30]:
        lines.append(
            f"  {f['name']:<40s}  "
            f"{f['registers_used']:>5d}  "
            f"{f['max_register']:>6d}  "
            f"{f['spill_count']:>6d}  "
            f"{f['instruction_count']:>6d}"
        )
    if len(report["functions"]) > 30:
        lines.append(f"  ... and {len(report['functions']) - 30} more functions")
    lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Analyze register allocation quality from shermes -dump-ra "
            "output. Reports per-function register count, spill count, "
            "and max register pressure."
        ),
        epilog="Reads from stdin if --input is not specified.",
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        default=None,
        help="Path to shermes -dump-ra output file (default: stdin)",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r") as f:
            input_lines = f.readlines()
    else:
        input_lines = sys.stdin.readlines()

    functions = parse_dump_ra(input_lines)
    report = build_report(functions)

    if args.json:
        print(json.dumps(report))
    else:
        print(format_human(report))


if __name__ == "__main__":
    main()
