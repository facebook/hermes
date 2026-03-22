#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Generated C code anti-pattern scanner for Static Hermes.

Scans .c files produced by `shermes -emit-c` for patterns that may inhibit
C compiler optimization.  These patterns are artifacts of the code generation
process and may indicate opportunities for improvements in the shermes
compiler.

Detected patterns:
    - excessive_pointer_aliasing: Multiple pointer dereferences through the
      same base that may prevent the C compiler from optimizing memory access
    - redundant_null_check: Consecutive null checks on the same variable
      without intervening modification
    - excessive_function_calls: Very high density of function calls in a
      single basic block, preventing register allocation optimization
    - unnecessary_gc_root: GC root push/pop pairs that bracket only a single
      non-allocating operation
    - redundant_cast: Back-to-back casts on the same value
    - large_stack_frame: Functions with many local variables that may cause
      stack pressure
    - unreachable_after_throw: Code after unconditional throw/longjmp that
      will never execute

Usage:
    python3 generated_code_antipatterns.py --input output.c [--json]

Examples:
    shermes -emit-c -o output.c input.js
    python3 generated_code_antipatterns.py --input output.c --json
"""

import argparse
import json
import re
from collections import Counter
from typing import Dict, List, Optional


# Pattern to match function definitions in generated C.
FUNC_DEF_RE = re.compile(r"^(?:static\s+)?\w[\w\s*]*\s+(_\d+_\w+)\s*\(")
FUNC_END_RE = re.compile(r"^}")


def scan_generated_c(filepath: str) -> List[Dict]:
    """Scan a generated C file for anti-patterns.

    Returns list of findings:
        {file, line, pattern, detail, severity, function}
    """
    try:
        with open(filepath, "r") as f:
            lines = f.readlines()
    except (OSError, UnicodeDecodeError):
        return []

    findings: List[Dict] = []
    current_func: Optional[str] = None
    brace_depth = 0
    func_local_count = 0
    func_start_line = 0

    # Tracking for redundant null check detection.
    last_null_check_var: Optional[str] = None
    last_null_check_line: int = 0

    # Tracking for GC root analysis.
    gc_push_line: Optional[int] = None
    gc_push_var: Optional[str] = None  # noqa: F841
    lines_since_gc_push: int = 0
    has_alloc_since_gc_push = False

    # Tracking for redundant cast detection.
    last_cast_var: Optional[str] = None
    last_cast_line: int = 0

    # Consecutive function call tracking for excessive_function_calls.
    consecutive_calls = 0
    call_block_start = 0

    for i, raw_line in enumerate(lines):
        line = raw_line.rstrip("\n")
        stripped = line.strip()
        lineno = i + 1

        # --- Function boundary tracking ---
        if current_func is not None:
            brace_depth += raw_line.count("{") - raw_line.count("}")

            # Count local variable declarations.
            if re.match(
                r"\s*(SHLegacyValue|SHRuntime|double|int|bool|void)\s+\w+", stripped
            ):
                func_local_count += 1

            if brace_depth <= 0:
                # End of function: check for large stack frame.
                if func_local_count > 50:
                    findings.append(
                        {
                            "file": filepath,
                            "line": func_start_line,
                            "pattern": "large_stack_frame",
                            "detail": (
                                f"Function {current_func} has "
                                f"{func_local_count} local variables, "
                                f"which may cause stack pressure"
                            ),
                            "severity": "medium",
                            "function": current_func,
                        }
                    )
                current_func = None
                func_local_count = 0
                last_null_check_var = None
                gc_push_line = None
                last_cast_var = None
                consecutive_calls = 0
                continue
        else:
            m_func = FUNC_DEF_RE.match(line)
            if m_func and "{" in line:
                current_func = m_func.group(1)
                brace_depth = raw_line.count("{") - raw_line.count("}")
                func_start_line = lineno
                func_local_count = 0
                last_null_check_var = None
                gc_push_line = None
                last_cast_var = None
                consecutive_calls = 0
            continue

        func_name = current_func or "<global>"

        # --- 1. Excessive pointer aliasing ---
        # Detect chains like shr->a->b->c (3+ dereferences).
        deref_chain = re.findall(r"(\w+(?:->|->\w+){3,})", stripped)
        for chain in deref_chain:
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "excessive_pointer_aliasing",
                    "detail": (
                        f"Deep pointer chain '{chain}' may prevent "
                        f"C compiler aliasing optimization"
                    ),
                    "severity": "medium",
                    "function": func_name,
                }
            )

        # --- 2. Redundant null checks ---
        null_check = re.search(r"if\s*\(\s*(\w+)\s*==\s*NULL", stripped)
        if not null_check:
            null_check = re.search(r"if\s*\(\s*!\s*(\w+)\s*\)", stripped)
        if null_check:
            var = null_check.group(1)
            if var == last_null_check_var and lineno - last_null_check_line <= 5:
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "redundant_null_check",
                        "detail": (
                            f"Variable '{var}' null-checked again at line "
                            f"{lineno}, previously at line "
                            f"{last_null_check_line}"
                        ),
                        "severity": "low",
                        "function": func_name,
                    }
                )
            last_null_check_var = var
            last_null_check_line = lineno
        elif re.search(
            r"\b" + re.escape(last_null_check_var or "") + r"\s*=", stripped
        ):
            # Variable was reassigned; reset tracking.
            last_null_check_var = None

        # --- 3. Excessive function calls ---
        if re.search(r"\b_sh_\w+\s*\(", stripped):
            if consecutive_calls == 0:
                call_block_start = lineno
            consecutive_calls += 1
        else:
            if consecutive_calls > 8:
                findings.append(
                    {
                        "file": filepath,
                        "line": call_block_start,
                        "pattern": "excessive_function_calls",
                        "detail": (
                            f"{consecutive_calls} consecutive runtime calls "
                            f"(lines {call_block_start}-{lineno - 1}) "
                            f"may prevent register allocation optimization"
                        ),
                        "severity": "high",
                        "function": func_name,
                    }
                )
            consecutive_calls = 0

        # --- 4. Unnecessary GC root management ---
        if "_sh_push_locals" in stripped or "_sh_new_gcscope" in stripped:
            gc_push_line = lineno
            _ = stripped  # gc_push_var
            lines_since_gc_push = 0
            has_alloc_since_gc_push = False

        if gc_push_line is not None:
            lines_since_gc_push += 1
            # Check for allocating calls.
            if re.search(r"_sh_(new_|ljs_create|ljs_new_object|gc_alloc)", stripped):
                has_alloc_since_gc_push = True

            if "_sh_pop_locals" in stripped or "_sh_end_gcscope" in stripped:
                if lines_since_gc_push <= 3 and not has_alloc_since_gc_push:
                    findings.append(
                        {
                            "file": filepath,
                            "line": gc_push_line,
                            "pattern": "unnecessary_gc_root",
                            "detail": (
                                f"GC scope at line {gc_push_line} brackets "
                                f"only {lines_since_gc_push} lines with no "
                                f"allocating operations"
                            ),
                            "severity": "low",
                            "function": func_name,
                        }
                    )
                gc_push_line = None

        # --- 5. Redundant casts ---
        cast_match = re.search(
            r"\((?:SHLegacyValue|double|int|bool)\)\s*\((?:SHLegacyValue|double|int|bool)\)",
            stripped,
        )
        if cast_match:
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "redundant_cast",
                    "detail": f"Back-to-back casts: {cast_match.group(0)}",
                    "severity": "low",
                    "function": func_name,
                }
            )

        # Also detect cast on a variable that was just cast.
        single_cast = re.search(
            r"\((?:SHLegacyValue|double|int|bool)\)\s*(\w+)", stripped
        )
        if single_cast:
            var = single_cast.group(1)
            if var == last_cast_var and lineno - last_cast_line <= 2:
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "redundant_cast",
                        "detail": (
                            f"Variable '{var}' cast again at line {lineno}, "
                            f"previously cast at line {last_cast_line}"
                        ),
                        "severity": "low",
                        "function": func_name,
                    }
                )
            last_cast_var = var
            last_cast_line = lineno

        # --- 6. Unreachable code after throw/longjmp ---
        if re.search(r"\b(longjmp|_sh_throw)\s*\(", stripped) and not stripped.endswith(
            "{"
        ):
            # Check if next non-empty line is not a label, closing brace,
            # or case.
            for j in range(i + 1, min(i + 4, len(lines))):
                next_stripped = lines[j].strip()
                if not next_stripped:
                    continue
                if (
                    next_stripped.startswith("}")
                    or next_stripped.startswith("case ")
                    or next_stripped.endswith(":")
                ):
                    break
                findings.append(
                    {
                        "file": filepath,
                        "line": j + 1,
                        "pattern": "unreachable_after_throw",
                        "detail": (
                            f"Code at line {j + 1} appears unreachable "
                            f"after throw/longjmp at line {lineno}"
                        ),
                        "severity": "low",
                        "function": func_name,
                    }
                )
                break

    return findings


def format_human(findings: List[Dict]) -> str:
    """Format findings as a human-readable report."""
    if not findings:
        return "No generated-code anti-patterns detected."

    lines: List[str] = []
    lines.append(f"Found {len(findings)} generated-code anti-pattern(s):")
    lines.append("")

    # Group by severity.
    by_severity: Dict[str, List[Dict]] = {"high": [], "medium": [], "low": []}
    for f in findings:
        by_severity.get(f["severity"], by_severity["low"]).append(f)

    for severity in ["high", "medium", "low"]:
        items = by_severity[severity]
        if not items:
            continue
        lines.append(f"--- {severity.upper()} ({len(items)}) ---")
        for f in items:
            func = f.get("function", "<unknown>")
            lines.append(f"  line {f['line']:>5d}  [{f['pattern']}]  in {func}")
            lines.append(f"    {f['detail']}")
        lines.append("")

    # Summary by pattern type.
    pattern_counts: Counter = Counter(f["pattern"] for f in findings)
    lines.append("--- Summary by Pattern ---")
    for pattern, count in pattern_counts.most_common():
        lines.append(f"  {pattern:<35s}  {count:>4d}")
    lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Scan generated C code (from shermes -emit-c) for patterns "
            "that inhibit C compiler optimization."
        ),
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        required=True,
        help="Path to the generated .c file",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    findings = scan_generated_c(args.input)

    if args.json:
        print(json.dumps(findings))
    else:
        print(format_human(findings))


if __name__ == "__main__":
    main()
