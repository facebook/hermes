#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Analyze generated C code from shermes -emit-c.

This is one of the most important diagnostic tools for Static Hermes (SH)
performance analysis. It parses the .c file produced by `shermes -emit-c`
and provides detailed metrics on runtime call usage, per-function statistics,
and detects common anti-patterns that may indicate optimization opportunities.

Runtime calls (_sh_*) are categorized into:
    - property_access: _sh_ljs_get_by_id*, _sh_ljs_put_by_id*, _sh_ljs_get_by_val*, etc.
    - type_check: _sh_ljs_typeof*, _sh_ljs_is_*, _sh_to_*, _sh_ljs_double*
    - gc: _sh_push_locals, _sh_pop_locals, _sh_gc_*, _sh_check_native_stack
    - arithmetic: _sh_ljs_add*, _sh_ljs_sub*, _sh_ljs_mul*, _sh_ljs_div*,
                  _sh_ljs_mod*, _sh_ljs_inc*, _sh_ljs_dec*, _sh_ljs_negate*
    - object: _sh_ljs_create*, _sh_ljs_new_object*, _sh_new_*
    - string: _sh_ljs_string*, _sh_string_*, _sh_ljs_concat*
    - call: _sh_ljs_call*, _sh_ljs_construct*
    - comparison: _sh_ljs_equal*, _sh_ljs_strict_equal*, _sh_ljs_less*,
                  _sh_ljs_greater*
    - other: everything else matching _sh_*

Usage:
    python3 generated_code_analyzer.py --input output.c [--json]

Examples:
    shermes -emit-c -o output.c input.js
    python3 generated_code_analyzer.py --input output.c --json
    python3 generated_code_analyzer.py --input output.c
