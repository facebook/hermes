#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import csv
import math  # Needed for infinity when sorting N/A values
import os
import re
import shlex
import statistics
import subprocess
import sys
from collections import defaultdict


def find_first_score(output_text):
    """Finds the first line matching 'Name: Score' and returns the score as float."""
    pattern = re.compile(r":?\s*(\d+(?:\.\d+)?)\s*$")
    for line in output_text.strip().split("\n"):
        match = pattern.search(line.strip())
        if match:
            try:
                return float(match.group(1))
            except ValueError:
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
    if df == 0:
        return (None, None)
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


def calculate_geometric_mean_ratios(ratios):
    """Calculate geometric mean of ratios.

    Args:
        ratios: List of ratios (e.g., [1.18, 0.89, 1.05])

    Returns:
        Geometric mean of ratios, or None if calculation not possible
    """
    if not ratios or any(r <= 0 for r in ratios):
        return None
    product = 1.0
    for r in ratios:
        product *= r
    return product ** (1.0 / len(ratios))


def print_per_benchmark_table(benchmark_result, display_format, confidence):
    """Prints detailed table for a single benchmark with all engines."""
    engines = benchmark_result["engines"]
    if not engines:
        return

    # Determine column widths
    max_engine_len = (
        max(
            len(f"engine{e['index'] + 1} {'(base)' if e['index'] == 0 else ''}")
            for e in engines
        )
        + 2
    )
    score_width = 11
    vs_base_width = 11
    conf_width = 17
    stdev_width = 11
    runs_width = 11

    # Header
    print(f"--- Results for {benchmark_result['name']} (baseline: engine1) ---")
    header = f"+-{'-' * max_engine_len}-+-{'-' * score_width}-+-{'-' * vs_base_width}-+-{'-' * conf_width}-+-{'-' * stdev_width}-+-{'-' * runs_width}-+"
    print(header)
    print(
        f"| {'Engine'.ljust(max_engine_len)} | {'Avg Score'.rjust(score_width)} | {'vs Base'.rjust(vs_base_width)} | "
        f"{(str(round(confidence * 100)) + '% CI').rjust(conf_width)} | {'Stdev'.rjust(stdev_width)} | {'Valid Runs'.rjust(runs_width)} |"
    )
    print(header)

    # Data rows
    for engine in engines:
        # Engine name
        engine_name = f"engine{engine['index'] + 1}"
        if engine["index"] == 0:
            engine_name += " (base)"
        engine_name = engine_name.ljust(max_engine_len)

        # Average score
        avg_score_str = f"{engine['avg_score']:.1f}".rjust(score_width)

        # vs Base
        if engine["index"] == 0:
            vs_base_str = "-".rjust(vs_base_width)
        else:
            if display_format == "percentage" and engine["vs_baseline_pct"] is not None:
                vs_base_str = f"{engine['vs_baseline_pct']:+.1f}%".rjust(vs_base_width)
            elif engine["vs_baseline_ratio"] is not None:
                vs_base_str = f"{engine['vs_baseline_ratio']:.2f}x".rjust(vs_base_width)
            else:
                vs_base_str = "N/A".rjust(vs_base_width)

        # Confidence interval
        conf_int = engine.get("confidence_interval", (None, None))
        if conf_int[0] is not None:
            conf_str = f"{conf_int[0]:.1f} - {conf_int[1]:.1f}".rjust(conf_width)
        else:
            conf_str = "N/A".rjust(conf_width)

        # Stdev
        if engine.get("stdev") is not None:
            stdev_str = f"+/- {engine['stdev']:.1f}".rjust(stdev_width)
        else:
            stdev_str = "N/A".rjust(stdev_width)

        # Valid runs
        runs_str = f"({engine['valid_runs']} runs)".rjust(runs_width)

        print(
            f"| {engine_name} | {avg_score_str} | {vs_base_str} | {conf_str} | {stdev_str} | {runs_str} |"
        )

    print(header)
    print()


