#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# count_instructions.sh — Measure retired CPU instruction counts for
# native binaries produced by shermes.
#
# Uses perf stat -e instructions for deterministic measurement.
# Falls back to wall-clock nanoseconds if perf is unavailable.
#
# Usage:
#   count_instructions.sh <js_file> [--shermes <path>] [--iterations N]
#                         [--typed] [--json] [--compile-only]
#   count_instructions.sh <js_file> --compare <shermes_a> <shermes_b>
#                         [--iterations N] [--typed] [--json]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Defaults
ITERATIONS=11
SHERMES=""
SHERMES_A=""
SHERMES_B=""
JS_FILE=""
TYPED=false
JSON_OUTPUT=false
COMPARE_MODE=false
COMPILE_ONLY=false

usage() {
  cat <<EOF
Usage: $(basename "$0") <js_file> [options]

Measure retired CPU instruction counts for Static Hermes native binaries.

Options:
  --shermes <path>       Path to shermes binary (auto-detected if not set)
  --compare <a> <b>      A/B comparison mode with two shermes binaries
  --iterations <N>       Number of iterations (default: $ITERATIONS)
  --typed                Compile with --typed flag
  --compile-only         Measure compilation instructions only (no execution)
  --json                 Output structured JSON
  -h, --help             Show this help

Examples:
  $(basename "$0") test.js --shermes ./build/bin/shermes
  $(basename "$0") test.js --compare ./build-a/bin/shermes ./build-b/bin/shermes --json
  $(basename "$0") test.js --typed --iterations 21
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --shermes) SHERMES="$2"; shift 2 ;;
    --compare) COMPARE_MODE=true; SHERMES_A="$2"; SHERMES_B="$3"; shift 3 ;;
    --iterations) ITERATIONS="$2"; shift 2 ;;
    --typed) TYPED=true; shift ;;
    --compile-only) COMPILE_ONLY=true; shift ;;
    --json) JSON_OUTPUT=true; shift ;;
    -h|--help) usage ;;
    -*) echo "Unknown option: $1" >&2; exit 1 ;;
    *) JS_FILE="$1"; shift ;;
  esac
done

if [[ -z "$JS_FILE" ]]; then
  echo "Error: JavaScript file required" >&2
  usage
fi

if [[ ! -f "$JS_FILE" ]]; then
  echo "Error: File not found: $JS_FILE" >&2
  exit 1
fi

# Check if perf is available
HAS_PERF=false
if command -v perf &>/dev/null && perf stat -e instructions true 2>/dev/null; then
  HAS_PERF=true
fi

# Auto-detect shermes if not provided
find_shermes() {
  local search_paths=(
    "$REPO_ROOT/cmake-build-release/bin/shermes"
    "$REPO_ROOT/cmake-build-debug/bin/shermes"
  )
  for p in "${search_paths[@]}"; do
    if [[ -x "$p" ]]; then
      echo "$p"
      return 0
    fi
  done
  return 1
}

if [[ -z "$SHERMES" ]] && ! $COMPARE_MODE; then
  SHERMES=$(find_shermes) || {
    echo "Error: Cannot find shermes binary. Use --shermes <path>" >&2
    exit 1
  }
fi

# Build shermes flags array
build_shermes_flags() {
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  echo "${flags[@]}"
}

# Compile JS to native binary
compile_to_native() {
  local shermes_bin="$1"
  local js_file="$2"
  local output_binary="$3"
  local -a flags=()
  if $TYPED; then flags+=("--typed"); fi

  # Compile to executable
  "$shermes_bin" "${flags[@]}" -o "$output_binary" "$js_file" 2>/dev/null
}

# Measure instruction count for a single run
measure_instructions() {
  local binary="$1"
  if $HAS_PERF; then
    perf stat -e instructions:u -x',' "$binary" 2>&1 >/dev/null | \
      grep 'instructions' | cut -d',' -f1 | tr -d ' '
  else
    # Fallback: wall-clock nanoseconds
    local start end
    start=$(date +%s%N)
    "$binary" >/dev/null 2>&1
    end=$(date +%s%N)
    echo $((end - start))
  fi
}

