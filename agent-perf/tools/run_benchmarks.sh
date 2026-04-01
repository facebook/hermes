#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# run_benchmarks.sh — Benchmark runner for Static Hermes.
# Compiles each JS benchmark with shermes -exec, runs the binary,
# and extracts results from RESULT: lines (micro/macro) or
# "<Name> <Score>" lines (octane).
#
# Usage:
#   run_benchmarks.sh <shermes_binary> [micro|macro|octane|all] [--json]
#                     [--typed] [--iterations N] [--octane]
#                     [--shermes-args "extra args for shermes"]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/../benchmarks" && pwd)"
# Octane benchmarks live in the main benchmarks/ dir, not agent-perf/benchmarks/
SH_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
OCTANE_DIR="$SH_ROOT/benchmarks/octane"

# Defaults
SHERMES=""
SUITE="all"
ITERATIONS=5
TYPED=false
JSON_OUTPUT=false
OCTANE=false
SHERMES_EXTRA_ARGS=""

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> [micro|macro|all] [options]

Benchmark runner for Static Hermes. Uses shermes -exec to compile and
run JS benchmarks. Extracts ops/sec from RESULT: lines (micro/macro)
or "<Name> <Score>" lines (octane).

Arguments:
  <shermes_binary>       Path to the shermes compiler binary
  [micro|macro|octane|all]  Benchmark suite to run (default: all)

Options:
  --json                 Output structured JSON
  --typed                Compile with --typed flag
  --iterations <N>       Number of iterations per benchmark (default: $ITERATIONS)
  --octane               Also run benchmarks/octane/ suite
  --shermes-args "ARGS"  Extra arguments to pass to shermes
  -h, --help             Show this help

Output:
  For each benchmark, prints median ops/sec (or score for octane), mean,
  and stddev across N iterations. JSON mode outputs valid JSON (no inf/nan).

Examples:
  $(basename "$0") ./build/bin/shermes micro --json
  $(basename "$0") ./build/bin/shermes octane --iterations 5 --json
  $(basename "$0") \$SHERMES all --shermes-args '-L\$LIBS -Wc,-I\$INC @\$RESP'
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --json) JSON_OUTPUT=true; shift ;;
    --typed) TYPED=true; shift ;;
    --iterations) ITERATIONS="$2"; shift 2 ;;
    --octane) OCTANE=true; shift ;;
    --shermes-args) SHERMES_EXTRA_ARGS="$2"; shift 2 ;;
    -h|--help) usage ;;
    -*) echo "Error: Unknown option: $1" >&2; exit 1 ;;
    *)
      if [[ -z "$SHERMES" ]]; then
        SHERMES="$1"
      elif [[ "$1" == "micro" || "$1" == "macro" || "$1" == "octane" || "$1" == "all" ]]; then
        SUITE="$1"
      else
        echo "Error: Unknown argument: $1" >&2; exit 1
      fi
      shift
      ;;
  esac
done

if [[ -z "$SHERMES" ]]; then
  echo "Error: shermes binary path required" >&2
  usage
fi

if [[ ! -x "$SHERMES" ]]; then
  echo "Error: shermes binary not found or not executable: $SHERMES" >&2
  exit 1
fi

# Collect benchmark JS files for selected suite
collect_benchmarks() {
  local files=()
  if [[ "$SUITE" == "micro" || "$SUITE" == "all" ]]; then
    if [[ -d "$BENCH_DIR/micro" ]]; then
      while IFS= read -r f; do
        files+=("$f")
      done < <(find "$BENCH_DIR/micro" -name '*.js' -type f | sort)
    fi
  fi
  if [[ "$SUITE" == "macro" || "$SUITE" == "all" ]]; then
    if [[ -d "$BENCH_DIR/macro" ]]; then
      while IFS= read -r f; do
        files+=("$f")
      done < <(find "$BENCH_DIR/macro" -name '*.js' -type f | sort)
    fi
  fi
  if [[ "$SUITE" == "octane" || "$SUITE" == "all" ]] || $OCTANE; then
    if [[ -d "$OCTANE_DIR" ]]; then
      while IFS= read -r f; do
        files+=("$f")
      done < <(find "$OCTANE_DIR" -name '*.js' -type f | sort)
    else
      echo "Warning: octane directory not found at $OCTANE_DIR" >&2
    fi
  fi
  printf '%s\n' "${files[@]}"
}

# Run a benchmark with shermes -exec and return output
run_exec() {
  local js_file="$1"
  local -a cmd=("$SHERMES")
  if $TYPED; then
    cmd+=("--typed")
  fi
  # shellcheck disable=SC2206
  cmd+=($SHERMES_EXTRA_ARGS)
  cmd+=("-exec" "$js_file")
  "${cmd[@]}" 2>/dev/null
}

