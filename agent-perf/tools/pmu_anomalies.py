#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""PMU anomaly detection for Static Hermes performance analysis.

Accepts PMU counter data as JSON input (as produced by collect_pmu.sh or
similar tools) and flags values that exceed known-good thresholds.

Anomaly thresholds:
    IPC              < 1.0
    Branch miss rate > 10%
    L1 icache miss   > 5%
    L1 dcache miss   > 10%
    dTLB miss rate   > 3%

Usage:
    python3 pmu_anomalies.py --input pmu_data.json [--json]
    cat pmu_data.json | python3 pmu_anomalies.py [--json]

Input JSON format (example):
    {
        "instructions": 1000000,
        "cycles": 1500000,
        "branch_instructions": 200000,
        "branch_misses": 25000,
        "L1_icache_loads": 500000,
        "L1_icache_misses": 30000,
        "L1_dcache_loads": 800000,
        "L1_dcache_misses": 100000,
        "dTLB_loads": 800000,
        "dTLB_misses": 30000
    }
"""

import argparse
import json
import sys
from typing import Any, Dict, List, Optional


# Each threshold is (check_name, description, threshold, direction, hint).
# direction: "below" means anomaly if value < threshold,
#            "above" means anomaly if value > threshold.
ANOMALY_CHECKS = [
    {
        "name": "low_ipc",
        "label": "IPC (Instructions Per Cycle)",
        "threshold": 1.0,
        "direction": "below",
        "hint": (
            "Low IPC indicates pipeline stalls. Check for cache misses, "
            "branch mispredictions, or long-latency operations. Consider "
            "reordering data structures for better locality."
        ),
    },
    {
        "name": "high_branch_miss",
        "label": "Branch Miss Rate",
        "threshold": 10.0,
        "direction": "above",
        "hint": (
            "High branch misprediction rate. Look for unpredictable "
            "conditional branches in hot paths. Consider branchless "
            "alternatives, profile-guided optimization (PGO), or "
            "reorganizing control flow."
        ),
    },
    {
        "name": "high_l1_icache_miss",
        "label": "L1 I-Cache Miss Rate",
        "threshold": 5.0,
        "direction": "above",
        "hint": (
            "High instruction cache miss rate suggests code is too large "
            "or scattered. Consider function reordering (BOLT/PGO), "
            "reducing code bloat, or merging related functions."
        ),
    },
    {
        "name": "high_l1_dcache_miss",
        "label": "L1 D-Cache Miss Rate",
        "threshold": 10.0,
        "direction": "above",
        "hint": (
            "High data cache miss rate. Check for poor data locality, "
            "large working sets, or pointer-chasing patterns. Consider "
            "compacting data structures, improving access patterns, or "
            "prefetching."
        ),
    },
    {
        "name": "high_dtlb_miss",
        "label": "dTLB Miss Rate",
        "threshold": 3.0,
        "direction": "above",
        "hint": (
            "High dTLB miss rate indicates the program touches many "
            "memory pages. Consider using huge pages, improving data "
            "locality, or reducing the memory footprint."
        ),
    },
]


def safe_div(a: float, b: float) -> Optional[float]:
    """Divide a by b, returning None if b is zero."""
    if b == 0:
        return None
    return a / b


def compute_metrics(data: Dict[str, Any]) -> Dict[str, Optional[float]]:
    """Derive rate metrics from raw PMU counters."""
    instructions = float(data.get("instructions", 0))
    cycles = float(data.get("cycles", 0))
    branch_instr = float(data.get("branch_instructions", 0))
    branch_misses = float(data.get("branch_misses", 0))
    l1i_loads = float(data.get("L1_icache_loads", 0))
    l1i_misses = float(data.get("L1_icache_misses", 0))
    l1d_loads = float(data.get("L1_dcache_loads", 0))
    l1d_misses = float(data.get("L1_dcache_misses", 0))
    dtlb_loads = float(data.get("dTLB_loads", 0))
    dtlb_misses = float(data.get("dTLB_misses", 0))

    return {
        "ipc": safe_div(instructions, cycles),
        "branch_miss_rate": safe_div(branch_misses * 100, branch_instr),
        "l1_icache_miss_rate": safe_div(l1i_misses * 100, l1i_loads),
        "l1_dcache_miss_rate": safe_div(l1d_misses * 100, l1d_loads),
        "dtlb_miss_rate": safe_div(dtlb_misses * 100, dtlb_loads),
    }


# Map check name -> metric key.
_CHECK_TO_METRIC = {
    "low_ipc": "ipc",
    "high_branch_miss": "branch_miss_rate",
    "high_l1_icache_miss": "l1_icache_miss_rate",
    "high_l1_dcache_miss": "l1_dcache_miss_rate",
    "high_dtlb_miss": "dtlb_miss_rate",
}


def detect_anomalies(metrics: Dict[str, Optional[float]]) -> List[Dict]:
    """Check computed metrics against thresholds and return anomalies."""
    anomalies: List[Dict] = []
    for check in ANOMALY_CHECKS:
        metric_key = _CHECK_TO_METRIC[check["name"]]
        value = metrics.get(metric_key)
        if value is None:
            continue

        triggered = False
        if check["direction"] == "below" and value < check["threshold"]:
            triggered = True
        elif check["direction"] == "above" and value > check["threshold"]:
            triggered = True

        if triggered:
            anomalies.append(
                {
                    "anomaly": check["name"],
                    "label": check["label"],
                    "measured_value": round(value, 4),
                    "threshold": check["threshold"],
                    "direction": check["direction"],
                    "hint": check["hint"],
                }
            )

    return anomalies


def format_human(anomalies: List[Dict], metrics: Dict[str, Optional[float]]) -> str:
    """Format anomalies as a human-readable report."""
    lines: List[str] = []
    lines.append("=== PMU Metrics ===")
    for key, val in sorted(metrics.items()):
        if val is not None:
            lines.append(f"  {key}: {val:.4f}")
        else:
            lines.append(f"  {key}: N/A (insufficient data)")
    lines.append("")

    if not anomalies:
        lines.append("No anomalies detected. All metrics within thresholds.")
        return "\n".join(lines)

    lines.append(f"=== Anomalies Detected: {len(anomalies)} ===")
    lines.append("")
    for a in anomalies:
        direction_word = "below" if a["direction"] == "below" else "above"
        lines.append(f"[ANOMALY] {a['label']}")
        lines.append(
            f"  Measured: {a['measured_value']:.4f}  "
            f"(threshold: {direction_word} {a['threshold']})"
        )
        lines.append(f"  Hint: {a['hint']}")
        lines.append("")

    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Detect PMU anomalies from counter data.",
        epilog="Reads JSON from stdin if --input is not specified.",
    )
    parser.add_argument(
        "--input",
        "-i",
        type=str,
        default=None,
        help="Path to JSON file with PMU counter data (default: stdin)",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output results as JSON (default: human-readable report)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input, "r") as f:
            data = json.load(f)
    else:
        data = json.load(sys.stdin)

    metrics = compute_metrics(data)
    anomalies = detect_anomalies(metrics)

    if args.json:
        output = {"metrics": metrics, "anomalies": anomalies}
        print(json.dumps(output))
    else:
        print(format_human(anomalies, metrics))


if __name__ == "__main__":
    main()