# Measure compilation instruction count
measure_compile_instructions() {
  local shermes_bin="$1"
  local js_file="$2"
  local output_binary="$3"
  local -a flags=()
  if $TYPED; then flags+=("--typed"); fi

  if $HAS_PERF; then
    perf stat -e instructions:u -x',' "$shermes_bin" "${flags[@]}" -o "$output_binary" "$js_file" 2>&1 >/dev/null | \
      grep 'instructions' | cut -d',' -f1 | tr -d ' '
  else
    local start end
    start=$(date +%s%N)
    "$shermes_bin" "${flags[@]}" -o "$output_binary" "$js_file" >/dev/null 2>&1
    end=$(date +%s%N)
    echo $((end - start))
  fi
}

# Collect N measurements and compute median
collect_measurements() {
  local binary="$1"
  local n="$2"
  local mode="${3:-exec}"  # exec or compile
  local shermes_bin="${4:-}"
  local js_file="${5:-}"
  local tmp_binary="${6:-}"

  local measurements=()
  for ((i = 0; i < n; i++)); do
    local val
    if [[ "$mode" == "compile" ]]; then
      val=$(measure_compile_instructions "$shermes_bin" "$js_file" "$tmp_binary")
    else
      val=$(measure_instructions "$binary")
    fi
    measurements+=("$val")
  done

  # Sort and find median
  local sorted
  mapfile -t sorted < <(printf '%s\n' "${measurements[@]}" | sort -n)
  local mid=$((n / 2))
  local median="${sorted[$mid]}"

  # Compute mean
  local sum=0
  for m in "${measurements[@]}"; do
    sum=$((sum + m))
  done
  local mean=$((sum / n))

  echo "$median $mean ${measurements[*]}"
}

# Single measurement mode
run_single() {
  local shermes_bin="$1"
  local tmpdir
  tmpdir=$(mktemp -d)
  trap 'rm -rf "$tmpdir"' EXIT

  local native_binary="$tmpdir/bench_binary"

  # Compile
  compile_to_native "$shermes_bin" "$JS_FILE" "$native_binary"
  local binary_size
  binary_size=$(stat -c%s "$native_binary" 2>/dev/null || stat -f%z "$native_binary" 2>/dev/null)

  local metric_name="instructions"
  if ! $HAS_PERF; then
    metric_name="nanoseconds"
  fi

  if $COMPILE_ONLY; then
    local result
    result=$(collect_measurements "" "$ITERATIONS" "compile" "$shermes_bin" "$JS_FILE" "$native_binary")
    local median mean
    read -r median mean _ <<< "$result"

    if $JSON_OUTPUT; then
      python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "mode": "compile",
  "metric": "$metric_name",
  "typed": $TYPED,
  "iterations": $ITERATIONS,
  "median": $median,
  "mean": $mean,
  "binary_size_bytes": $binary_size
}
ENDJSON
    else
      echo "File: $JS_FILE"
      echo "Mode: compilation only"
      echo "Metric: $metric_name"
      echo "Typed: $TYPED"
      echo "Iterations: $ITERATIONS"
      echo "Median: $median"
      echo "Mean: $mean"
      echo "Binary size: $binary_size bytes"
    fi
  else
    # Measure execution
    local result
    result=$(collect_measurements "$native_binary" "$ITERATIONS" "exec")
    local median mean
    read -r median mean _ <<< "$result"

    if $JSON_OUTPUT; then
      python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "mode": "execute",
  "metric": "$metric_name",
  "typed": $TYPED,
  "iterations": $ITERATIONS,
  "median": $median,
  "mean": $mean,
  "binary_size_bytes": $binary_size
}
ENDJSON
    else
      echo "File: $JS_FILE"
      echo "Mode: execution"
      echo "Metric: $metric_name"
      echo "Typed: $TYPED"
      echo "Iterations: $ITERATIONS"
      echo "Median: $median"
      echo "Mean: $mean"
      echo "Binary size: $binary_size bytes"
    fi
  fi
}

