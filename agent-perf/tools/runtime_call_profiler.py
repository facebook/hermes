#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Profile _sh_* runtime function usage from perf report output.

Parses perf report output filtered to Static Hermes _sh_* runtime functions,
ranks them by sample count, groups them by category, and identifies the top
optimization candidates.

Categories:
    - property_access: get/put/del by id, val, index
    - type_check: typeof, is_*, to_*, coercions
    - gc: push/pop locals, GC scope management
    - arithmetic: add, sub, mul, div, mod, inc, dec, negate, bitwise
    - comparison: equal, strict_equal, less, greater
    - call: function calls and construct
    - object: object/array creation
    - string: string operations and concatenation
    - other: everything else

Usage:
    perf report --stdio | python3 runtime_call_profiler.py [--top N] [--json]
    python3 runtime_call_profiler.py --input report.txt --top 20 --json

Examples:
    perf report --stdio | python3 runtime_call_profiler.py --top 10
    python3 runtime_call_profiler.py -i report.txt --json
"""

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from typing import Dict, List, Optional


# Category classification for _sh_* functions (same categories as
# generated_code_analyzer.py for consistency).
_CATEGORY_PATTERNS = [
    (
        "property_access",
        re.compile(
            r"_sh_ljs_(get|put|del|has)_by_(id|val|index)"
            r"|_sh_ljs_get_computed"
            r"|_sh_prload"
            r"|_sh_prstore"
            r"|_sh_find_property"
        ),
    ),
    (
        "type_check",
        re.compile(
            r"_sh_ljs_typeof"
            r"|_sh_ljs_is_"
            r"|_sh_to_"
            r"|_sh_ljs_double"
            r"|_sh_ljs_bool"
        ),
    ),
    (
        "gc",
        re.compile(
            r"_sh_push_locals"
            r"|_sh_pop_locals"
            r"|_sh_gc_"
            r"|_sh_check_native_stack"
            r"|_sh_new_gcscope"
            r"|_sh_end_gcscope"
            r"|_sh_flush_gcscope"
        ),
    ),
    (
        "arithmetic",
        re.compile(
            r"_sh_ljs_(add|sub|mul|div|mod|inc|dec|negate|bit|lshift|rshift)"
            r"|_sh_ljs_exp"
        ),
    ),
    (
        "comparison",
        re.compile(r"_sh_ljs_(equal|strict_equal|less|greater|abstract_equal)"),
    ),
    ("call", re.compile(r"_sh_ljs_(call|construct)|_sh_call")),
    (
        "object",
        re.compile(
            r"_sh_ljs_create"
            r"|_sh_ljs_new_object"
            r"|_sh_new_"
            r"|_sh_ljs_object"
            r"|_sh_ljs_array"
        ),
    ),
    (
        "string",
        re.compile(
            r"_sh_ljs_string"
            r"|_sh_string_"
            r"|_sh_ljs_concat"
        ),
    ),
]


def categorize(name: str) -> str:
    """Classify an _sh_* function into a category."""
    for category, pattern in _CATEGORY_PATTERNS:
        if pattern.search(name):
            return category
    return "other"


def parse_perf_report_sh(lines: List[str]) -> List[Dict]:
    """Parse perf report output, extracting only _sh_* function entries.

    Returns list of dicts: {function, module, samples, percentage, category}.
    """
    results = []

    # Matches perf report lines with sample counts.
    pattern_with_samples = re.compile(
        r"^\s*(\d+\.\d+)%\s+"
        r"(\d+)\s+"
        r"(\S+)\s+"
        r"(\S+)\s+"
        r"\[.\]\s+"
        r"(.+?)\s*$"
    )
    # Matches perf report lines without sample counts.
    pattern_no_samples = re.compile(
        r"^\s*(\d+\.\d+)%\s+"
        r"(\S+)\s+"
        r"(\S+)\s+"
        r"\[.\]\s+"
        r"(.+?)\s*$"
    )

    for line in lines:
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue

        func_name = None
        module = None
        samples = 0
        pct = 0.0

        m = pattern_with_samples.match(line)
        if m:
            pct = float(m.group(1))
            samples = int(m.group(2))
            module = m.group(4)
            func_name = m.group(5)
        else:
            m = pattern_no_samples.match(line)
            if m:
                pct = float(m.group(1))
                module = m.group(3)
                func_name = m.group(4)

        if func_name and func_name.startswith("_sh_"):
            results.append(
                {
                    "function": func_name,
                    "module": module or "<unknown>",
                    "samples": samples,
                    "percentage": pct,
                    "category": categorize(func_name),
                }
            )

    # Sort by percentage descending.
    results.sort(key=lambda r: r["percentage"], reverse=True)
    return results


def build_report(entries: List[Dict], top_n: Optional[int] = None) -> Dict:
    """Build a structured report from parsed entries."""
    # Category aggregation.
    cat_samples: Counter = Counter()
    cat_pct: Dict[str, float] = defaultdict(float)
    cat_count: Counter = Counter()

    for e in entries:
        cat_samples[e["category"]] += e["samples"]
        cat_pct[e["category"]] += e["percentage"]
        cat_count[e["category"]] += 1

    categories = []
    for cat in sorted(cat_pct, key=lambda c: -cat_pct[c]):
        categories.append(
            {
                "category": cat,
                "total_percentage": round(cat_pct[cat], 2),
                "total_samples": cat_samples[cat],
                "function_count": cat_count[cat],
            }
        )

    # Top optimization candidates.
    candidates = entries[:top_n] if top_n else entries

    return {
        "total_sh_functions": len(entries),
        "total_sh_percentage": round(sum(e["percentage"] for e in entries), 2),
        "total_sh_samples": sum(e["samples"] for e in entries),
        "categories": categories,
        "optimization_candidates": candidates,
        "all_entries": entries,
    }


def format_human(report: Dict, top_n: Optional[int] = None) -> str:
    """Format the report as a human-readable table."""
    lines: List[str] = []

    lines.append("=" * 70)
    lines.append("  Static Hermes Runtime Call Profile (_sh_* functions)")
    lines.append("=" * 70)
    lines.append("")
    lines.append(f"Total _sh_* functions profiled: {report['total_sh_functions']}")
    lines.append(
        f"Total _sh_* percentage:         {report['total_sh_percentage']:.2f}%"
    )
    lines.append(f"Total _sh_* samples:            {report['total_sh_samples']}")
    lines.append("")

    lines.append("--- Category Breakdown ---")
    lines.append(f"  {'Category':<20s}  {'%':>7s}  {'Samples':>8s}  {'#Funcs':>6s}")
    lines.append("  " + "-" * 50)
    for cat in report["categories"]:
        lines.append(
            f"  {cat['category']:<20s}  "
            f"{cat['total_percentage']:6.2f}%  "
            f"{cat['total_samples']:>8d}  "
            f"{cat['function_count']:>6d}"
        )
    lines.append("")

    candidates = report["optimization_candidates"]
    label = f"Top {len(candidates)} Optimization Candidates"
    lines.append(f"--- {label} ---")
    lines.append(f"  {'Function':<45s}  {'%':>7s}  {'Samples':>8s}  {'Category'}")
    lines.append("  " + "-" * 75)
    for e in candidates:
        lines.append(
            f"  {e['function']:<45s}  "
            f"{e['percentage']:6.2f}%  "
            f"{e['samples']:>8d}  "
            f"{e['category']}"
        )
    lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Profile _sh_* runtime function usage from perf report output. "
            "Ranks by sample count, groups by category, and identifies "
            "optimization candidates."
        ),
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
        default=20,
        help="Number of top optimization candidates to show (default: 20)",
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

    entries = parse_perf_report_sh(input_lines)
    report = build_report(entries, top_n=args.top)

    if args.json:
        print(json.dumps(report))
    else:
        print(format_human(report, top_n=args.top))


if __name__ == "__main__":
    main()
