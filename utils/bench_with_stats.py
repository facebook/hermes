#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import math  # Needed for infinity when sorting N/A values
import os
import re
import shlex
import statistics
import subprocess
import sys


def find_first_score(output_text):
    """Finds the first line matching 'Name: Score' and returns the score as float."""
    pattern = re.compile(r":?\s*(\d+(?:\.\d+)?)\s*$")
    for line in output_text.strip().split("\n"):
        match = pattern.search(line.strip())
        if match:
            try:
                return float(match.group(1))
            except (ValueError, IndexError):
                continue
    return None


def run_command(command_parts, filename):
    """Runs the command and returns the completed process object."""
    cmd = command_parts + [filename]
    env = os.environ.copy()
    env["PATH"] = f".:{env.get('PATH', '')}"
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        check=False,  # Don't raise error on non-zero exit
        env=env,
    )
    return result


def format_change_line(avg_pct, stdev_pct, num_valid_runs):
    """Formats the 'Change: ...' line based on calculated values."""
    if avg_pct is None:
        # Handle cases where no valid data could be gathered
        if num_valid_runs == 0:
            return "Change: N/A (No valid score pairs found)"
        else:  # Should mean avg_old was 0
            return (
                f"Change: N/A (Average OLD score is 0 from {num_valid_runs} valid runs)"
            )
    elif stdev_pct is not None:
        return f"Change: {avg_pct:+.1f}% +/- {stdev_pct:.1f}% (from {num_valid_runs} valid runs)"
    else:  # Only average is available (1 valid run)
        return f"Change: {avg_pct:+.1f}% (from {num_valid_runs} valid run, +/- N/A)"


def manual_ttest_confidence_interval(data1, data2, confidence=0.95):
    """Calculate confidence interval manually without scipy dependency."""
    if len(data1) < 2 or len(data2) < 2:
        return (None, None)

    # t-distribution critical values for common confidence levels and degrees of freedom
    t_values = {
        0.90: {
            1: 6.314,
            2: 2.920,
            3: 2.353,
            4: 2.132,
            5: 2.015,
            6: 1.943,
            7: 1.895,
            8: 1.860,
            9: 1.833,
            10: 1.812,
            15: 1.753,
            20: 1.725,
            25: 1.708,
            30: 1.697,
            40: 1.684,
            60: 1.671,
            120: 1.658,
            float("inf"): 1.645,
        },
        0.95: {
            1: 12.706,
            2: 4.303,
            3: 3.182,
            4: 2.776,
            5: 2.571,
            6: 2.447,
            7: 2.365,
            8: 2.306,
            9: 2.262,
            10: 2.228,
            15: 2.131,
            20: 2.086,
            25: 2.060,
            30: 2.042,
            40: 2.021,
            60: 2.000,
            120: 1.980,
            float("inf"): 1.960,
        },
        0.99: {
            1: 63.657,
            2: 9.925,
            3: 5.841,
            4: 4.604,
            5: 4.032,
            6: 3.707,
            7: 3.499,
            8: 3.355,
            9: 3.250,
            10: 3.169,
            15: 2.947,
            20: 2.845,
            25: 2.787,
            30: 2.750,
            40: 2.704,
            60: 2.660,
            120: 2.617,
            float("inf"): 2.576,
        },
    }

    n1, n2 = len(data1), len(data2)
    df = n1 + n2 - 2  # degrees of freedom

    # Get t-critical value
    if confidence not in t_values:
        return (None, None)

    t_table = t_values[confidence]
    t_crit = None

    # Find appropriate t-critical value for degrees of freedom
    for deg_freedom in sorted(t_table.keys()):
        if df <= deg_freedom:
            t_crit = t_table[deg_freedom]
            break

    if t_crit is None:
        # Use infinity value for very large df
        t_crit = t_table[float("inf")]

    # Calculate means and variances
    mean1, mean2 = statistics.mean(data1), statistics.mean(data2)
    var1 = statistics.variance(data1) if len(data1) > 1 else 0
    var2 = statistics.variance(data2) if len(data2) > 1 else 0

    # Pooled variance and standard error
    pooled_var = ((n1 - 1) * var1 + (n2 - 1) * var2) / df
    se = math.sqrt(pooled_var * (1 / n1 + 1 / n2))

    if se == 0:
        return (None, None)

    # Difference of means and margin of error
    # Note: data1=new_scores, data2=old_scores, so mean1-mean2 = new-old
    diff = mean1 - mean2
    margin = t_crit * se

    return (diff - margin, diff + margin)