# A/B comparison mode
run_compare() {
  local tmpdir
  tmpdir=$(mktemp -d)
  trap 'rm -rf "$tmpdir"' EXIT

  local binary_a="$tmpdir/binary_a"
  local binary_b="$tmpdir/binary_b"

  echo "Compiling with binary A: $SHERMES_A" >&2
  compile_to_native "$SHERMES_A" "$JS_FILE" "$binary_a"
  local size_a
  size_a=$(stat -c%s "$binary_a" 2>/dev/null || stat -f%z "$binary_a" 2>/dev/null)

  echo "Compiling with binary B: $SHERMES_B" >&2
  compile_to_native "$SHERMES_B" "$JS_FILE" "$binary_b"
  local size_b
  size_b=$(stat -c%s "$binary_b" 2>/dev/null || stat -f%z "$binary_b" 2>/dev/null)

  local metric_name="instructions"
  if ! $HAS_PERF; then
    metric_name="nanoseconds"
  fi

  # Interleaved execution
  local measurements_a=()
  local measurements_b=()
  echo "Running $ITERATIONS interleaved iterations..." >&2
  for ((i = 0; i < ITERATIONS; i++)); do
    local val_a val_b
    val_a=$(measure_instructions "$binary_a")
    val_b=$(measure_instructions "$binary_b")
    measurements_a+=("$val_a")
    measurements_b+=("$val_b")
  done

  # Compute medians
  local sorted_a sorted_b
  mapfile -t sorted_a < <(printf '%s\n' "${measurements_a[@]}" | sort -n)
  mapfile -t sorted_b < <(printf '%s\n' "${measurements_b[@]}" | sort -n)
  local mid=$((ITERATIONS / 2))
  local median_a="${sorted_a[$mid]}"
  local median_b="${sorted_b[$mid]}"

  # Compute delta
  local delta_pct
  if [[ "$median_a" -ne 0 ]]; then
    delta_pct=$(python3 -c "print(f'{(($median_b - $median_a) / $median_a) * 100:.3f}')")
  else
    delta_pct="0.000"
  fi

  # Mann-Whitney U test for significance
  local p_value significant
  p_value=$(python3 -c "
import sys
try:
    from scipy.stats import mannwhitneyu
    a = [${measurements_a[*]// /,}]
    b = [${measurements_b[*]// /,}]
    stat, p = mannwhitneyu(a, b, alternative='two-sided')
    print(f'{p:.6f}')
except ImportError:
    # Fallback: simple normal approximation
    import math
    a = sorted([${measurements_a[*]// /,}])
    b = sorted([${measurements_b[*]// /,}])
    n1, n2 = len(a), len(b)
    combined = sorted([(v, 'a') for v in a] + [(v, 'b') for v in b], key=lambda x: x[0])
    rank_sum_a = sum(i+1 for i, (v, g) in enumerate(combined) if g == 'a')
    U = rank_sum_a - n1 * (n1 + 1) / 2
    mu = n1 * n2 / 2
    sigma = math.sqrt(n1 * n2 * (n1 + n2 + 1) / 12)
    z = (U - mu) / sigma if sigma > 0 else 0
    p = 2 * (1 - 0.5 * (1 + math.erf(abs(z) / math.sqrt(2))))
    print(f'{p:.6f}')
" 2>/dev/null || echo "1.000000")

  significant=$(python3 -c "print('true' if float('$p_value') < 0.05 else 'false')")

  if $JSON_OUTPUT; then
    python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "mode": "compare",
  "metric": "$metric_name",
  "typed": $TYPED,
  "iterations": $ITERATIONS,
  "binary_a": {
    "shermes": "$SHERMES_A",
    "median": $median_a,
    "binary_size_bytes": $size_a
  },
  "binary_b": {
    "shermes": "$SHERMES_B",
    "median": $median_b,
    "binary_size_bytes": $size_b
  },
  "delta_pct": $delta_pct,
  "p_value": $p_value,
  "significant": $significant
}
ENDJSON
  else
    echo "File: $JS_FILE"
    echo "Metric: $metric_name"
    echo "Typed: $TYPED"
    echo "Iterations: $ITERATIONS"
    echo ""
    echo "Binary A ($SHERMES_A):"
    echo "  Median: $median_a"
    echo "  Binary size: $size_a bytes"
    echo ""
    echo "Binary B ($SHERMES_B):"
    echo "  Median: $median_b"
    echo "  Binary size: $size_b bytes"
    echo ""
    echo "Delta: ${delta_pct}%"
    echo "p-value: $p_value"
    echo "Significant: $significant"
  fi

  # Exit 1 if regression (B is significantly worse than A)
  if [[ "$significant" == "true" ]]; then
    local is_regression
    is_regression=$(python3 -c "print('true' if float('$delta_pct') > 2.0 else 'false')")
    if [[ "$is_regression" == "true" ]]; then
      exit 1
    fi
  fi
}

# Main
if $COMPARE_MODE; then
  run_compare
else
  run_single "$SHERMES"
fi
