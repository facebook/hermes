#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pass_experiment.sh — Optimization pass experiments for Static Hermes.
# Measures the marginal value of a specific optimization pass by comparing
# full pipeline vs. pipeline with that pass excluded.
#
# Usage:
#   pass_experiment.sh <shermes_binary> <workload.js> --disable <pass_name>
#                      [--typed] [--json]

set -euo pipefail

# Defaults
SHERMES=""
JS_FILE=""
DISABLE_PASS=""
TYPED=false
JSON_OUTPUT=false

# Known passes
KNOWN_PASSES="dce cse mem2reg instsimplify simplifycfg stackpromotion typeinference inlining codemotion funcsigopts tdzdedup"

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> <workload.js> --disable <pass_name> [options]

Optimization pass experiments for Static Hermes. Measures the marginal
value of a specific pass by comparing full pipeline vs. pipeline with
that pass excluded.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Required:
  --disable <pass_name>  Name of the optimization pass to exclude

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  -h, --help             Show this help

Available passes:
  dce              Dead Code Elimination
  cse              Common Subexpression Elimination
  mem2reg          Memory to Register promotion
  instsimplify     Instruction Simplification
  simplifycfg      Control Flow Graph Simplification
  stackpromotion   Stack Promotion of heap allocations
  typeinference    Type Inference
  inlining         Function Inlining
  codemotion       Code Motion (hoisting/sinking)
  funcsigopts      Function Signature Optimizations
  tdzdedup         TDZ (Temporal Dead Zone) Deduplication

Reports:
  - Generated C code size difference
  - Native binary size difference
  - Execution performance difference (wall-clock)

Examples:
  $(basename "$0") ./build/bin/shermes test.js --disable inlining --json
  $(basename "$0") ./build/bin/shermes bench.js --disable dce --typed
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --disable) DISABLE_PASS="$2"; shift 2 ;;
    --typed) TYPED=true; shift ;;
    --json) JSON_OUTPUT=true; shift ;;
    -h|--help) usage ;;
    -*) echo "Error: Unknown option: $1" >&2; exit 1 ;;
    *)
      if [[ -z "$SHERMES" ]]; then
        SHERMES="$1"
      elif [[ -z "$JS_FILE" ]]; then
        JS_FILE="$1"
      fi
      shift
      ;;
  esac
done

if [[ -z "$SHERMES" ]] || [[ -z "$JS_FILE" ]] || [[ -z "$DISABLE_PASS" ]]; then
  echo "Error: shermes binary, JS file, and --disable <pass> required" >&2
  usage
fi

if [[ ! -x "$SHERMES" ]]; then
  echo "Error: shermes binary not found or not executable: $SHERMES" >&2
  exit 1
fi

if [[ ! -f "$JS_FILE" ]]; then
  echo "Error: JS file not found: $JS_FILE" >&2
  exit 1
fi

# Validate pass name
PASS_VALID=false
for p in $KNOWN_PASSES; do
  if [[ "$p" == "$DISABLE_PASS" ]]; then
    PASS_VALID=true
    break
  fi
done

if ! $PASS_VALID; then
  echo "Error: Unknown pass '$DISABLE_PASS'. Available passes:" >&2
  echo "  $KNOWN_PASSES" >&2
  exit 1
fi

# Build shermes base flags
shermes_base_flags() {
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  printf '%s\n' "${flags[@]}"
}

# Build the custom-opt string with one pass excluded
# The default pipeline (approximate) runs all passes.
# We use -custom-opt to specify the pipeline minus the disabled pass.
build_custom_opt() {
  local skip="$1"
  local all_passes="$KNOWN_PASSES"
  local pipeline=""
  for p in $all_passes; do
    if [[ "$p" != "$skip" ]]; then
      if [[ -n "$pipeline" ]]; then
        pipeline="${pipeline},${p}"
      else
        pipeline="$p"
      fi
    fi
  done
  echo "$pipeline"
}

# Measure wall-clock time (median of 3 runs)
measure_time_ms() {
  local binary="$1"
  local times=()
  for ((i = 0; i < 3; i++)); do
    local start end
    start=$(date +%s%N)
    "$binary" >/dev/null 2>&1 || true
    end=$(date +%s%N)
    times+=("$(( (end - start) / 1000000 ))")
  done
  # Return median
  local -a sorted
  mapfile -t sorted < <(printf '%s\n' "${times[@]}" | sort -n)
  echo "${sorted[1]}"
}

# Main
TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

mapfile -t BASE_FLAGS < <(shermes_base_flags)
CUSTOM_PIPELINE=$(build_custom_opt "$DISABLE_PASS")

# --- Full pipeline (baseline) ---
echo "Compiling with full pipeline..." >&2
C_FULL="$TMPDIR_BASE/full.c"
BIN_FULL="$TMPDIR_BASE/full"

"$SHERMES" "${BASE_FLAGS[@]}" -emit-c -o "$C_FULL" "$JS_FILE" 2>/dev/null
"$SHERMES" "${BASE_FLAGS[@]}" -o "$BIN_FULL" "$JS_FILE" 2>/dev/null

