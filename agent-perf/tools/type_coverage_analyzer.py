#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Analyze typed mode effectiveness by comparing generated C files.

Compares two .c files produced by shermes -emit-c -- one compiled in typed
mode and one in untyped mode -- to measure how effectively typed mode
eliminates runtime calls.

The tool counts runtime calls (_sh_*) in each file, identifies which calls
were eliminated or reduced by typed mode, and reports per-function type
coverage.

Usage:
    python3 type_coverage_analyzer.py --typed typed.c --untyped untyped.c [--json]

Examples:
    shermes -emit-c -typed -o typed.c input.js
    shermes -emit-c -o untyped.c input.js
    python3 type_coverage_analyzer.py --typed typed.c --untyped untyped.c
"""

import argparse
import json
import re
from collections import Counter
from typing import Any, Dict, List, Optional, Set


# Pattern to match _sh_* runtime calls.
SH_CALL_RE = re.compile(r"\b(_sh_\w+)\s*\(")

# Pattern to match function definitions in generated C.
FUNC_DEF_RE = re.compile(r"^(?:static\s+)?\w[\w\s*]*\s+(_\d+_\w+)\s*\(")
FUNC_END_RE = re.compile(r"^}")


def extract_runtime_calls(source: str) -> Dict[str, Any]:
    """Extract per-function and global runtime call counts from a C source.

    Returns:
        {
            "global_calls": Counter of all _sh_* calls,
            "functions": {func_name: Counter of _sh_* calls in that function},
            "function_lines": {func_name: line_count},
        }
    """
    lines = source.split("\n")
    global_calls: Counter = Counter()
    functions: Dict[str, Counter] = {}
    function_lines: Dict[str, int] = {}

    current_func: Optional[str] = None
    current_calls: Counter = Counter()
    brace_depth = 0
    func_line_count = 0

    for line in lines:
        if current_func is not None:
            brace_depth += line.count("{") - line.count("}")
            func_line_count += 1

            for m in SH_CALL_RE.finditer(line):
                call = m.group(1)
                current_calls[call] += 1
                global_calls[call] += 1

            if brace_depth <= 0:
                functions[current_func] = current_calls
                function_lines[current_func] = func_line_count
                current_func = None
                current_calls = Counter()
                func_line_count = 0
                continue
        else:
            m_func = FUNC_DEF_RE.match(line)
            if m_func and "{" in line:
                current_func = m_func.group(1)
                brace_depth = line.count("{") - line.count("}")
                func_line_count = 1
                current_calls = Counter()

                for m in SH_CALL_RE.finditer(line):
                    call = m.group(1)
                    current_calls[call] += 1
                    global_calls[call] += 1

    return {
        "global_calls": global_calls,
        "functions": functions,
        "function_lines": function_lines,
    }


def compare(typed_data: Dict, untyped_data: Dict) -> Dict[str, Any]:
    """Compare typed vs untyped generated code.

    Returns a structured comparison report.
    """
    typed_global = typed_data["global_calls"]
    untyped_global = untyped_data["global_calls"]

    all_calls: Set[str] = set(typed_global.keys()) | set(untyped_global.keys())

    # Per-call comparison.
    call_comparison: List[Dict] = []
    eliminated: List[str] = []
    reduced: List[Dict] = []
    increased: List[Dict] = []

    for call in sorted(all_calls):
        tc = typed_global.get(call, 0)
        uc = untyped_global.get(call, 0)
        delta = tc - uc
        call_comparison.append(
            {
                "call": call,
                "typed_count": tc,
                "untyped_count": uc,
                "delta": delta,
            }
        )
        if uc > 0 and tc == 0:
            eliminated.append(call)
        elif uc > 0 and tc < uc:
            pct_reduction = round((1 - tc / uc) * 100, 1)
            reduced.append(
                {
                    "call": call,
                    "typed": tc,
                    "untyped": uc,
                    "reduction_pct": pct_reduction,
                }
            )
        elif tc > uc:
            increased.append({"call": call, "typed": tc, "untyped": uc, "delta": delta})

    # Sort reduced by reduction percentage (most reduced first).
    reduced.sort(key=lambda x: -x["reduction_pct"])

    # Per-function coverage.
    typed_funcs = typed_data["functions"]
    untyped_funcs = untyped_data["functions"]
    all_func_names: Set[str] = set(typed_funcs.keys()) | set(untyped_funcs.keys())

    per_function: List[Dict] = []
    for fname in sorted(all_func_names):
        tc = sum(typed_funcs.get(fname, Counter()).values())
        uc = sum(untyped_funcs.get(fname, Counter()).values())
        if uc > 0:
            coverage = round((1 - tc / uc) * 100, 1) if tc < uc else 0.0
        else:
            coverage = 0.0
        per_function.append(
            {
                "function": fname,
                "typed_calls": tc,
                "untyped_calls": uc,
                "calls_eliminated": max(0, uc - tc),
                "coverage_pct": coverage,
            }
        )

    per_function.sort(key=lambda x: -x["coverage_pct"])

    total_typed = sum(typed_global.values())
    total_untyped = sum(untyped_global.values())
    overall_reduction = (
        round((1 - total_typed / total_untyped) * 100, 1) if total_untyped > 0 else 0.0
    )

    return {
        "summary": {
            "typed_total_calls": total_typed,
            "untyped_total_calls": total_untyped,
            "calls_eliminated": max(0, total_untyped - total_typed),
            "overall_reduction_pct": overall_reduction,
            "unique_calls_eliminated": len(eliminated),
            "unique_calls_reduced": len(reduced),
            "unique_calls_increased": len(increased),
        },
        "eliminated_calls": eliminated,
        "reduced_calls": reduced,
        "increased_calls": increased,
        "per_function": per_function,
        "call_comparison": call_comparison,
    }


def format_human(report: Dict) -> str:
    """Format comparison as a human-readable report."""
    lines: List[str] = []
    s = report["summary"]

    lines.append("=" * 70)
    lines.append("  Type Coverage Analysis: Typed vs Untyped")
    lines.append("=" * 70)
    lines.append("")
    lines.append(f"Untyped total runtime calls: {s['untyped_total_calls']}")
    lines.append(f"Typed total runtime calls:   {s['typed_total_calls']}")
    lines.append(f"Calls eliminated:            {s['calls_eliminated']}")
    lines.append(f"Overall reduction:           {s['overall_reduction_pct']}%")
    lines.append("")

    if report["eliminated_calls"]:
        lines.append(
            f"--- Calls Fully Eliminated ({len(report['eliminated_calls'])}) ---"
        )
        for call in report["eliminated_calls"]:
            lines.append(f"  {call}")
        lines.append("")

    if report["reduced_calls"]:
        lines.append(f"--- Calls Reduced ({len(report['reduced_calls'])}) ---")
        lines.append(
            f"  {'Call':<45s}  {'Typed':>6s}  {'Untyped':>7s}  {'Reduction':>9s}"
        )
        lines.append("  " + "-" * 72)
        for r in report["reduced_calls"][:20]:
            lines.append(
                f"  {r['call']:<45s}  "
                f"{r['typed']:>6d}  "
                f"{r['untyped']:>7d}  "
                f"{r['reduction_pct']:>8.1f}%"
            )
        lines.append("")

    if report["increased_calls"]:
        lines.append(f"--- Calls Increased ({len(report['increased_calls'])}) ---")
        for r in report["increased_calls"][:10]:
            lines.append(
                f"  {r['call']:<45s}  "
                f"typed={r['typed']}  untyped={r['untyped']}  "
                f"delta=+{r['delta']}"
            )
        lines.append("")

    lines.append("--- Per-Function Type Coverage (top 30) ---")
    lines.append(
        f"  {'Function':<40s}  {'Typed':>6s}  {'Untyped':>7s}  "
        f"{'Elim':>5s}  {'Coverage':>8s}"
    )
    lines.append("  " + "-" * 72)
    for f in report["per_function"][:30]:
        lines.append(
            f"  {f['function']:<40s}  "
            f"{f['typed_calls']:>6d}  "
            f"{f['untyped_calls']:>7d}  "
            f"{f['calls_eliminated']:>5d}  "
            f"{f['coverage_pct']:>7.1f}%"
        )
    if len(report["per_function"]) > 30:
        lines.append(f"  ... and {len(report['per_function']) - 30} more functions")
    lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Analyze typed mode effectiveness by comparing generated C files. "
            "Counts runtime calls in typed vs untyped output and reports "
            "which calls were eliminated or reduced."
        ),
    )
    parser.add_argument(
        "--typed",
        "-t",
        type=str,
        required=True,
        help="Path to the typed-mode generated .c file",
    )
    parser.add_argument(
        "--untyped",
        "-u",
        type=str,
        required=True,
        help="Path to the untyped-mode generated .c file",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    with open(args.typed, "r") as f:
        typed_source = f.read()
    with open(args.untyped, "r") as f:
        untyped_source = f.read()

    typed_data = extract_runtime_calls(typed_source)
    untyped_data = extract_runtime_calls(untyped_source)
    report = compare(typed_data, untyped_data)

    if args.json:
        print(json.dumps(report))
    else:
        print(format_human(report))


if __name__ == "__main__":
    main()
