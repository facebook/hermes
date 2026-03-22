#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# gc_stats.sh — GC statistics wrapper for Static Hermes.
# Compiles JS with shermes, runs the native binary, and captures
# GC statistics from stderr (-gc-print-stats JSON output).
# Computes derived metrics: allocation rate, GC overhead %, survival ratio.
#
# Usage:
#   gc_stats.sh <shermes_binary> <workload.js> [--typed] [--json]
#               [--compare <shermes_b>]

set -euo pipefail

# Defaults
SHERMES=""
JS_FILE=""
SHERMES_B=""
TYPED=false
JSON_OUTPUT=false
COMPARE_MODE=false

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> <workload.js> [options]

GC statistics wrapper for Static Hermes native binaries.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  --compare <shermes_b>  A/B comparison with second shermes binary
  -h, --help             Show this help

Captures GC statistics printed to stderr by Hermes runtime with
-gc-print-stats. Computes derived metrics including allocation rate,
GC overhead percentage, and survival ratio.

Examples:
  $(basename "$0") ./build/bin/shermes test.js --json
  $(basename "$0") ./build/bin/shermes bench.js --compare ./build2/bin/shermes --json
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --typed) TYPED=true; shift ;;
    --json) JSON_OUTPUT=true; shift ;;
    --compare) COMPARE_MODE=true; SHERMES_B="$2"; shift 2 ;;
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

# Compile JS to native
compile_to_native() {
  local shermes_bin="$1"
  local js_file="$2"
  local output="$3"
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  "$shermes_bin" "${flags[@]}" -o "$output" "$js_file" 2>/dev/null
}

# Run binary and capture GC stats from stderr
# The binary is expected to emit GC stats JSON to stderr when
# compiled with GC print stats enabled.
collect_gc_stats() {
  local binary="$1"
  local tmpfile
  tmpfile=$(mktemp)

  # Run the binary, capturing stderr for GC stats and measuring wall time
  local start end
  start=$(date +%s%N)
  "$binary" 2>"$tmpfile" >/dev/null || true
  end=$(date +%s%N)
  local wall_ns=$((end - start))
  local wall_ms=$((wall_ns / 1000000))

  # Parse GC stats from stderr
  # Hermes GC prints JSON like: {"totalTime":..., "collections":..., etc.}
  # Also look for line-based stats format
  python3 -c "
import json
import sys
import re

stderr_content = open('$tmpfile').read()

# Try to find JSON GC stats block
gc_data = {}
# Look for JSON object in stderr
json_match = re.search(r'\{[^{}]*\"(totalTime|gcTime|numCollections|heapSize)\"[^{}]*\}', stderr_content)
if json_match:
    try:
        gc_data = json.loads(json_match.group())
    except json.JSONDecodeError:
        pass

# Also try key:value format
if not gc_data:
    for line in stderr_content.strip().split('\n'):
        line = line.strip()
        # Match patterns like 'Total GC time: 123 ms'
        m = re.match(r'Total GC time:\s*([\d.]+)\s*ms', line)
        if m: gc_data['gc_time_ms'] = float(m.group(1))
        m = re.match(r'Number of collections:\s*(\d+)', line)
        if m: gc_data['num_collections'] = int(m.group(1))
        m = re.match(r'Total allocated:\s*([\d.]+)\s*(MB|KB|bytes)', line)
        if m:
            val = float(m.group(1))
            unit = m.group(2)
            if unit == 'MB': val *= 1024 * 1024
            elif unit == 'KB': val *= 1024
            gc_data['total_allocated_bytes'] = int(val)
        m = re.match(r'Total freed:\s*([\d.]+)\s*(MB|KB|bytes)', line)
        if m:
            val = float(m.group(1))
            unit = m.group(2)
            if unit == 'MB': val *= 1024 * 1024
            elif unit == 'KB': val *= 1024
            gc_data['total_freed_bytes'] = int(val)
        m = re.match(r'Heap size:\s*([\d.]+)\s*(MB|KB|bytes)', line)
        if m:
            val = float(m.group(1))
            unit = m.group(2)
            if unit == 'MB': val *= 1024 * 1024
            elif unit == 'KB': val *= 1024
            gc_data['heap_size_bytes'] = int(val)
        m = re.match(r'Peak heap:\s*([\d.]+)\s*(MB|KB|bytes)', line)
        if m:
            val = float(m.group(1))
            unit = m.group(2)
            if unit == 'MB': val *= 1024 * 1024
            elif unit == 'KB': val *= 1024
            gc_data['peak_heap_bytes'] = int(val)

wall_ms = $wall_ms

# Compute derived metrics
gc_time_ms = gc_data.get('gc_time_ms', gc_data.get('totalTime', 0))
num_collections = gc_data.get('num_collections', gc_data.get('numCollections', 0))
total_allocated = gc_data.get('total_allocated_bytes', gc_data.get('totalAllocatedBytes', 0))
total_freed = gc_data.get('total_freed_bytes', gc_data.get('totalFreedBytes', 0))
heap_size = gc_data.get('heap_size_bytes', gc_data.get('heapSize', 0))
peak_heap = gc_data.get('peak_heap_bytes', gc_data.get('peakHeapSize', 0))

# Derived
gc_overhead_pct = (gc_time_ms / wall_ms * 100) if wall_ms > 0 else 0
alloc_rate_mb_s = (total_allocated / (1024*1024)) / (wall_ms / 1000) if wall_ms > 0 and total_allocated > 0 else 0
survival_ratio = 1.0 - (total_freed / total_allocated) if total_allocated > 0 else 0

result = {
    'wall_time_ms': wall_ms,
    'gc_time_ms': gc_time_ms,
    'num_collections': num_collections,
    'total_allocated_bytes': total_allocated,
    'total_freed_bytes': total_freed,
    'heap_size_bytes': heap_size,
    'peak_heap_bytes': peak_heap,
    'gc_overhead_pct': round(gc_overhead_pct, 4),
    'allocation_rate_mb_per_sec': round(alloc_rate_mb_s, 4),
    'survival_ratio': round(survival_ratio, 6),
    'raw_gc_data': gc_data
}
print(json.dumps(result))
"
  rm -f "$tmpfile"
}