def print_compact_summary_table(
    title, all_results, display_format, num_engines, sort_by_last=False
):
    """Prints compact summary table with all benchmarks and engines."""
    if not all_results:
        print(f"\n--- {title} ---")
        print("No data to display.")
        return

    # Prepare data
    summary_data = []
    for result in all_results:
        row = {"name": result["name"], "comparisons": []}
        for engine_idx in range(1, num_engines):
            # Find this engine in results
            engine_data = None
            for e in result["engines"]:
                if e["index"] == engine_idx:
                    engine_data = e
                    break

            if engine_data:
                if display_format == "percentage":
                    value = engine_data.get("vs_baseline_pct")
                else:
                    value = engine_data.get("vs_baseline_ratio")
            else:
                value = None

            row["comparisons"].append(value)

        summary_data.append(row)

    # Sort if requested
    if sort_by_last and summary_data and summary_data[0]["comparisons"]:
        summary_data = sorted(
            summary_data,
            key=lambda x: x["comparisons"][-1]
            if x["comparisons"][-1] is not None
            else -math.inf,
            reverse=True,
        )

    # Determine column widths
    max_name_len = max(len(row["name"]) for row in summary_data) if summary_data else 10
    # Also consider "Geometric Mean" and "Benchmark" in the width calculation
    max_name_len = max(max_name_len, len("Geometric Mean"), len("Benchmark"))
    comp_width = 11

    # Header
    print(f"\n--- {title} ---")
    header = f"+-{'-' * max_name_len}-"
    for _ in range(1, num_engines):
        header += f"+-{'-' * comp_width}-"
    header += "+"
    print(header)

    # Column headers
    header_row = f"| {'Benchmark'.ljust(max_name_len)} "
    for i in range(1, num_engines):
        header_row += f"| {f'engine{i + 1}/1'.rjust(comp_width)} "
    header_row += "|"
    print(header_row)
    print(header)

    # Data rows
    for row in summary_data:
        data_row = f"| {row['name'].ljust(max_name_len)} "
        for comp_value in row["comparisons"]:
            if comp_value is not None:
                if display_format == "percentage":
                    value_str = f"{comp_value:+.1f}%"
                else:
                    value_str = f"{comp_value:.2f}x"
            else:
                value_str = "N/A"
            data_row += f"| {value_str.rjust(comp_width)} "
        data_row += "|"
        print(data_row)

    # Footer with geometric means
    print(header)

    # Calculate geometric means for each engine comparison
    geo_means = []
    for engine_idx in range(len(summary_data[0]["comparisons"]) if summary_data else 0):
        values = [
            row["comparisons"][engine_idx]
            for row in summary_data
            if row["comparisons"][engine_idx] is not None
        ]
        if values:
            if display_format == "percentage":
                geo_mean = calculate_geometric_mean(values)
            else:
                geo_mean = calculate_geometric_mean_ratios(values)
            geo_means.append(geo_mean)
        else:
            geo_means.append(None)

    if any(gm is not None for gm in geo_means):
        geo_row = f"| {'Geometric Mean'.ljust(max_name_len)} "
        for gm in geo_means:
            if gm is not None:
                if display_format == "percentage":
                    value_str = f"{gm:+.1f}%"
                else:
                    value_str = f"{gm:.2f}x"
            else:
                value_str = "N/A"
            geo_row += f"| {value_str.rjust(comp_width)} "
        geo_row += "|"
        print(geo_row)
        print(header)


