#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# compilation_profiler.sh — Compilation pipeline profiler for Static Hermes.
# Measures time for each compilation stage separately:
#   a. JS -> C (shermes -emit-c)
#   b. C -> object (cc -c)
#   c. Linking (shermes full pipeline)
# Reports IR statistics at each dump level if available.
#
# Usage:
#   compilation_profiler.sh <shermes_binary> <workload.js> [--typed] [--json]

set -euo pipefail

# Defaults
SHERMES=""
JS_FILE=""
TYPED=false
JSON_OUTPUT=false

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> <workload.js> [options]

Compilation pipeline profiler for Static Hermes.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  -h, --help             Show this help

Measures each compilation stage independently:
  Stage 1: JS -> C          (shermes -emit-c)
  Stage 2: C -> object      (cc -c on the generated C)
  Stage 3: Full pipeline    (shermes -o, end-to-end including linking)

Also collects file size metrics at each stage and reports IR statistics
if shermes supports dumping them.

Examples:
  $(basename "$0") ./build/bin/shermes test.js --json
  $(basename "$0") ./build/bin/shermes large_app.js --typed
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
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

if [[ -z "$SHERMES" ]] || [[ -z "$JS_FILE" ]]; then
  echo "Error: shermes binary and JS workload file required" >&2
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

# Build shermes flags
shermes_flags() {
  if $TYPED; then echo "--typed"; fi
}

# Get file size cross-platform
file_size() {
  stat -c%s "$1" 2>/dev/null || stat -f%z "$1" 2>/dev/null || echo 0
}

# Measure command execution time in milliseconds (median of N runs)
measure_ms() {
  local n="${1:-3}"
  shift
  local cmd=("$@")
  local times=()

  for ((i = 0; i < n; i++)); do
    local start end
    start=$(date +%s%N)
    "${cmd[@]}" >/dev/null 2>&1 || true
    end=$(date +%s%N)
    times+=("$(( (end - start) / 1000000 ))")
  done

  # Return median
  local sorted
  mapfile -t sorted < <(printf '%s\n' "${times[@]}" | sort -n)
  local mid=$((n / 2))
  echo "${sorted[$mid]}"
}

# Find CC compiler
find_cc() {
  if command -v cc &>/dev/null; then echo "cc"
  elif command -v gcc &>/dev/null; then echo "gcc"
  elif command -v clang &>/dev/null; then echo "clang"
  else echo ""; fi
}

# Main
TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

# Build flags array
declare -a FLAGS=()
if $TYPED; then
  FLAGS+=("--typed")
fi
JS_SIZE=$(file_size "$JS_FILE")
JS_LINES=$(wc -l < "$JS_FILE")

CC=$(find_cc)
if [[ -z "$CC" ]]; then
  echo "Warning: No C compiler found; Stage 2 (C -> object) will be skipped" >&2
fi

# --- Stage 1: JS -> C ---
echo "Stage 1: JS -> C (shermes -emit-c)..." >&2
C_FILE="$TMPDIR_BASE/output.c"
STAGE1_MS=$(measure_ms 3 "$SHERMES" "${FLAGS[@]}" -emit-c -o "$C_FILE" "$JS_FILE")

# Generate once more to get the actual file
"$SHERMES" "${FLAGS[@]}" -emit-c -o "$C_FILE" "$JS_FILE" 2>/dev/null
C_SIZE=$(file_size "$C_FILE")
C_LINES=$(wc -l < "$C_FILE")

# Count functions and runtime calls in generated C
FUNC_COUNT=$(grep -c '^static SHLegacyValue\|^static SHLegacyResult' "$C_FILE" 2>/dev/null || echo 0)
SH_CALL_COUNT=$(grep -c '_sh_' "$C_FILE" 2>/dev/null || echo 0)

# --- Stage 2: C -> object ---
STAGE2_MS=0
OBJ_SIZE=0
if [[ -n "$CC" ]]; then
  echo "Stage 2: C -> object ($CC -c)..." >&2
  OBJ_FILE="$TMPDIR_BASE/output.o"

  # We need the shermes include path for SH headers
  SHERMES_DIR=$(dirname "$SHERMES")
  INCLUDE_DIRS=""
  # Try common include paths relative to shermes binary
  for inc_dir in "$SHERMES_DIR/../include" "$SHERMES_DIR/../../include" "$SHERMES_DIR/../lib"; do
    if [[ -d "$inc_dir" ]]; then
      INCLUDE_DIRS="$INCLUDE_DIRS -I$inc_dir"
    fi
  done

  # Build compiler flags array
  declare -a CC_FLAGS=(-c -O2)
  if [[ -n "$INCLUDE_DIRS" ]]; then
    # shellcheck disable=SC2206
    CC_FLAGS+=($INCLUDE_DIRS)
  fi

  STAGE2_MS=$(measure_ms 3 "$CC" "${CC_FLAGS[@]}" -o "$OBJ_FILE" "$C_FILE")
  # Compile once more to get actual file
  "$CC" "${CC_FLAGS[@]}" -o "$OBJ_FILE" "$C_FILE" 2>/dev/null || true
  if [[ -f "$OBJ_FILE" ]]; then
    OBJ_SIZE=$(file_size "$OBJ_FILE")
  fi
else
  echo "Stage 2: Skipped (no C compiler)" >&2
fi

