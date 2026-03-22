#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""JavaScript anti-pattern scanner for Static Hermes.

Scans JavaScript source files for patterns that exercise slow engine paths
in Hermes / Static Hermes.  These patterns often prevent effective
ahead-of-time compilation or force fallbacks to the slow interpreter.

Detected patterns:
    - eval_usage: Use of eval() prevents static analysis and optimization
    - with_statement: 'with' creates dynamic scopes, disabling optimizations
    - arguments_materialization: Use of 'arguments' object forces
      materialization (use rest parameters instead)
    - megamorphic_access: Property access on highly variable object shapes
      (detected via bracket notation with dynamic keys in hot loops)
    - dynamic_property_name: Computed property names in object literals
    - prototype_pollution: Direct __proto__ or Object.setPrototypeOf usage
    - delete_operator: 'delete' forces transition to dictionary mode
    - try_catch_in_loop: try/catch inside loops prevents many optimizations
    - sparse_array: Array with gaps or very large indices
    - implicit_global: Assignment to undeclared variable (no var/let/const)

Usage:
    python3 js_antipattern_scan.py --input file.js [--json]
    python3 js_antipattern_scan.py --input src/ [--json]

Examples:
    python3 js_antipattern_scan.py --input app.js
    python3 js_antipattern_scan.py --input src/ --json