# Extract results from benchmark output.
# Handles two formats:
#   RESULT: <name> <ops_sec> ops/sec  (micro/macro benchmarks)
#   <Name> <Score>                     (octane benchmarks)
extract_results() {
  local output="$1"
  local is_octane="$2"
  while IFS= read -r line; do
    if [[ "$line" == RESULT:* ]]; then
      local name ops_sec
      name=$(echo "$line" | awk '{print $2}')
      ops_sec=$(echo "$line" | awk '{print $3}')
      echo "$name $ops_sec"
    elif $is_octane && [[ "$line" =~ ^[A-Za-z][A-Za-z0-9_-]*[[:space:]][0-9]+$ ]]; then
      local name score
      name=$(echo "$line" | awk '{print $1}')
      score=$(echo "$line" | awk '{print $2}')
      echo "$name $score"
    fi
  done <<< "$output"
}

# Compute statistics from a space-separated list of numbers.
# Output: median mean stddev (valid finite numbers, never inf/nan)
compute_stats() {
  local values="$1"
  python3 -c "
import math, sys
vals = [float(x) for x in '$values'.split() if x]
if not vals:
    print('0.00 0.00 0.00')
    sys.exit(0)
# Filter out inf/nan
vals = [v for v in vals if math.isfinite(v)]
if not vals:
    print('0.00 0.00 0.00')
    sys.exit(0)
vals.sort()
n = len(vals)
median = vals[n // 2]
mean = sum(vals) / n
variance = sum((x - mean) ** 2 for x in vals) / n
stddev = math.sqrt(variance)
print(f'{median:.2f} {mean:.2f} {stddev:.2f}')
"
}

# Main execution
mapfile -t BENCHMARKS < <(collect_benchmarks)

if [[ ${#BENCHMARKS[@]} -eq 0 ]]; then
  echo "Error: No benchmark files found for suite '$SUITE'" >&2
  exit 1
fi

# For each benchmark file, run N iterations with shermes -exec
# Accumulate per-sub-benchmark measurements
declare -A MEASUREMENTS  # key="file:sub_name" -> space-separated values

for bench_file in "${BENCHMARKS[@]}"; do
  bench_name=$(basename "$bench_file" .js)
  is_octane=false
  if [[ "$bench_file" == *"/octane/"* ]]; then
    is_octane=true
  fi

  echo "Running: $bench_file ($ITERATIONS iterations)" >&2
  for ((iter = 0; iter < ITERATIONS; iter++)); do
    output=$(run_exec "$bench_file" || true)
    while IFS= read -r result_line; do
      [[ -z "$result_line" ]] && continue
      local_name=$(echo "$result_line" | awk '{print $1}')
      local_ops=$(echo "$result_line" | awk '{print $2}')
      key="${bench_name}:${local_name}"
      if [[ -n "${MEASUREMENTS[$key]:-}" ]]; then
        MEASUREMENTS[$key]="${MEASUREMENTS[$key]} $local_ops"
      else
        MEASUREMENTS[$key]="$local_ops"
      fi
    done < <(extract_results "$output" "$is_octane")
  done
done

# Collect unique keys in order
KEYS=()
for bench_file in "${BENCHMARKS[@]}"; do
  bench_name=$(basename "$bench_file" .js)
  for key in "${!MEASUREMENTS[@]}"; do
    if [[ "$key" == "${bench_name}:"* ]]; then
      KEYS+=("$key")
    fi
  done
done
# Deduplicate while preserving order
UNIQUE_KEYS=()
declare -A SEEN_KEYS
for k in "${KEYS[@]}"; do
  if [[ -z "${SEEN_KEYS[$k]:-}" ]]; then
    UNIQUE_KEYS+=("$k")
    SEEN_KEYS[$k]=1
  fi
done

# Output
if $JSON_OUTPUT; then
  (
  echo "["
  first=true
  for key in "${UNIQUE_KEYS[@]}"; do
    values="${MEASUREMENTS[$key]}"
    stats=$(compute_stats "$values")
    read -r median mean stddev <<< "$stats"
    bench_name="${key%%:*}"
    sub_name="${key#*:}"
    full_name="${bench_name}/${sub_name}"

    if $first; then
      first=false
    else
      echo "  ,"
    fi
    cat <<ENDJSON
  {
    "name": "$full_name",
    "metric": "ops/sec",
    "metric_interpretation": "throughput (higher is better)",
    "median_ops_sec": $median,
    "mean_ops_sec": $mean,
    "stddev": $stddev,
    "iterations": $ITERATIONS
  }
ENDJSON
  done
  echo "]"
  ) | python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))"
else
  echo "Metric: ops/sec (throughput — higher is better)"
  echo ""
  printf "%-45s %15s %15s %12s %5s\n" \
    "Benchmark" "Median ops/sec" "Mean ops/sec" "Stddev" "N"
  printf "%-45s %15s %15s %12s %5s\n" \
    "---------" "--------------" "------------" "------" "-"
  for key in "${UNIQUE_KEYS[@]}"; do
    values="${MEASUREMENTS[$key]}"
    stats=$(compute_stats "$values")
    read -r median mean stddev <<< "$stats"
    bench_name="${key%%:*}"
    sub_name="${key#*:}"
    full_name="${bench_name}/${sub_name}"

    printf "%-45s %15.2f %15.2f %12.2f %5d\n" \
      "$full_name" "$median" "$mean" "$stddev" "$ITERATIONS"
  done
fi
