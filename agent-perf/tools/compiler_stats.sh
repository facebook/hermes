#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# compiler_stats.sh — Compiler pass statistics for Static Hermes.
# Runs shermes with --print-stats to capture LLVM STATISTIC() counters
# and pass timing information, then parses into structured output.
#
# Usage:
#   compiler_stats.sh <shermes_binary> <workload.js> [--typed] [--json]

set -euo pipefail

# Defaults
SHERMES=""
JS_FILE=""
TYPED=false
JSON_OUTPUT=false

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> <workload.js> [options]

Compiler pass statistics for Static Hermes.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  -h, --help             Show this help

Runs shermes with --print-stats (if available) and captures:
  - LLVM STATISTIC() counters (inlining, DCE, CSE, type inference, etc.)
  - Pass timing information
  - Key optimization counters

The output is parsed into a structured format with per-pass breakdown.

Examples:
  $(basename "$0") ./build/bin/shermes test.js --json
  $(basename "$0") ./build/bin/shermes bench.js --typed
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
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  printf '%s\n' "${flags[@]}"
}

# Main
TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

# Build flags array
FLAGS=()
if $TYPED; then
  FLAGS+=("--typed")
fi

STATS_FILE="$TMPDIR_BASE/stats_output.txt"
OUTPUT_C="$TMPDIR_BASE/output.c"

# Try --print-stats first, fall back to -stats
echo "Running shermes with statistics collection..." >&2
STATS_FLAG=""
if "$SHERMES" --help 2>&1 | grep -q '\-\-print-stats'; then
  STATS_FLAG="--print-stats"
elif "$SHERMES" --help 2>&1 | grep -q '\-stats'; then
  STATS_FLAG="-stats"
fi

# Also try -time-passes for pass timing
TIME_FLAG=""
if "$SHERMES" --help 2>&1 | grep -q '\-time-passes'; then
  TIME_FLAG="-time-passes"
fi

# Run compilation capturing all output
{
  "$SHERMES" "${FLAGS[@]}" $STATS_FLAG $TIME_FLAG -emit-c -o "$OUTPUT_C" "$JS_FILE" 2>&1 || true
} > "$STATS_FILE" 2>&1

# Also measure compilation time
COMPILE_START=$(date +%s%N)
"$SHERMES" "${FLAGS[@]}" -emit-c -o /dev/null "$JS_FILE" 2>/dev/null || true
COMPILE_END=$(date +%s%N)
COMPILE_MS=$(( (COMPILE_END - COMPILE_START) / 1000000 ))

# Parse statistics output
python3 -c "
import json
import re
import sys

stats_text = open('$STATS_FILE').read()
compile_ms = $COMPILE_MS

# Parse LLVM-style STATISTIC counters
# Format: '  NNN counter-name   - Description'
statistics = {}
for line in stats_text.split('\n'):
    # Match lines like: '    42 dce      - Number of instructions removed by DCE'
    m = re.match(r'\s+(\d+)\s+([\w.-]+)\s+-\s+(.+)', line)
    if m:
        count = int(m.group(1))
        name = m.group(2).strip()
        desc = m.group(3).strip()
        statistics[name] = {'count': count, 'description': desc}

# Parse pass timing
# Format: '  ---Pass Name---   Total  Wall Time  ...'
pass_timings = {}
timing_section = False
for line in stats_text.split('\n'):
    if 'Pass execution timing' in line or '---Pass Name---' in line:
        timing_section = True
        continue
    if timing_section:
        # Match lines like: '  0.0123 ( 12.3%)   0.0123 ( 12.3%)   PassName'
        m = re.match(r'\s+([\d.]+)\s+\(\s*[\d.]+%\)\s+([\d.]+)\s+\(\s*[\d.]+%\)\s+(.+)', line)
        if m:
            wall_time = float(m.group(2))
            pass_name = m.group(3).strip()
            pass_timings[pass_name] = {'wall_time_sec': wall_time}
        elif line.strip() == '' or line.strip().startswith('==='):
            timing_section = False

# Key counters we care about
key_counters = {
    'inlining': 0,
    'dce': 0,
    'cse': 0,
    'type_inference': 0,
    'simplifycfg': 0,
    'mem2reg': 0,
    'instsimplify': 0,
    'stackpromotion': 0,
    'codemotion': 0,
    'funcsigopts': 0,
}

for name, data in statistics.items():
    lower = name.lower()
    if 'inline' in lower:
        key_counters['inlining'] += data['count']
    elif 'dce' in lower or 'dead' in lower:
        key_counters['dce'] += data['count']
    elif 'cse' in lower:
        key_counters['cse'] += data['count']
    elif 'type' in lower and ('infer' in lower or 'narrow' in lower):
        key_counters['type_inference'] += data['count']
    elif 'simplify' in lower and 'cfg' in lower:
        key_counters['simplifycfg'] += data['count']
    elif 'mem2reg' in lower:
        key_counters['mem2reg'] += data['count']
    elif 'inst' in lower and 'simplify' in lower:
        key_counters['instsimplify'] += data['count']
    elif 'stack' in lower and 'promot' in lower:
        key_counters['stackpromotion'] += data['count']
    elif 'motion' in lower or 'hoist' in lower or 'sink' in lower:
        key_counters['codemotion'] += data['count']
    elif 'sig' in lower and 'opt' in lower:
        key_counters['funcsigopts'] += data['count']

json_mode = $( $JSON_OUTPUT && echo 'True' || echo 'False' )

if json_mode:
    output = {
        'file': '$JS_FILE',
        'typed': $( $TYPED && echo 'True' || echo 'False' ),
        'compile_time_ms': compile_ms,
        'stats_flag': '$STATS_FLAG' or None,
        'key_counters': key_counters,
        'all_statistics': {k: v['count'] for k, v in statistics.items()},
        'statistic_details': statistics,
        'pass_timings': pass_timings,
        'total_statistics_count': len(statistics),
    }
    print(json.dumps(output))
else:
    print(f'Compiler Statistics for: $JS_FILE')
    print(f'Typed: $TYPED')
    print(f'Compile time: {compile_ms} ms')
    print()

    if statistics:
        print('=== Key Optimization Counters ===')
        for name, count in sorted(key_counters.items()):
            if count > 0:
                print(f'  {name:25s} {count:>8d}')
        print()

        print('=== All STATISTIC Counters ===')
        for name, data in sorted(statistics.items(), key=lambda x: -x[1]['count']):
            print(f'  {data[\"count\"]:>8d}  {name:30s}  {data[\"description\"]}')
    else:
        print('No STATISTIC counters found in output.')
        print('(shermes may need to be built with LLVM statistics enabled)')
        print()
        if stats_text.strip():
            print('=== Raw Output ===')
            # Show first 50 lines
            for line in stats_text.strip().split('\n')[:50]:
                print(f'  {line}')

    if pass_timings:
        print()
        print('=== Pass Timings ===')
        for name, data in sorted(pass_timings.items(), key=lambda x: -x[1]['wall_time_sec']):
            print(f'  {data[\"wall_time_sec\"]:8.4f}s  {name}')
"