# --- Stage 3: Full pipeline (JS -> native) ---
echo "Stage 3: Full pipeline (shermes -o)..." >&2
BIN_FILE="$TMPDIR_BASE/output"
STAGE3_MS=$(measure_ms 3 "$SHERMES" "${FLAGS[@]}" -o "$BIN_FILE" "$JS_FILE")

# Generate once more to get actual file
"$SHERMES" "${FLAGS[@]}" -o "$BIN_FILE" "$JS_FILE" 2>/dev/null
BIN_SIZE=$(file_size "$BIN_FILE")

# Infer link time = full pipeline - JS->C - C->obj (approximate)
LINK_MS=0
if [[ "$STAGE2_MS" -gt 0 ]]; then
  LINK_MS=$((STAGE3_MS - STAGE1_MS - STAGE2_MS))
  if [[ "$LINK_MS" -lt 0 ]]; then LINK_MS=0; fi
fi

# --- IR statistics (if available) ---
IR_STATS=""
IR_FILE="$TMPDIR_BASE/ir_dump.txt"
if "$SHERMES" --help 2>&1 | grep -q '\-dump-ir'; then
  echo "Collecting IR statistics..." >&2
  "$SHERMES" "${FLAGS[@]}" -dump-ir "$JS_FILE" > "$IR_FILE" 2>&1 || true
  if [[ -s "$IR_FILE" ]]; then
    IR_STATS=$(python3 -c "
import re
import json

with open('$IR_FILE') as f:
    content = f.read()

# Count IR constructs
functions = len(re.findall(r'^function ', content, re.MULTILINE))
basic_blocks = len(re.findall(r'^%BB\d+', content, re.MULTILINE))
instructions = len(re.findall(r'^\s+%\d+', content, re.MULTILINE))

stats = {
    'ir_functions': functions,
    'ir_basic_blocks': basic_blocks,
    'ir_instructions': instructions,
}
print(json.dumps(stats))
" 2>/dev/null || echo '{}')
  fi
fi

if [[ -z "$IR_STATS" ]]; then
  IR_STATS='{}'
fi

# Output
if $JSON_OUTPUT; then
  python3 -c "
import json

ir_stats = json.loads('$IR_STATS')

output = {
    'file': '$JS_FILE',
    'typed': $( $TYPED && echo 'True' || echo 'False' ),
    'input': {
        'js_bytes': $JS_SIZE,
        'js_lines': $JS_LINES
    },
    'stages': {
        'js_to_c': {
            'time_ms': $STAGE1_MS,
            'output_bytes': $C_SIZE,
            'output_lines': $C_LINES,
            'functions_generated': $FUNC_COUNT,
            'runtime_calls': $SH_CALL_COUNT
        },
        'c_to_object': {
            'time_ms': $STAGE2_MS,
            'output_bytes': $OBJ_SIZE,
            'compiler': '$CC' or None
        },
        'full_pipeline': {
            'time_ms': $STAGE3_MS,
            'output_bytes': $BIN_SIZE,
            'estimated_link_time_ms': $LINK_MS
        }
    },
    'size_ratios': {
        'c_to_js_ratio': round($C_SIZE / max($JS_SIZE, 1), 2),
        'bin_to_js_ratio': round($BIN_SIZE / max($JS_SIZE, 1), 2),
        'bin_to_c_ratio': round($BIN_SIZE / max($C_SIZE, 1), 4)
    },
    'ir_statistics': ir_stats
}
print(json.dumps(output))
"
else
  echo "Compilation Profile: $JS_FILE"
  echo "Shermes: $SHERMES"
  echo "Typed: $TYPED"
  echo ""
  echo "=== Input ==="
  printf "  JS file:     %d bytes, %d lines\n" "$JS_SIZE" "$JS_LINES"
  echo ""
  echo "=== Stage Timings ==="
  printf "  %-25s %8d ms -> %d bytes (%d lines, %d functions)\n" \
    "Stage 1 (JS -> C):" "$STAGE1_MS" "$C_SIZE" "$C_LINES" "$FUNC_COUNT"
  if [[ -n "$CC" ]]; then
    printf "  %-25s %8d ms -> %d bytes\n" \
      "Stage 2 (C -> object):" "$STAGE2_MS" "$OBJ_SIZE"
  else
    echo "  Stage 2 (C -> object):  Skipped (no CC)"
  fi
  printf "  %-25s %8d ms -> %d bytes\n" \
    "Stage 3 (full pipeline):" "$STAGE3_MS" "$BIN_SIZE"
  if [[ "$LINK_MS" -gt 0 ]]; then
    printf "  %-25s %8d ms (estimated)\n" "  Link time:" "$LINK_MS"
  fi
  echo ""
  echo "=== Size Ratios ==="
  printf "  C / JS:      %.2fx\n" "$(python3 -c "print($C_SIZE / max($JS_SIZE, 1))")"
  printf "  Binary / JS: %.2fx\n" "$(python3 -c "print($BIN_SIZE / max($JS_SIZE, 1))")"
  printf "  Binary / C:  %.4fx\n" "$(python3 -c "print($BIN_SIZE / max($C_SIZE, 1))")"
  echo ""
  echo "=== Generated C Metrics ==="
  printf "  Functions:     %d\n" "$FUNC_COUNT"
  printf "  Runtime calls: %d\n" "$SH_CALL_COUNT"

  # IR stats
  python3 -c "
import json
ir = json.loads('$IR_STATS')
if ir:
    print()
    print('=== IR Statistics ===')
    for k, v in sorted(ir.items()):
        print(f'  {k:25s} {v:>8d}')
" 2>/dev/null || true
fi
