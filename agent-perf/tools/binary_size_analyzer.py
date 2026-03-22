#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""binary_size_analyzer.py — Analyze binary size impact of compiler changes.

Parses nm/objdump output from two binaries compiled from the same JS source to
produce a detailed size comparison at the symbol level.

Usage:
  binary_size_analyzer.py --baseline <binary_a> --optimized <binary_b> [--json]
  binary_size_analyzer.py --nm-a <nm_output_a> --nm-b <nm_output_b> [--json]

The tool reports:
  - Total .text size delta
  - Per-function size changes (sorted by absolute delta)
  - Inlined function expansion (functions present in B but not A)
  - get_by_val-related symbol growth
"""

import argparse
import json
import re
import subprocess
import sys


def parse_nm_output(text):
    """Parse nm --print-size --radix=d output into {symbol: size} dict.

    Expected format per line:
      <address> <size> <type> <symbol>
    We only care about text symbols (type T or t).
    """
    symbols = {}
    for line in text.strip().splitlines():
        parts = line.split()
        if len(parts) < 4:
            continue
        size_str, sym_type, name = parts[1], parts[2], parts[3]
        if sym_type not in ("T", "t"):
            continue
        try:
            size = int(size_str)
        except ValueError:
            continue
        # Accumulate sizes for duplicate symbols (e.g. template instantiations)
        symbols[name] = symbols.get(name, 0) + size
    return symbols


def run_nm(binary_path):
    """Run nm on a binary and return parsed symbols."""
    try:
        result = subprocess.run(
            ["nm", "--print-size", "--radix=d", binary_path],
            capture_output=True,
            text=True,
            timeout=30,
        )
        return parse_nm_output(result.stdout)
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return {}


def get_text_size(binary_path):
    """Get .text section size using objdump."""
    try:
        result = subprocess.run(
            ["objdump", "-h", binary_path],
            capture_output=True,
            text=True,
            timeout=10,
        )
        for line in result.stdout.splitlines():
            if ".text" in line:
                parts = line.split()
                for i, p in enumerate(parts):
                    if p == ".text" and i + 1 < len(parts):
                        return int(parts[i + 1], 16)
    except (subprocess.TimeoutExpired, FileNotFoundError):
        pass
    return 0


def compare_symbols(syms_a, syms_b):
    """Compare two symbol dictionaries and return detailed analysis."""
    all_syms = set(syms_a.keys()) | set(syms_b.keys())

    changes = []
    for sym in all_syms:
        size_a = syms_a.get(sym, 0)
        size_b = syms_b.get(sym, 0)
        delta = size_b - size_a
        if delta != 0:
            changes.append(
                {
                    "symbol": sym,
                    "baseline_bytes": size_a,
                    "optimized_bytes": size_b,
                    "delta_bytes": delta,
                    "is_new": sym not in syms_a,
                    "is_removed": sym not in syms_b,
                }
            )

    # Sort by absolute delta descending
    changes.sort(key=lambda x: abs(x["delta_bytes"]), reverse=True)
    return changes


def classify_symbol(name):
    """Classify a symbol into categories for reporting."""
    if "get_by_val" in name:
        return "get_by_val"
    if "get_by_id" in name:
        return "get_by_id"
    if "put_by" in name:
        return "property_put"
    if "_sh_ljs_call" in name or "_sh_ljs_construct" in name:
        return "calls"
    if "_sh_ljs_add" in name or "_sh_ljs_sub" in name or "_sh_ljs_mul" in name:
        return "arithmetic"
    if "_0_global" in name or re.match(r"_\d+_", name):
        return "generated_js"
    if name.startswith("_sh_"):
        return "runtime"
    return "other"


def format_human(syms_a, syms_b, text_a, text_b):
    """Format comparison results for human consumption."""
    changes = compare_symbols(syms_a, syms_b)

    total_a = sum(syms_a.values())
    total_b = sum(syms_b.values())
    total_delta = total_b - total_a

    lines = []
    lines.append("Binary Size Comparison")
    lines.append("=" * 70)
    lines.append("")

    if text_a > 0 or text_b > 0:
        text_delta = text_b - text_a
        text_pct = (text_delta / text_a * 100) if text_a > 0 else 0
        lines.append(
            f"  .text section:  {text_a:>10d} -> {text_b:>10d}  "
            f"({text_delta:+d} bytes, {text_pct:+.2f}%)"
        )

    sym_pct = (total_delta / total_a * 100) if total_a > 0 else 0
    lines.append(
        f"  Symbol total:   {total_a:>10d} -> {total_b:>10d}  "
        f"({total_delta:+d} bytes, {sym_pct:+.2f}%)"
    )
    lines.append("")

    if not changes:
        lines.append("No symbol size changes detected.")
        return "\n".join(lines)

    # Category summary
    categories = {}
    for c in changes:
        cat = classify_symbol(c["symbol"])
        if cat not in categories:
            categories[cat] = {"count": 0, "delta": 0}
        categories[cat]["count"] += 1
        categories[cat]["delta"] += c["delta_bytes"]

    lines.append("Category Summary:")
    lines.append(f"  {'Category':<20s} {'Symbols':>8s} {'Delta':>12s}")
    lines.append(f"  {'-' * 20} {'-' * 8} {'-' * 12}")
    for cat, info in sorted(
        categories.items(), key=lambda x: abs(x[1]["delta"]), reverse=True
    ):
        lines.append(f"  {cat:<20s} {info['count']:>8d} {info['delta']:>+12d}")
    lines.append("")

    # Top 20 changed symbols
    lines.append("Top 20 Changed Symbols:")
    lines.append(
        f"  {'Symbol':<50s} {'Baseline':>10s} {'Optimized':>10s} {'Delta':>10s}"
    )
    lines.append(f"  {'-' * 50} {'-' * 10} {'-' * 10} {'-' * 10}")
    for c in changes[:20]:
        name = c["symbol"][:50]
        tag = ""
        if c["is_new"]:
            tag = " [NEW]"
        elif c["is_removed"]:
            tag = " [REMOVED]"
        lines.append(
            f"  {name:<50s} {c['baseline_bytes']:>10d} "
            f"{c['optimized_bytes']:>10d} {c['delta_bytes']:>+10d}{tag}"
        )

    return "\n".join(lines)


def format_json(syms_a, syms_b, text_a, text_b, binary_a_path="", binary_b_path=""):
    """Format comparison results as JSON."""
    changes = compare_symbols(syms_a, syms_b)

    categories = {}
    for c in changes:
        cat = classify_symbol(c["symbol"])
        if cat not in categories:
            categories[cat] = {"count": 0, "delta_bytes": 0}
        categories[cat]["count"] += 1
        categories[cat]["delta_bytes"] += c["delta_bytes"]

    total_a = sum(syms_a.values())
    total_b = sum(syms_b.values())

    return {
        "baseline": binary_a_path,
        "optimized": binary_b_path,
        "text_section": {
            "baseline_bytes": text_a,
            "optimized_bytes": text_b,
            "delta_bytes": text_b - text_a,
            "delta_pct": round((text_b - text_a) / text_a * 100, 2) if text_a else 0,
        },
        "symbol_total": {
            "baseline_bytes": total_a,
            "optimized_bytes": total_b,
            "delta_bytes": total_b - total_a,
            "delta_pct": round((total_b - total_a) / total_a * 100, 2)
            if total_a
            else 0,
        },
        "categories": categories,
        "top_changes": changes[:30],
        "total_changed_symbols": len(changes),
    }


def main():
    parser = argparse.ArgumentParser(
        description="Compare binary sizes between two Static Hermes builds"
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--baseline",
        help="Path to baseline binary",
    )
    group.add_argument(
        "--nm-a",
        help="Path to pre-captured nm output for baseline",
    )

    parser.add_argument("--optimized", help="Path to optimized binary")
    parser.add_argument("--nm-b", help="Path to pre-captured nm output for optimized")
    parser.add_argument("--json", action="store_true", help="Output JSON")
    args = parser.parse_args()

    # Get symbol data
    if args.baseline:
        syms_a = run_nm(args.baseline)
        text_a = get_text_size(args.baseline)
        binary_a = args.baseline
    else:
        with open(args.nm_a) as f:
            syms_a = parse_nm_output(f.read())
        text_a = 0
        binary_a = args.nm_a

    opt_path = args.optimized or args.nm_b
    if not opt_path:
        print("Error: --optimized or --nm-b required", file=sys.stderr)
        sys.exit(1)

    if args.optimized:
        syms_b = run_nm(args.optimized)
        text_b = get_text_size(args.optimized)
        binary_b = args.optimized
    else:
        with open(args.nm_b) as f:
            syms_b = parse_nm_output(f.read())
        text_b = 0
        binary_b = args.nm_b

    if not syms_a and not syms_b:
        print("Error: Could not extract symbols from either binary", file=sys.stderr)
        sys.exit(1)

    if args.json:
        result = format_json(syms_a, syms_b, text_a, text_b, binary_a, binary_b)
        print(json.dumps(result))
    else:
        print(format_human(syms_a, syms_b, text_a, text_b))


if __name__ == "__main__":
    main()