def export_to_csv(filename, all_results, engine_commands):
    """Export benchmark results to CSV file for easy graphing."""
    with open(filename, "w", newline="") as csvfile:
        # Prepare header
        fieldnames = ["Benchmark"]
        for i, _ in enumerate(engine_commands):
            engine_name = f"Engine{i + 1}"
            fieldnames.append(engine_name)

        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        # Write data rows
        for result in all_results:
            row = {"Benchmark": result["name"]}

            # Initialize all engine columns to ensure consistent CSV
            for i in range(len(engine_commands)):
                row[f"Engine{i + 1}"] = ""

            for engine in result["engines"]:
                engine_idx = engine["index"]
                engine_name = f"Engine{engine_idx + 1}"

                # Engine 1 is always 1.0, others show their ratio
                if engine_idx == 0:
                    row[engine_name] = 1.0
                else:
                    ratio = engine.get("vs_baseline_ratio")
                    row[engine_name] = ratio if ratio is not None else ""

            writer.writerow(row)

    print(f"\nResults exported to: {filename}")


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
        help='Command to run (including arguments). Must be specified at least twice. Example: -b "./hermes -w -Xjit"',
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
        "--percentage",
        action="store_true",
        help="Force percentage change display (default for 2 engines)",
    )
    parser.add_argument(
        "--ratio",
        action="store_true",
        help="Force ratio display (default for 3+ engines)",
    )
    parser.add_argument(
        "--compact",
        action="store_true",
        help="Hide detailed per-benchmark tables, show only summary tables",
    )
    parser.add_argument(
        "--csv",
        metavar="FILE",
        help="Export results to CSV file for graphing (in addition to normal output)",
    )
    parser.add_argument(
        "benchmarks",
        metavar="BENCHMARK.JS",
        nargs="+",  # Require at least one benchmark file
        help="Path to the benchmark file(s) to run.",
    )
    args = parser.parse_args()

    # Validate at least two binaries
    if len(args.binary) < 2:
        print(
            f"Error: At least two -b/--binary arguments required, got {len(args.binary)}.",
            file=sys.stderr,
        )
        sys.exit(1)

    if args.runs <= 0:
        print("Error: Number of runs must be positive.", file=sys.stderr)
        sys.exit(1)

    # Parse the command strings into lists
    engine_commands = [shlex.split(cmd) for cmd in args.binary]
    num_engines = len(engine_commands)

    # Determine display format
    if args.percentage and args.ratio:
        # Last one wins
        display_format = "ratio" if args.ratio else "percentage"
    elif args.percentage:
        display_format = "percentage"
    elif args.ratio:
        display_format = "ratio"
    else:
        # Default behavior
        display_format = "percentage" if num_engines == 2 else "ratio"

    # Print engine commands
    if num_engines == 2:
        # Backward compatibility for 2 engines
        print(f"OLD command: {' '.join(engine_commands[0])}")
        print(f"NEW command: {' '.join(engine_commands[1])}")
    else:
        for i, cmd in enumerate(engine_commands):
            print(f"Engine {i + 1} command: {' '.join(cmd)}")
    print()

    # Store summary results here
    summary_results = []

    # Store all results for multi-engine summary
    all_benchmark_results = []

    for filename in args.benchmarks:
        print(f"--- Running benchmark: {filename} ({args.runs} runs) ---")

        # Initialize data structures for multiple engines
        engine_scores = defaultdict(list)  # {engine_index: [scores]}

        # Run benchmarks for all engines
        for run_idx in range(args.runs):
            for engine_idx, engine_cmd in enumerate(engine_commands):
                # Determine display name for backward compatibility
                if num_engines == 2:
                    engine_name = "OLD" if engine_idx == 0 else "NEW"
                else:
                    engine_name = f"Engine {engine_idx + 1}"

                print(f"{engine_name} {filename} (Run {run_idx + 1}/{args.runs})")
                result = run_command(engine_cmd, filename)
                print(result.stdout, end="")
                score = find_first_score(result.stdout)

                if score is not None:
                    engine_scores[engine_idx].append(score)
                else:
                    print(
                        f"Warning: Score not found for {engine_name} {filename} run {run_idx + 1}/{args.runs}."
                    )

            print("-" * 20)  # Separator between runs

        # Process results for this benchmark
        benchmark_result = {"name": filename, "engines": []}

        # Calculate statistics for each engine
        baseline_avg = None
        for engine_idx in range(num_engines):
            scores = engine_scores[engine_idx]
            if not scores:
                continue

            avg_score = statistics.mean(scores)
            valid_runs = len(scores)

            # Set baseline from first engine
            if engine_idx == 0:
                baseline_avg = avg_score

            engine_data = {
                "index": engine_idx,
                "command": " ".join(engine_commands[engine_idx]),
                "scores": scores,
                "avg_score": avg_score,
                "valid_runs": valid_runs,
            }

            # Calculate standard deviation if we have enough runs
            if valid_runs >= 2:
                try:
                    engine_data["stdev"] = statistics.stdev(scores)
                except (statistics.StatisticsError, ValueError):
                    engine_data["stdev"] = None
            else:
                engine_data["stdev"] = None

            # Calculate confidence interval
            if engine_idx > 0 and valid_runs >= 2 and len(engine_scores[0]) >= 2:
                (low, high) = manual_ttest_confidence_interval(
                    scores, engine_scores[0], args.confidence
                )
                if low is not None and high is not None and baseline_avg != 0:
                    engine_data["confidence_interval"] = (
                        baseline_avg + low,
                        baseline_avg + high,
                    )
                else:
                    engine_data["confidence_interval"] = (None, None)
            else:
                engine_data["confidence_interval"] = (None, None)

            # Calculate comparison to baseline
            if baseline_avg and baseline_avg != 0:
                engine_data["vs_baseline_ratio"] = avg_score / baseline_avg
                engine_data["vs_baseline_pct"] = (
                    (avg_score - baseline_avg) / baseline_avg
                ) * 100
            else:
                engine_data["vs_baseline_ratio"] = None
                engine_data["vs_baseline_pct"] = None

            benchmark_result["engines"].append(engine_data)

        all_benchmark_results.append(benchmark_result)

        # For backward compatibility with 2 engines, maintain old summary format
        if num_engines == 2 and len(benchmark_result["engines"]) >= 2:
            engine0 = benchmark_result["engines"][0]
            engine1 = benchmark_result["engines"][1]

            if engine0["avg_score"] and engine0["avg_score"] != 0:
                avg_percentage_change = engine1["vs_baseline_pct"]

                # Calculate stdev of percentage changes
                individual_pct_changes = []
                for i in range(min(len(engine0["scores"]), len(engine1["scores"]))):
                    if engine0["scores"][i] != 0:
                        pct = (
                            (engine1["scores"][i] - engine0["scores"][i])
                            / engine0["scores"][i]
                        ) * 100
                        individual_pct_changes.append(pct)

                if len(individual_pct_changes) >= 2:
                    try:
                        stdev_percentage_change = statistics.stdev(
                            individual_pct_changes
                        )
                    except (statistics.StatisticsError, ValueError):
                        stdev_percentage_change = None
                else:
                    stdev_percentage_change = None

                # Get confidence interval
                conf_int = engine1.get("confidence_interval", (None, None))
                if conf_int[0] is not None and engine0["avg_score"] != 0:
                    pct_low = (
                        (conf_int[0] - engine0["avg_score"])
                        / engine0["avg_score"]
                        * 100
                    )
                    pct_high = (
                        (conf_int[1] - engine0["avg_score"])
                        / engine0["avg_score"]
                        * 100
                    )
                else:
                    pct_low, pct_high = None, None

                change_output_line = format_change_line(
                    avg_percentage_change,
                    stdev_percentage_change,
                    len(individual_pct_changes),
                )
                print(f"Summary for {filename}: {change_output_line}")

                # Store for legacy summary table
                summary_results.append(
                    {
                        "name": filename,
                        "avg_pct": avg_percentage_change,
                        "stdev_pct": stdev_percentage_change,
                        "ttest": (pct_low, pct_high),
                        "runs": len(individual_pct_changes),
                    }
                )
            else:
                print(f"Summary for {filename}: Change: N/A (baseline score is 0)")
                summary_results.append(
                    {
                        "name": filename,
                        "avg_pct": None,
                        "stdev_pct": None,
                        "ttest": (None, None),
                        "runs": 0,
                    }
                )

        # Print detailed per-benchmark table for multi-engine case
        if num_engines > 2 and not args.compact:
            print_per_benchmark_table(benchmark_result, display_format, args.confidence)

        print()  # Empty line before next benchmark

    # --- Print Final Summary Tables ---
    print("=" * 60)
    print("Overall Benchmark Summary")
    print("=" * 60)

    if num_engines == 2:
        # Use legacy format for 2 engines
        # 1. Table in Original Order
        print_summary_table(
            "Results (Original Order)", args.confidence, summary_results
        )

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
    else:
        # Use new compact format for 3+ engines
        # 1. Summary in original order
        print_compact_summary_table(
            "Summary Results (Original Order)",
            all_benchmark_results,
            display_format,
            num_engines,
            sort_by_last=False,
        )

        # 2. Summary sorted by last engine
        print_compact_summary_table(
            f"Summary Results (Sorted by engine{num_engines}/engine1)",
            all_benchmark_results,
            display_format,
            num_engines,
            sort_by_last=True,
        )

    # Export to CSV if requested
    if args.csv:
        export_to_csv(args.csv, all_benchmark_results, engine_commands)

    print("\nBenchmark Comparison Complete.")
