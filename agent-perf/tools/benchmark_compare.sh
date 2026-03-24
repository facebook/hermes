#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# benchmark_compare.sh — Statistical A/B comparison of two shermes binaries.
# Compiles each benchmark with both shermes binaries, uses interleaved
# execution for bias reduction, and performs Mann-Whitney U test for
# statistical significance.
#
# Usage:
#   benchmark_compare.sh <shermes_a> <shermes_b> [--suite micro|macro|all]
#                        [--json] [--typed] [--iterations N] [--threshold PCT]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/../benchmarks" && pwd)"

# Defaults
SHERMES_A=""
SHERMES_B=""
SUITE="all"
ITERATIONS=11
TYPED=false
JSON_OUTPUT=false
THRESHOLD=2.0

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_a> <shermes_b> [options]

Statistical A/B comparison of two Static Hermes compiler binaries.

Arguments:
  <shermes_a>            Path to baseline shermes binary (A)
  <shermes_b>            Path to candidate shermes binary (B)

Options:
  --suite <s>            Benchmark suite: micro, macro, all (default: $SUITE)
  --json                 Output structured JSON
  --typed                Compile with --typed flag
  --iterations <N>       Iterations per benchmark (default: $ITERATIONS)
  --threshold <PCT>      Regression threshold percentage (default: $THRESHOLD)
  -h, --help             Show this help

Methodology:
  - Compiles each benchmark with both shermes binaries
  - Interleaves A/B execution to reduce ordering bias
  - Uses perf stat -e instructions:u if available, wall-clock otherwise
  - Mann-Whitney U test for significance (scipy or builtin fallback)
  - Reports per-benchmark deltas and geometric mean delta
  - Exit code 1 if any benchmark regresses beyond threshold

Examples:
  $(basename "$0") ./build-old/bin/shermes ./build-new/bin/shermes
  $(basename "$0") ./a/shermes ./b/shermes --suite micro --json --iterations 21
  $(basename "$0") ./a/shermes ./b/shermes --threshold 5.0
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --suite) SUITE="$2"; shift 2 ;;
    --json) JSON_OUTPUT=true; shift ;;
    --typed) TYPED=true; shift ;;
    --iterations) ITERATIONS="$2"; shift 2 ;;
    --threshold) THRESHOLD="$2"; shift 2 ;;
    -h|--help) usage ;;
    -*) echo "Error: Unknown option: $1" >&2; exit 1 ;;
    *)
      if [[ -z "$SHERMES_A" ]]; then
        SHERMES_A="$1"
      elif [[ -z "$SHERMES_B" ]]; then
        SHERMES_B="$1"
      else
        echo "Error: Unexpected argument: $1" >&2; exit 1
      fi
      shift
      ;;
  esac
done

if [[ -z "$SHERMES_A" ]] || [[ -z "$SHERMES_B" ]]; then
  echo "Error: Two shermes binaries required" >&2
  usage
fi

for bin in "$SHERMES_A" "$SHERMES_B"; do
  if [[ ! -x "$bin" ]]; then
    echo "Error: Binary not found or not executable: $bin" >&2
    exit 1
  fi
done

# Check for perf
HAS_PERF=false
if command -v perf &>/dev/null && perf stat -e instructions true 2>/dev/null; then
  HAS_PERF=true
fi

METRIC_NAME="instructions"
if ! $HAS_PERF; then
  METRIC_NAME="nanoseconds"
fi

# Collect benchmark files
collect_benchmarks() {
  local files=()
  if [[ "$SUITE" == "micro" || "$SUITE" == "all" ]]; then
    if [[ -d "$BENCH_DIR/micro" ]]; then
      while IFS= read -r f; do files+=("$f"); done < <(find "$BENCH_DIR/micro" -name '*.js' -type f | sort)
    fi
  fi
  if [[ "$SUITE" == "macro" || "$SUITE" == "all" ]]; then
    if [[ -d "$BENCH_DIR/macro" ]]; then
      while IFS= read -r f; do files+=("$f"); done < <(find "$BENCH_DIR/macro" -name '*.js' -type f | sort)
    fi
  fi
  printf '%s\n' "${files[@]}"
}

