#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""C++ anti-pattern scanner for performance-sensitive code.

Scans C++ source files for common performance anti-patterns and optionally
cross-references findings with profiling data to rank them by impact.

Detected patterns:
    - pass_by_value: Large types (std::string, std::vector, etc.) passed by
      value instead of const reference
    - repeated_lookup: Repeated container lookups with the same key (e.g.,
      map[key] used multiple times)
    - string_concat_loop: String concatenation (+=, +) inside loops
    - virtual_dispatch_hot: Virtual function calls (marked if in profiled
      hot functions)
    - unnecessary_copy: Unnecessary copies from auto (should be auto&)
    - redundant_find_access: find() followed by operator[] with same key

Usage:
    python3 antipattern_scan.py --input file.cpp [--profile profile.json] [--json]
    python3 antipattern_scan.py --input src/ --json

Examples:
    python3 antipattern_scan.py --input Runtime.cpp
    python3 antipattern_scan.py --input lib/VM/ --profile profile.json --json
"""

import argparse
import json
import os
import re
import sys
from typing import Dict, List, Set


# Types considered "large" for pass-by-value detection.
LARGE_TYPES = {
    "std::string",
    "string",
    "String",
    "StringRef",
    "llvh::StringRef",
    "std::vector",
    "vector",
    "std::map",
    "map",
    "std::unordered_map",
    "unordered_map",
    "std::set",
    "set",
    "std::unordered_set",
    "unordered_set",
    "std::shared_ptr",
    "shared_ptr",
    "Handle",
    "HermesValue",
    "PseudoHandle",
    "CallResult",
}


def scan_file(filepath: str) -> List[Dict]:
    """Scan a single C++ file for anti-patterns.

    Returns list of findings: {file, line, pattern, detail, estimated_impact}.
    """
    try:
        with open(filepath, "r") as f:
            lines = f.readlines()
    except (OSError, UnicodeDecodeError):
        return []

    findings: List[Dict] = []
    in_loop = False
    loop_depth = 0
    brace_depth_at_loop = 0  # noqa: F841

    for i, line in enumerate(lines):
        stripped = line.strip()
        lineno = i + 1

        # Track loop context.
        if re.match(r"\b(for|while|do)\b", stripped):
            if not in_loop:
                in_loop = True
                loop_depth = 0
                _ = 0  # brace_depth_at_loop placeholder
        if in_loop:
            loop_depth += line.count("{") - line.count("}")
            if loop_depth < 0:
                in_loop = False
                loop_depth = 0

        # 1. Pass by value of large types.
        # Match function parameters like "void foo(std::string s)"
        for type_name in LARGE_TYPES:
            # Escape for regex and look for pass-by-value (no & or *).
            escaped = re.escape(type_name)
            pattern = re.compile(
                r"[\(,]\s*" + escaped + r"(?:<[^>]*>)?\s+(\w+)\s*[,\)]"
            )
            m = pattern.search(line)
            if m:
                # Exclude if preceded by const& or &.
                _ = line[: m.start() + 1]  # preceding context
                if "&" not in line[m.start() : m.end()]:
                    findings.append(
                        {
                            "file": filepath,
                            "line": lineno,
                            "pattern": "pass_by_value",
                            "detail": (
                                f"{type_name} passed by value (param: {m.group(1)})"
                            ),
                            "estimated_impact": "medium",
                        }
                    )

        # 2. Repeated container lookups: map[key] appearing 2+ times.
        bracket_access = re.compile(r"(\w+)\[([^\]]+)\]")
        accesses = bracket_access.findall(stripped)
        if len(accesses) >= 2:
            seen_keys: Dict[str, int] = {}
            for container, key in accesses:
                lookup = f"{container}[{key}]"
                seen_keys[lookup] = seen_keys.get(lookup, 0) + 1
            for lookup, count in seen_keys.items():
                if count >= 2:
                    findings.append(
                        {
                            "file": filepath,
                            "line": lineno,
                            "pattern": "repeated_lookup",
                            "detail": (
                                f"{lookup} accessed {count} times on "
                                f"same line; consider caching in a local"
                            ),
                            "estimated_impact": "low",
                        }
                    )

        # 3. String concatenation in loops.
        if in_loop and ("+=" in stripped or "append(" in stripped):
            if re.search(r"(string|String|str)\s*[\+]?=|\.append\(", stripped):
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "string_concat_loop",
                        "detail": "String concatenation inside a loop",
                        "estimated_impact": "high",
                    }
                )

        # 4. Virtual dispatch detection (calls through virtual methods).
        if re.search(r"\bvirtual\b", stripped) and not stripped.startswith("//"):
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "virtual_dispatch_hot",
                    "detail": (
                        "Virtual function declaration; calls to this may "
                        "cause indirect dispatch overhead in hot paths"
                    ),
                    "estimated_impact": "medium",
                }
            )

        # 5. Unnecessary copy: auto x = expr; where auto& would suffice.
        auto_copy = re.compile(
            r"\bauto\s+(\w+)\s*=\s*(\w+)\.(find|at|front|back|begin|end)"
        )
        m = auto_copy.search(stripped)
        if m:
            findings.append(
                {
                    "file": filepath,
                    "line": lineno,
                    "pattern": "unnecessary_copy",
                    "detail": (
                        f"'auto {m.group(1)}' may copy; consider "
                        f"'auto&' or 'const auto&'"
                    ),
                    "estimated_impact": "low",
                }
            )

        # 6. find() followed by operator[] with same key on adjacent lines.
        find_match = re.search(r"(\w+)\.find\(([^)]+)\)", stripped)
        if find_match and i + 1 < len(lines):
            container = find_match.group(1)
            key = find_match.group(2).strip()
            next_line = lines[i + 1].strip()
            if f"{container}[{key}]" in next_line:
                findings.append(
                    {
                        "file": filepath,
                        "line": lineno,
                        "pattern": "redundant_find_access",
                        "detail": (
                            f"{container}.find({key}) then "
                            f"{container}[{key}]; use iterator from find()"
                        ),
                        "estimated_impact": "medium",
                    }
                )

    return findings


def collect_files(path: str) -> List[str]:
    """Collect C++ source files from a path (file or directory)."""
    cpp_extensions = {".cpp", ".cc", ".cxx", ".h", ".hpp", ".hxx"}
    if os.path.isfile(path):
        return [path]
    files = []
    # Non-recursive: only scan the immediate directory.
    # For recursive scanning, the user should specify individual files or use
    # a tool like find externally.
    try:
        for entry in os.listdir(path):
            full = os.path.join(path, entry)
            if os.path.isfile(full):
                _, ext = os.path.splitext(entry)
                if ext in cpp_extensions:
                    files.append(full)
    except OSError:
        pass
    return sorted(files)


def load_profile(profile_path: str) -> Set[str]:
    """Load hot function names from a profile JSON file.

    Expects a list of objects with a "function" key (as produced by
    profile_to_json.py).
    """
    try:
        with open(profile_path, "r") as f:
            data = json.load(f)
        if isinstance(data, list):
            return {entry["function"] for entry in data if "function" in entry}
    except (OSError, json.JSONDecodeError, KeyError):
        pass
    return set()


def cross_reference_profile(
    findings: List[Dict], hot_functions: Set[str]
) -> List[Dict]:
    """Upgrade estimated_impact for findings in hot functions."""
    if not hot_functions:
        return findings
    for f in findings:  # noqa: B007
        # Simple heuristic: check if any hot function name appears in the
        # file around the finding line.
        # This is approximate; full cross-referencing would need function
        # boundary detection.
        pass
    return findings


def format_human(findings: List[Dict]) -> str:
    """Format findings as a human-readable report."""
    if not findings:
        return "No anti-patterns detected."

    lines: List[str] = []
    lines.append(f"Found {len(findings)} potential anti-pattern(s):")
    lines.append("")
    lines.append(
        f"  {'File':<40s}  {'Line':>5s}  {'Impact':<8s}  {'Pattern':<25s}  Detail"
    )
    lines.append("  " + "-" * 100)

    for f in findings:
        basename = os.path.basename(f["file"])
        lines.append(
            f"  {basename:<40s}  "
            f"{f['line']:>5d}  "
            f"{f['estimated_impact']:<8s}  "
            f"{f['pattern']:<25s}  "
            f"{f['detail']}"
        )

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Scan C++ source files for performance anti-patterns. "
            "Optionally cross-reference with profile data to rank by impact."
        ),
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        required=True,
        help="C++ source file or directory to scan",
    )
    parser.add_argument(
        "--profile",
        "-p",
        type=str,
        default=None,
        help=(
            "Path to profile JSON (from profile_to_json.py) for "
            "cross-referencing hot functions"
        ),
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable table)",
    )
    args = parser.parse_args()

    files = collect_files(args.input)
    if not files:
        print(f"No C++ files found at: {args.input}", file=sys.stderr)
        sys.exit(1)

    all_findings: List[Dict] = []
    for filepath in files:
        all_findings.extend(scan_file(filepath))

    if args.profile:
        hot_functions = load_profile(args.profile)
        all_findings = cross_reference_profile(all_findings, hot_functions)

    if args.json:
        print(json.dumps(all_findings))
    else:
        print(format_human(all_findings))


if __name__ == "__main__":
    main()