"""

import argparse
import json
import re
from collections import Counter
from typing import Any, Dict, List, Optional, Tuple


# Categories for _sh_* runtime calls, ordered by specificity.
CALL_CATEGORIES: List[Tuple[str, re.Pattern]] = [
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
    (
        "call",
        re.compile(r"_sh_ljs_(call|construct)|_sh_call"),
    ),
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


def categorize_call(name: str) -> str:
    """Return the category for a given _sh_* function name."""
    for category, pattern in CALL_CATEGORIES:
        if pattern.search(name):
            return category
    return "other"


def parse_generated_c(source: str) -> Dict[str, Any]:
    """Parse a generated C file from shermes -emit-c.

    Returns a dict with:
        - functions: list of per-function info
        - runtime_calls: aggregated call counts by name and category
        - patterns: detected anti-patterns
        - summary: overall statistics
    """
    lines = source.split("\n")

    # --- Detect functions ---
    # Generated functions typically look like:
    #   static SHLegacyValue _0_myFunc(SHRuntime *shr, ...) {
    func_pattern = re.compile(r"^(?:static\s+)?\w[\w\s*]*\s+(_\d+_\w+)\s*\(")
    func_end = re.compile(r"^}")  # noqa: F841 End of function at column 0.

    # --- Detect _sh_* calls ---
    sh_call_pattern = re.compile(r"\b(_sh_\w+)\s*\(")

    functions: List[Dict[str, Any]] = []
    all_calls: Counter = Counter()
    category_counts: Counter = Counter()

    current_func: Optional[str] = None
    current_func_start: int = 0
    current_func_calls: Counter = Counter()
    current_func_gc_scopes: int = 0
    current_func_lines: int = 0
    brace_depth: int = 0

    for i, line in enumerate(lines):
        # Track brace depth for function boundary detection.
        if current_func is not None:
            brace_depth += line.count("{") - line.count("}")
            current_func_lines += 1

            # Collect _sh_* calls in this function.
            for m in sh_call_pattern.finditer(line):
                call_name = m.group(1)
                current_func_calls[call_name] += 1
                all_calls[call_name] += 1
                category_counts[categorize_call(call_name)] += 1

            # Count GC scope markers.
            if "_sh_push_locals" in line or "_sh_new_gcscope" in line:
                current_func_gc_scopes += 1

            # End of function.
            if brace_depth <= 0:
                total_calls = sum(current_func_calls.values())
                func_categories: Counter = Counter()
                for name, count in current_func_calls.items():
                    func_categories[categorize_call(name)] += count

                functions.append(
                    {
                        "name": current_func,
                        "start_line": current_func_start + 1,
                        "end_line": i + 1,
                        "line_count": current_func_lines,
                        "total_runtime_calls": total_calls,
                        "gc_scopes": current_func_gc_scopes,
                        "calls_per_line": (
                            round(total_calls / current_func_lines, 3)
                            if current_func_lines > 0
                            else 0
                        ),
                        "call_breakdown": dict(func_categories),
                        "top_calls": current_func_calls.most_common(10),
                    }
                )
                current_func = None
                current_func_calls = Counter()
                current_func_gc_scopes = 0
                current_func_lines = 0
                continue

        # Look for function start.
        m_func = func_pattern.match(line)
        if m_func and "{" in line:
            current_func = m_func.group(1)
            current_func_start = i
            brace_depth = line.count("{") - line.count("}")
            current_func_lines = 1

            # Check this first line for calls too.
            for m in sh_call_pattern.finditer(line):
                call_name = m.group(1)
                current_func_calls[call_name] += 1
                all_calls[call_name] += 1
                category_counts[categorize_call(call_name)] += 1

            if "_sh_push_locals" in line or "_sh_new_gcscope" in line:
                current_func_gc_scopes += 1

    # --- Detect patterns / anti-patterns ---
    patterns = _detect_patterns(source, lines, functions)

    # --- Summary ---
    summary = {
        "total_functions": len(functions),
        "total_runtime_calls": sum(all_calls.values()),
        "total_lines": len(lines),
        "category_breakdown": dict(category_counts),
        "top_runtime_calls": all_calls.most_common(20),
    }

    return {
        "functions": functions,
        "runtime_calls": dict(all_calls),
        "category_breakdown": dict(category_counts),
        "patterns": patterns,
        "summary": summary,
    }


def _detect_patterns(
    source: str, lines: List[str], functions: List[Dict]
) -> List[Dict]:
    """Detect common anti-patterns in generated C code."""
    patterns: List[Dict] = []

    # 1. Redundant type checks: consecutive _sh_ljs_typeof or _sh_ljs_is_ calls.
    type_check_re = re.compile(r"\b(_sh_ljs_typeof|_sh_ljs_is_\w+)\s*\(")
    prev_type_check: Optional[str] = None
    prev_line_no: int = 0
    for i, line in enumerate(lines):
        m = type_check_re.search(line)
        if m:
            if (
                prev_type_check is not None
                and i - prev_line_no <= 2
                and m.group(1) == prev_type_check
            ):
                patterns.append(
                    {
                        "pattern": "redundant_type_check",
                        "line": i + 1,
                        "detail": (
                            f"Repeated {m.group(1)} call near line {i + 1}, "
                            f"previous at line {prev_line_no + 1}"
                        ),
                        "severity": "medium",
                    }
                )
            prev_type_check = m.group(1)
            prev_line_no = i
        else:
            prev_type_check = None

    # 2. Excessive GC scoping: functions with many GC scope operations.
    for func in functions:
        if func["gc_scopes"] > 5:
            patterns.append(
                {
                    "pattern": "excessive_gc_scoping",
                    "line": func["start_line"],
                    "detail": (
                        f"Function {func['name']} has {func['gc_scopes']} "
                        f"GC scope operations in {func['line_count']} lines"
                    ),
                    "severity": "medium",
                }
            )

    # 3. Inline opportunities: very short functions with few calls.
    for func in functions:
        if (
            func["line_count"] <= 10
            and func["total_runtime_calls"] <= 2
            and func["line_count"] > 0
        ):
            patterns.append(
                {
                    "pattern": "inline_opportunity",
                    "line": func["start_line"],
                    "detail": (
                        f"Function {func['name']} is very short "
                        f"({func['line_count']} lines, "
                        f"{func['total_runtime_calls']} runtime calls) "
                        f"and may benefit from inlining"
                    ),
                    "severity": "low",
                }
            )

    # 4. High runtime call density.
    for func in functions:
        if func["calls_per_line"] > 0.5 and func["line_count"] > 20:
            patterns.append(
                {
                    "pattern": "high_call_density",
                    "line": func["start_line"],
                    "detail": (
                        f"Function {func['name']} has high runtime call "
                        f"density ({func['calls_per_line']:.2f} calls/line "
                        f"over {func['line_count']} lines)"
                    ),
                    "severity": "high",
                }
            )

    return patterns


def format_human(result: Dict[str, Any]) -> str:
    """Format analysis results as a human-readable report."""
    lines: List[str] = []
    summary = result["summary"]

    lines.append("=" * 70)
    lines.append("  Static Hermes Generated Code Analysis")
    lines.append("=" * 70)
    lines.append("")
    lines.append(f"Total functions:      {summary['total_functions']}")
    lines.append(f"Total runtime calls:  {summary['total_runtime_calls']}")
    lines.append(f"Total lines:          {summary['total_lines']}")
    lines.append("")

    lines.append("--- Runtime Call Categories ---")
    for cat, count in sorted(
        summary["category_breakdown"].items(), key=lambda x: -x[1]
    ):
        lines.append(f"  {cat:<20s}  {count:>6d}")
    lines.append("")

    lines.append("--- Top Runtime Calls ---")
    for name, count in summary["top_runtime_calls"]:
        cat = categorize_call(name)
        lines.append(f"  {name:<50s}  {count:>5d}  [{cat}]")
    lines.append("")

    lines.append("--- Per-Function Summary (sorted by runtime calls) ---")
    sorted_funcs = sorted(result["functions"], key=lambda f: -f["total_runtime_calls"])
    for func in sorted_funcs[:30]:
        lines.append(
            f"  {func['name']:<40s}  "
            f"calls={func['total_runtime_calls']:>4d}  "
            f"lines={func['line_count']:>4d}  "
            f"gc_scopes={func['gc_scopes']:>2d}  "
            f"density={func['calls_per_line']:.2f}"
        )
    if len(sorted_funcs) > 30:
        lines.append(f"  ... and {len(sorted_funcs) - 30} more functions")
    lines.append("")

    if result["patterns"]:
        lines.append(f"--- Detected Patterns ({len(result['patterns'])}) ---")
        for p in result["patterns"]:
            lines.append(
                f"  [{p['severity'].upper():>6s}] line {p['line']:>5d}: {p['pattern']}"
            )
            lines.append(f"          {p['detail']}")
        lines.append("")
    else:
        lines.append("--- No anti-patterns detected ---")
        lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Analyze generated C code from shermes -emit-c. "
            "Categorizes runtime calls, computes per-function metrics, "
            "and detects optimization anti-patterns."
        ),
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        required=True,
        help="Path to the .c file generated by shermes -emit-c",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    with open(args.input, "r") as f:
        source = f.read()

    result = parse_generated_c(source)

    if args.json:
        print(json.dumps(result))
    else:
        print(format_human(result))


if __name__ == "__main__":
    main()