def calculate_geometric_mean(percentages):
    """Calculate geometric mean of percentage improvements.

    Args:
        percentages: List of percentage changes (e.g., [10.5, -5.2, 15.3])

    Returns:
        Geometric mean as a percentage, or None if calculation not possible
    """
    if not percentages:
        return None

    # Convert percentages to multiplicative factors (e.g., +10% -> 1.10)
    factors = [1 + (p / 100) for p in percentages]

    # Check for non-positive factors (would make geometric mean undefined)
    if any(f <= 0 for f in factors):
        return None

    # Calculate geometric mean of factors
    product = 1.0
    for f in factors:
        product *= f

    geo_mean_factor = product ** (1.0 / len(factors))

    # Convert back to percentage change
    return (geo_mean_factor - 1) * 100


def print_summary_table(title, confidence, summary_data):
    """Prints an ASCII table from the collected summary data."""
    if not summary_data:
        print(f"\n--- {title} ---")
        print("No data to display.")
        return

    # Determine column widths
    max_name_len = (
        max(len(item["name"]) for item in summary_data) if summary_data else 10
    )
    # Fixed widths for numbers are often cleaner
    avg_width = 10  # e.g., "+100.5%"
    std_width = 10  # e.g., "+/- 10.5%"
    conf_width = 15  # e.g., "+/- 10.5%"
    runs_width = 12  # e.g., "(10 runs)"
    confidence = round(confidence * 100)

    # Header
    print(f"\n--- {title} ---")
    header = f"+-{'-' * max_name_len}-+-{'-' * avg_width}-+-{'-' * conf_width}-+-{'-' * std_width}-+-{'-' * runs_width}-+"
    print(header)
    print(
        f"| {'Benchmark'.ljust(max_name_len)} | {'Avg Change'.rjust(avg_width)} | {(str(confidence) + '% interval').rjust(conf_width)} | {'Stdev'.rjust(std_width)} | {'Valid Runs'.rjust(runs_width)} |"
    )
    print(header)

    # Data Rows
    for item in summary_data:
        name = item["name"].ljust(max_name_len)
        avg_str = f"{item['avg_pct']:+.1f}%" if item["avg_pct"] is not None else "N/A"
        avg_str = avg_str.rjust(avg_width)
        (conf_low, conf_high) = item["ttest"]
        conf_str = (
            f"{conf_low:+.1f}% - {conf_high:+.1f}%" if conf_low is not None else "N/A"
        )
        conf_str = conf_str.rjust(conf_width)
        std_str = (
            f"+/- {item['stdev_pct']:.1f}%" if item["stdev_pct"] is not None else "N/A"
        )
        std_str = std_str.rjust(std_width)
        runs_str = f"({item['runs']} runs)"
        runs_str = runs_str.rjust(runs_width)

        print(f"| {name} | {avg_str} | {conf_str} | {std_str} | {runs_str} |")

    # Footer
    print(header)

    # Calculate and display geometric mean if we have valid percentage changes
    valid_percentages = [
        item["avg_pct"] for item in summary_data if item["avg_pct"] is not None
    ]

    if valid_percentages:
        geo_mean = calculate_geometric_mean(valid_percentages)
        if geo_mean is not None:
            print(f"\nGeometric Mean of Changes: {geo_mean:+.1f}%")