# Build shermes flags
shermes_flags() {
  local flags=()
  if $TYPED; then flags+=("--typed"); fi
  printf '%s\n' "${flags[@]}"
}

# Measure a single execution
measure_once() {
  local binary="$1"
  if $HAS_PERF; then
    perf stat -e instructions:u -x',' "$binary" 2>&1 >/dev/null | \
      grep 'instructions' | cut -d',' -f1 | tr -d ' '
  else
    local start end
    start=$(date +%s%N)
    "$binary" >/dev/null 2>&1
    end=$(date +%s%N)
    echo $((end - start))
  fi
}

# Main
mapfile -t BENCHMARKS < <(collect_benchmarks)

if [[ ${#BENCHMARKS[@]} -eq 0 ]]; then
  echo "Error: No benchmark files found for suite '$SUITE'" >&2
  exit 1
fi

TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

mapfile -t FLAGS < <(shermes_flags)

# Per-benchmark results storage
declare -a BENCH_NAMES=()
declare -a MEDIANS_A=()
declare -a MEDIANS_B=()
declare -a DELTAS=()
declare -a P_VALUES=()
declare -a SIGNIFICANTS=()
declare -a REGRESSIONS=()

HAS_REGRESSION=false

for bench_file in "${BENCHMARKS[@]}"; do
  bench_name=$(basename "$bench_file" .js)
  BENCH_NAMES+=("$bench_name")

  binary_a="$TMPDIR_BASE/${bench_name}_a"
  binary_b="$TMPDIR_BASE/${bench_name}_b"

  echo "Compiling: $bench_file" >&2
  if ! "$SHERMES_A" "${FLAGS[@]}" -o "$binary_a" "$bench_file" 2>/dev/null; then
    echo "Warning: Failed to compile with A: $bench_file, skipping" >&2
    MEDIANS_A+=("0"); MEDIANS_B+=("0"); DELTAS+=("0"); P_VALUES+=("1.0"); SIGNIFICANTS+=("false"); REGRESSIONS+=("false")
    continue
  fi
  if ! "$SHERMES_B" "${FLAGS[@]}" -o "$binary_b" "$bench_file" 2>/dev/null; then
    echo "Warning: Failed to compile with B: $bench_file, skipping" >&2
    MEDIANS_A+=("0"); MEDIANS_B+=("0"); DELTAS+=("0"); P_VALUES+=("1.0"); SIGNIFICANTS+=("false"); REGRESSIONS+=("false")
    continue
  fi

  # Interleaved execution
  measurements_a=""
  measurements_b=""
  echo "Running $ITERATIONS interleaved iterations for $bench_name..." >&2
  for ((iter = 0; iter < ITERATIONS; iter++)); do
    val_a=$(measure_once "$binary_a")
    val_b=$(measure_once "$binary_b")
    if [[ -n "$measurements_a" ]]; then
      measurements_a="$measurements_a $val_a"
      measurements_b="$measurements_b $val_b"
    else
      measurements_a="$val_a"
      measurements_b="$val_b"
    fi
  done

  # Statistical analysis via python
  read -r median_a median_b delta_pct p_value significant is_regression <<< "$(python3 -c "
import math

a = [float(x) for x in '$measurements_a'.split()]
b = [float(x) for x in '$measurements_b'.split()]

a.sort()
b.sort()
n = len(a)
mid = n // 2
median_a = a[mid]
median_b = b[mid]

if median_a > 0:
    delta_pct = ((median_b - median_a) / median_a) * 100
else:
    delta_pct = 0.0

# Mann-Whitney U test
try:
    from scipy.stats import mannwhitneyu
    stat, p = mannwhitneyu(a, b, alternative='two-sided')
except ImportError:
    combined = sorted([(v, 'a') for v in a] + [(v, 'b') for v in b], key=lambda x: x[0])
    rank_sum_a = sum(i+1 for i, (v, g) in enumerate(combined) if g == 'a')
    n1, n2 = len(a), len(b)
    U = rank_sum_a - n1 * (n1 + 1) / 2
    mu = n1 * n2 / 2
    sigma = math.sqrt(n1 * n2 * (n1 + n2 + 1) / 12)
    z = (U - mu) / sigma if sigma > 0 else 0
    p = 2 * (1 - 0.5 * (1 + math.erf(abs(z) / math.sqrt(2))))

significant = p < 0.05
is_regression = significant and delta_pct > $THRESHOLD

print(f'{median_a:.2f} {median_b:.2f} {delta_pct:.4f} {p:.6f} {\"true\" if significant else \"false\"} {\"true\" if is_regression else \"false\"}')
")"

  MEDIANS_A+=("$median_a")
  MEDIANS_B+=("$median_b")
  DELTAS+=("$delta_pct")
  P_VALUES+=("$p_value")
  SIGNIFICANTS+=("$significant")
  REGRESSIONS+=("$is_regression")

  if [[ "$is_regression" == "true" ]]; then
    HAS_REGRESSION=true
  fi
done

# Compute geometric mean of absolute deltas
GEO_MEAN=$(python3 -c "
import math
deltas = [float(x) for x in '${DELTAS[*]}'.split()]
# Geometric mean of (1 + delta/100) - 1, expressed as percentage
products = [1.0 + d / 100.0 for d in deltas]
if products:
    geo = math.exp(sum(math.log(abs(p)) for p in products) / len(products))
    sign = 1 if all(p > 0 for p in products) else -1
    print(f'{(sign * geo - 1) * 100:.4f}')
else:
    print('0.0000')
")

# Output
if $JSON_OUTPUT; then
  (
  cat <<ENDJSON
{
  "shermes_a": "$SHERMES_A",
  "shermes_b": "$SHERMES_B",
  "suite": "$SUITE",
  "metric": "$METRIC_NAME",
  "metric_interpretation": "cost (lower is better)",
  "delta_interpretation": "positive = regression (more cost), negative = improvement (less cost)",
  "typed": $TYPED,
  "iterations": $ITERATIONS,
  "threshold_pct": $THRESHOLD,
  "geometric_mean_delta_pct": $GEO_MEAN,
  "has_regression": $HAS_REGRESSION,
  "benchmarks": [
ENDJSON
  for i in "${!BENCH_NAMES[@]}"; do
    [[ $i -gt 0 ]] && echo "    ,"
    cat <<ENDJSON
    {
      "name": "${BENCH_NAMES[$i]}",
      "median_a": ${MEDIANS_A[$i]},
      "median_b": ${MEDIANS_B[$i]},
      "delta_pct": ${DELTAS[$i]},
      "p_value": ${P_VALUES[$i]},
      "significant": ${SIGNIFICANTS[$i]},
      "regression": ${REGRESSIONS[$i]}
    }
ENDJSON
  done
  cat <<ENDJSON
  ]
}
ENDJSON
  ) | python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))"
else
  echo "A/B Comparison: $METRIC_NAME"
  echo "  A (baseline):  $SHERMES_A"
  echo "  B (candidate): $SHERMES_B"
  echo "  Suite: $SUITE | Typed: $TYPED | Iterations: $ITERATIONS | Threshold: ${THRESHOLD}%"
  echo ""
  echo "  Metric: $METRIC_NAME (cost — lower is better)"
  echo "  Delta %: positive = more $METRIC_NAME = regression; negative = improvement"
  echo ""
  printf "%-30s %15s %15s %10s %10s %6s\n" \
    "Benchmark" "Median A" "Median B" "Delta %" "p-value" "Regr?"
  printf "%-30s %15s %15s %10s %10s %6s\n" \
    "---------" "--------" "--------" "-------" "-------" "-----"
  for i in "${!BENCH_NAMES[@]}"; do
    local_flag=""
    if [[ "${REGRESSIONS[$i]}" == "true" ]]; then
      local_flag="YES"
    fi
    printf "%-30s %15s %15s %10s %10s %6s\n" \
      "${BENCH_NAMES[$i]}" "${MEDIANS_A[$i]}" "${MEDIANS_B[$i]}" \
      "${DELTAS[$i]}%" "${P_VALUES[$i]}" "$local_flag"
  done
  echo ""
  echo "Geometric mean delta: ${GEO_MEAN}%"
  if $HAS_REGRESSION; then
    echo "REGRESSION DETECTED (threshold: ${THRESHOLD}%)"
  fi
fi

if $HAS_REGRESSION; then
  exit 1
fi