C_FULL_LINES=$(wc -l < "$C_FULL")
C_FULL_BYTES=$(stat -c%s "$C_FULL" 2>/dev/null || stat -f%z "$C_FULL" 2>/dev/null)
BIN_FULL_BYTES=$(stat -c%s "$BIN_FULL" 2>/dev/null || stat -f%z "$BIN_FULL" 2>/dev/null)

echo "Measuring full pipeline execution..." >&2
TIME_FULL=$(measure_time_ms "$BIN_FULL")

# --- Pipeline with pass disabled ---
echo "Compiling with '$DISABLE_PASS' disabled..." >&2
C_DISABLED="$TMPDIR_BASE/disabled.c"
BIN_DISABLED="$TMPDIR_BASE/disabled"

# Try -custom-opt flag; if unsupported, try -disable-pass
COMPILED_DISABLED=false
if "$SHERMES" --help 2>&1 | grep -q '\-custom-opt'; then
  if "$SHERMES" "${BASE_FLAGS[@]}" -custom-opt="$CUSTOM_PIPELINE" -emit-c -o "$C_DISABLED" "$JS_FILE" 2>/dev/null; then
    "$SHERMES" "${BASE_FLAGS[@]}" -custom-opt="$CUSTOM_PIPELINE" -o "$BIN_DISABLED" "$JS_FILE" 2>/dev/null
    COMPILED_DISABLED=true
  fi
fi

if ! $COMPILED_DISABLED; then
  # Try -disable-pass flag
  if "$SHERMES" "${BASE_FLAGS[@]}" -disable-pass="$DISABLE_PASS" -emit-c -o "$C_DISABLED" "$JS_FILE" 2>/dev/null; then
    "$SHERMES" "${BASE_FLAGS[@]}" -disable-pass="$DISABLE_PASS" -o "$BIN_DISABLED" "$JS_FILE" 2>/dev/null
    COMPILED_DISABLED=true
  fi
fi

if ! $COMPILED_DISABLED; then
  echo "Warning: Could not disable pass '$DISABLE_PASS'. Neither -custom-opt nor -disable-pass worked." >&2
  echo "Showing baseline results only." >&2
  C_DISABLED="$C_FULL"
  BIN_DISABLED="$BIN_FULL"
fi

C_DISABLED_LINES=$(wc -l < "$C_DISABLED")
C_DISABLED_BYTES=$(stat -c%s "$C_DISABLED" 2>/dev/null || stat -f%z "$C_DISABLED" 2>/dev/null)
BIN_DISABLED_BYTES=$(stat -c%s "$BIN_DISABLED" 2>/dev/null || stat -f%z "$BIN_DISABLED" 2>/dev/null)

echo "Measuring disabled pipeline execution..." >&2
TIME_DISABLED=$(measure_time_ms "$BIN_DISABLED")

# Compute deltas
C_LINES_DELTA=$(python3 -c "print(f'{(($C_DISABLED_LINES - $C_FULL_LINES) / max($C_FULL_LINES, 1)) * 100:.2f}')")
BIN_DELTA=$(python3 -c "print(f'{(($BIN_DISABLED_BYTES - $BIN_FULL_BYTES) / max($BIN_FULL_BYTES, 1)) * 100:.2f}')")
TIME_DELTA=$(python3 -c "print(f'{(($TIME_DISABLED - $TIME_FULL) / max($TIME_FULL, 1)) * 100:.2f}')")

# Output
if $JSON_OUTPUT; then
  python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "typed": $TYPED,
  "disabled_pass": "$DISABLE_PASS",
  "pass_available": $COMPILED_DISABLED,
  "baseline": {
    "c_lines": $C_FULL_LINES,
    "c_bytes": $C_FULL_BYTES,
    "binary_bytes": $BIN_FULL_BYTES,
    "execution_time_ms": $TIME_FULL
  },
  "with_pass_disabled": {
    "c_lines": $C_DISABLED_LINES,
    "c_bytes": $C_DISABLED_BYTES,
    "binary_bytes": $BIN_DISABLED_BYTES,
    "execution_time_ms": $TIME_DISABLED
  },
  "marginal_value": {
    "c_lines_delta_pct": $C_LINES_DELTA,
    "binary_size_delta_pct": $BIN_DELTA,
    "execution_time_delta_pct": $TIME_DELTA
  }
}
ENDJSON
else
  echo "Pass Experiment: $JS_FILE"
  echo "Disabled pass: $DISABLE_PASS"
  echo "Typed: $TYPED"
  echo ""
  printf "%-25s %15s %15s %12s\n" "" "Full Pipeline" "Pass Disabled" "Delta %"
  printf "%-25s %15s %15s %12s\n" "" "-------------" "-------------" "-------"
  printf "%-25s %15d %15d %12s\n" "C code lines:" "$C_FULL_LINES" "$C_DISABLED_LINES" "${C_LINES_DELTA}%"
  printf "%-25s %15d %15d %12s\n" "Binary size (bytes):" "$BIN_FULL_BYTES" "$BIN_DISABLED_BYTES" "${BIN_DELTA}%"
  printf "%-25s %15d %15d %12s\n" "Execution time (ms):" "$TIME_FULL" "$TIME_DISABLED" "${TIME_DELTA}%"
  echo ""
  if ! $COMPILED_DISABLED; then
    echo "NOTE: Pass disabling not supported by this shermes build."
    echo "      Showing identical results for both columns."
  fi
fi