"""

import argparse
import json
import os
import re
import sys
from typing import Dict, List


def scan_js_file(filepath: str) -> List[Dict]:
    """Scan a single JavaScript file for anti-patterns.

    Returns list of findings:
        {file, line, pattern, detail, severity, suggestion}
    """
    try:
        with open(filepath, "r") as f:
            lines = f.readlines()
    except (OSError, UnicodeDecodeError):
        return []

    findings: List[Dict] = []
    in_loop = False
    loop_brace_depth = 0
    in_comment_block = False

    for i, line in enumerate(lines):
        stripped = line.strip()
        lineno = i + 1

        # Skip block comments.
        if "/*" in stripped and "*/" not in stripped:
            in_comment_block = True
            continue
        if in_comment_block:
            if "*/" in stripped:
                in_comment_block = False
            continue
        # Skip single-line comments.
        if stripped.startswith("//"):
            continue

        # Track loop context.
        if re.search(r"\b(for|while|do)\s*[\(\{]", stripped):
            in_loop = True
            loop_brace_depth = 0
        if in_loop:
            loop_brace_depth += line.count("{") - line.count("}")
            if loop_brace_depth < 0:
                in_loop = False
                loop_brace_depth = 0

        # 1. eval() usage.
        if re.search(r"\beval\s*\(", stripped):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "eval_usage",
                    "detail": "eval() prevents static analysis and AOT compilation",
                    "severity": "high",
                    "suggestion": (
                        "Replace eval() with Function constructor or "
                        "restructure to avoid dynamic code execution"
                    ),
                }
            )

        # 2. with statement.
        if re.match(r"\bwith\s*\(", stripped):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "with_statement",
                    "detail": "'with' creates dynamic scope, preventing optimization",
                    "severity": "high",
                    "suggestion": (
                        "Remove 'with' and use explicit property access "
                        "or destructuring"
                    ),
                }
            )

        # 3. arguments object materialization.
        if re.search(r"\barguments\b", stripped) and not re.search(
            r"//.*\barguments\b", stripped
        ):
            # Exclude common false positives like comments and string literals.
            if not re.search(r"['\"].*\barguments\b.*['\"]", stripped):
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "arguments_materialization",
                        "detail": (
                            "'arguments' object forces materialization; "
                            "slow path in engine"
                        ),
                        "severity": "medium",
                        "suggestion": "Use rest parameters (...args) instead",
                    }
                )

        # 4. Megamorphic property access (bracket with variable in loop).
        if in_loop and re.search(r"\w+\[\w+\]", stripped):
            # Heuristic: bracket access with a variable inside a loop.
            if not re.search(r"\[\d+\]", stripped):  # Exclude numeric indices.
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "megamorphic_access",
                        "detail": (
                            "Dynamic property access in loop may cause "
                            "megamorphic inline cache misses"
                        ),
                        "severity": "medium",
                        "suggestion": (
                            "Consider using Map/Set, typed arrays, or "
                            "restructuring to use fixed property names"
                        ),
                    }
                )

        # 5. Computed property names in object literals.
        if re.search(r"\{\s*\[", stripped) or re.search(r",\s*\[.*\]\s*:", stripped):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "dynamic_property_name",
                    "detail": "Computed property name prevents shape prediction",
                    "severity": "low",
                    "suggestion": (
                        "Use static property names where possible for "
                        "better hidden class optimization"
                    ),
                }
            )

        # 6. Prototype pollution.
        if re.search(r"__proto__", stripped) or re.search(
            r"Object\.setPrototypeOf", stripped
        ):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "prototype_pollution",
                    "detail": (
                        "Prototype modification invalidates inline caches "
                        "and hidden class chains"
                    ),
                    "severity": "high",
                    "suggestion": (
                        "Use Object.create() at construction time instead "
                        "of mutating prototypes"
                    ),
                }
            )

        # 7. delete operator.
        if re.search(r"\bdelete\s+\w+[\.\[]", stripped):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "delete_operator",
                    "detail": (
                        "'delete' forces object to transition to "
                        "dictionary mode (slow properties)"
                    ),
                    "severity": "medium",
                    "suggestion": "Set property to undefined instead of deleting",
                }
            )

        # 8. try/catch in loop.
        if in_loop and re.match(r"\btry\s*\{", stripped):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "try_catch_in_loop",
                    "detail": "try/catch inside loop prevents many optimizations",
                    "severity": "medium",
                    "suggestion": "Move try/catch outside the loop if possible",
                }
            )

        # 9. Sparse array detection.
        sparse_match = re.search(r"\w+\[(\d{4,})\]", stripped)
        if sparse_match:
            idx = int(sparse_match.group(1))
            if idx > 10000:
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "sparse_array",
                        "detail": (
                            f"Array access at index {idx} may create "
                            f"a sparse array (slow)"
                        ),
                        "severity": "medium",
                        "suggestion": "Use Map instead of sparse arrays",
                    }
                )

        # 10. Implicit global (assignment without declaration).
        # Heuristic: assignment at start of line without var/let/const/this.
        if re.match(r"^[a-zA-Z_]\w*\s*=\s*[^=]", stripped):
            # Exclude object property assignments (preceded by .) and
            # known declaration keywords on previous line.
            if not stripped.startswith("this.") and not stripped.startswith("module."):
                # Check that the previous non-empty line is not a
                # var/let/const/function declaration.
                prev_idx = i - 1
                while prev_idx >= 0 and not lines[prev_idx].strip():
                    prev_idx -= 1
                prev = lines[prev_idx].strip() if prev_idx >= 0 else ""
                if not re.search(
                    r"\b(var|let|const|function|class|export|import)\b", prev
                ):
                    findings.append(
                        {
                            "file": filepath,
                            "line": lineno,
                            "pattern": "implicit_global",
                            "detail": (
                                "Possible implicit global assignment (no var/let/const)"
                            ),
                            "severity": "low",
                            "suggestion": (
                                "Add 'let' or 'const' declaration; use "
                                "'use strict' to catch these at runtime"
                            ),
                        }
                    )

    return findings


def collect_js_files(path: str) -> List[str]:
    """Collect JavaScript files from a path (file or directory)."""
    js_extensions = {".js", ".mjs", ".cjs", ".jsx"}
    if os.path.isfile(path):
        return [path]
    files = []
    try:
        for entry in os.listdir(path):
            full = os.path.join(path, entry)
            if os.path.isfile(full):
                _, ext = os.path.splitext(entry)
                if ext in js_extensions:
                    files.append(full)
    except OSError:
        pass
    return sorted(files)


def format_human(findings: List[Dict]) -> str:
    """Format findings as a human-readable report."""
    if not findings:
        return "No JavaScript anti-patterns detected."

    lines: List[str] = []
    lines.append(f"Found {len(findings)} JavaScript anti-pattern(s):")
    lines.append("")

    # Group by severity.
    by_severity = {"high": [], "medium": [], "low": []}
    for f in findings:
        by_severity.get(f["severity"], by_severity["low"]).append(f)

    for severity in ["high", "medium", "low"]:
        items = by_severity[severity]
        if not items:
            continue
        lines.append(f"--- {severity.upper()} severity ({len(items)}) ---")
        for f in items:
            basename = os.path.basename(f["file"])
            lines.append(f"  {basename}:{f['line']}  [{f['pattern']}]")
            lines.append(f"    {f['detail']}")
            lines.append(f"    -> {f['suggestion']}")
        lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Scan JavaScript files for patterns that exercise slow engine "
            "paths in Hermes / Static Hermes."
        ),
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        required=True,
        help="JavaScript file or directory to scan",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    files = collect_js_files(args.input)
    if not files:
        print(f"No JavaScript files found at: {args.input}", file=sys.stderr)
        sys.exit(1)

    all_findings: List[Dict] = []
    for filepath in files:
        all_findings.extend(scan_js_file(filepath))

    if args.json:
        print(json.dumps(all_findings))
    else:
        print(format_human(all_findings))


if __name__ == "__main__":
    main()