# Main
TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

# Compile and run A
native_a="$TMPDIR_BASE/native_a"
echo "Compiling: $JS_FILE with $SHERMES" >&2
compile_to_native "$SHERMES" "$JS_FILE" "$native_a"

echo "Running and collecting GC stats..." >&2
RESULT_A=$(collect_gc_stats "$native_a")

if $COMPARE_MODE; then
  if [[ ! -x "$SHERMES_B" ]]; then
    echo "Error: Comparison binary not found or not executable: $SHERMES_B" >&2
    exit 1
  fi

  native_b="$TMPDIR_BASE/native_b"
  echo "Compiling: $JS_FILE with $SHERMES_B" >&2
  compile_to_native "$SHERMES_B" "$JS_FILE" "$native_b"

  echo "Running and collecting GC stats (B)..." >&2
  RESULT_B=$(collect_gc_stats "$native_b")
fi

# Output
if $JSON_OUTPUT; then
  if $COMPARE_MODE; then
    python3 -c "
import json
a = json.loads('$RESULT_A')
b = json.loads('${RESULT_B//\'/\\\\\'}')

def safe_delta(va, vb):
    if va and va != 0:
        return round(((vb - va) / va) * 100, 4)
    return 0.0

deltas = {
    'wall_time_pct': safe_delta(a['wall_time_ms'], b['wall_time_ms']),
    'gc_time_pct': safe_delta(a['gc_time_ms'], b['gc_time_ms']),
    'gc_overhead_delta': round(b['gc_overhead_pct'] - a['gc_overhead_pct'], 4),
    'alloc_rate_pct': safe_delta(a['allocation_rate_mb_per_sec'], b['allocation_rate_mb_per_sec']),
    'survival_ratio_delta': round(b['survival_ratio'] - a['survival_ratio'], 6),
}

output = {
    'file': '$JS_FILE',
    'typed': $( $TYPED && echo 'True' || echo 'False' ),
    'mode': 'compare',
    'a': {'shermes': '$SHERMES', **a},
    'b': {'shermes': '$SHERMES_B', **b},
    'deltas': deltas
}
print(json.dumps(output))
"
  else
    python3 -c "
import json
a = json.loads('$RESULT_A')
output = {
    'file': '$JS_FILE',
    'typed': $( $TYPED && echo 'True' || echo 'False' ),
    'mode': 'single',
    'shermes': '$SHERMES',
    **a
}
print(json.dumps(output))
"
  fi
else
  echo "GC Statistics for: $JS_FILE"
  echo "Typed: $TYPED"
  echo ""

  python3 -c "
import json
a = json.loads('$RESULT_A')
print('=== Results ($SHERMES) ===')
print(f'  Wall time:           {a[\"wall_time_ms\"]} ms')
print(f'  GC time:             {a[\"gc_time_ms\"]} ms')
print(f'  GC overhead:         {a[\"gc_overhead_pct\"]:.2f}%')
print(f'  Collections:         {a[\"num_collections\"]}')
print(f'  Total allocated:     {a[\"total_allocated_bytes\"]} bytes')
print(f'  Total freed:         {a[\"total_freed_bytes\"]} bytes')
print(f'  Heap size:           {a[\"heap_size_bytes\"]} bytes')
print(f'  Peak heap:           {a[\"peak_heap_bytes\"]} bytes')
print(f'  Allocation rate:     {a[\"allocation_rate_mb_per_sec\"]:.2f} MB/s')
print(f'  Survival ratio:      {a[\"survival_ratio\"]:.6f}')
"

  if $COMPARE_MODE; then
    python3 -c "
import json
b = json.loads('${RESULT_B//\'/\\\\\'}')
print()
print('=== Results ($SHERMES_B) ===')
print(f'  Wall time:           {b[\"wall_time_ms\"]} ms')
print(f'  GC time:             {b[\"gc_time_ms\"]} ms')
print(f'  GC overhead:         {b[\"gc_overhead_pct\"]:.2f}%')
print(f'  Collections:         {b[\"num_collections\"]}')
print(f'  Total allocated:     {b[\"total_allocated_bytes\"]} bytes')
print(f'  Allocation rate:     {b[\"allocation_rate_mb_per_sec\"]:.2f} MB/s')
print(f'  Survival ratio:      {b[\"survival_ratio\"]:.6f}')
"
  fi
fi