# --- Main Script ---
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare two benchmark implementations by running them multiple times.",
        epilog='Example: %(prog)s -b "./old-exe -w -Xjit" -b "./new-exe -w -Xjit" benchmark1.js benchmark2.js',
    )
    parser.add_argument(
        "-b",
        "--binary",
        action="append",
        required=True,
        help='Command to run (including arguments). Must be specified exactly twice. Example: -b "./hermes -w -Xjit"',
        metavar="COMMAND",
    )
    parser.add_argument(
        "-n",
        "--runs",
        type=int,
        default=1,
        help="Number of times to run each benchmark (default: 1)",
    )
    parser.add_argument(
        "-c",
        "--confidence",
        type=float,
        default=0.95,
        help="Confidence limit (default: 0.95)",
    )
    parser.add_argument(
        "benchmarks",
        metavar="BENCHMARK.JS",
        nargs="+",  # Require at least one benchmark file
        help="Path to the benchmark file(s) to run.",
    )
    args = parser.parse_args()

    # Validate exactly two binaries
    if len(args.binary) != 2:
        print(
            f"Error: Exactly two -b/--binary arguments required, got {len(args.binary)}.",
            file=sys.stderr,
        )
        sys.exit(1)

    if args.runs <= 0:
        print("Error: Number of runs must be positive.", file=sys.stderr)
        sys.exit(1)

    # Parse the command strings into lists
    old_command = shlex.split(args.binary[0])
    new_command = shlex.split(args.binary[1])

    print(f"OLD command: {' '.join(old_command)}")
    print(f"NEW command: {' '.join(new_command)}")
    print()

    # Store summary results here
    summary_results = []

    for filename in args.benchmarks:
        print(f"--- Running benchmark: {filename} ({args.runs} runs) ---")
        old_scores = []
        new_scores = []
        individual_pct_changes = []

        # --- Run OLD and NEW multiple times ---
        for i in range(args.runs):
            # --- OLD RUN ---
            print(f"OLD {filename} (Run {i+1}/{args.runs})")  # Announce before running
            old_result = run_command(old_command, filename)
            print(old_result.stdout, end="")  # Print the output immediately
            old_score = find_first_score(old_result.stdout)

            # --- NEW RUN ---
            print(f"NEW {filename} (Run {i+1}/{args.runs})")  # Announce before running
            new_result = run_command(new_command, filename)
            print(new_result.stdout, end="")  # Print the output immediately
            new_score = find_first_score(new_result.stdout)

            # --- Store results from this run pair ---
            if old_score is not None and new_score is not None:
                old_scores.append(old_score)
                new_scores.append(new_score)
                if old_score != 0:
                    pct_change = ((new_score - old_score) / old_score) * 100
                    individual_pct_changes.append(pct_change)
                # else: Handle old score = 0 case if needed for individual changes
            else:
                # Optionally warn if scores weren't found for this specific run
                print(
                    f"Warning: Score pair not found for {filename} run {i+1}/{args.runs}."
                )

            print("-" * 20)  # Separator between individual runs

        # --- Calculate Averages and Standard Deviation for this benchmark ---
        num_valid_pairs = len(individual_pct_changes)
        avg_percentage_change = None
        stdev_percentage_change = None
        confidence_interval = None

        if num_valid_pairs > 0:
            # Ensure lists aren't empty before calling mean/stdev
            if old_scores and new_scores:
                avg_old = statistics.mean(old_scores)
                avg_new = statistics.mean(new_scores)
                if avg_old != 0:
                    avg_percentage_change = ((avg_new - avg_old) / avg_old) * 100
                    if num_valid_pairs >= 2:
                        try:
                            stdev_percentage_change = statistics.stdev(
                                individual_pct_changes
                            )
                        except statistics.StatisticsError:
                            # Handle potential errors if list is unexpectedly small
                            stdev_percentage_change = None
                    if num_valid_pairs >= 2:
                        (low, high) = manual_ttest_confidence_interval(
                            new_scores, old_scores, args.confidence
                        )
                        if low is not None and high is not None:
                            (pct_low, pct_high) = (
                                low / avg_old * 100,
                                high / avg_old * 100,
                            )
                        else:
                            (pct_low, pct_high) = (None, None)
                    else:
                        (pct_low, pct_high) = (None, None)

        # else: No valid runs, values remain None

        # --- Print the immediate summary line for this benchmark ---
        change_output_line = format_change_line(
            avg_percentage_change, stdev_percentage_change, num_valid_pairs
        )
        print(
            f"Summary for {filename}: {change_output_line}"
        )  # Make it clear this is the summary line
        print()  # Empty line before next benchmark or final tables

        # --- Store results for final tables ---
        summary_results.append(
            {
                "name": filename,
                "avg_pct": avg_percentage_change,
                "stdev_pct": stdev_percentage_change,
                "ttest": (pct_low, pct_high),
                "runs": num_valid_pairs,
            }
        )

    # --- Print Final Summary Tables ---
    print("=" * 60)
    print("Overall Benchmark Summary")
    print("=" * 60)

    # 1. Table in Original Order
    print_summary_table("Results (Original Order)", args.confidence, summary_results)

    # 2. Table Sorted by Average Change (Descending)
    sorted_summary = sorted(
        summary_results,
        key=lambda item: item.get("avg_pct")
        if item.get("avg_pct") is not None
        else -math.inf,
        reverse=True,
    )
    print_summary_table(
        "Results (Sorted by Avg Change Descending)", args.confidence, sorted_summary
    )

    print("\nBenchmark Comparison Complete.")
