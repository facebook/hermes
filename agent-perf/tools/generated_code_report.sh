#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# generated_code_report.sh — Generated C code analysis for Static Hermes.
# Generates C code with shermes -emit-c and reports per-function metrics:
# lines of C, runtime call count, and complexity indicators.
#
# Usage:
#   generated_code_report.sh <shermes_binary> <workload.js> [--typed] [--json]
#                            [--compare <shermes_b>]

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

Analyze generated C code from Static Hermes.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  --compare <shermes_b>  Compare generated code from two shermes binaries
  -h, --help             Show this help

Reports per-function:
  - Lines of C code
  - Runtime (_sh_*) call count
  - Complexity estimate (branch count)

Reports totals:
  - Function count, total lines
  - Call breakdown by category (property access, type checks, GC, etc.)

Examples:
  $(basename "$0") ./build/bin/shermes test.js --json
  $(basename "$0") ./build/bin/shermes bench.js --compare ./build2/bin/shermes
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
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  printf '%s\n' "${flags[@]+"${flags[@]}"}"
}

# Generate C code
generate_c() {
  local shermes_bin="$1"
  local js_file="$2"
  local output_c="$3"
  local -a flags=()
  if $TYPED; then
    flags+=("--typed")
  fi
  "$shermes_bin" "${flags[@]+"${flags[@]}"}" -emit-c -o "$output_c" "$js_file" 2>/dev/null
}

# Analyze a C file and produce JSON report
analyze_c_file() {
  local c_file="$1"
  local label="$2"
  local shermes_bin="$3"

  python3 -c "
import json
import re
import sys

with open('$c_file', 'r') as f:
    content = f.read()
    lines = content.split('\n')

total_lines = len(lines)

# Find function boundaries
# Functions typically start with: 'static SHLegacyValue ...' or 'static SHLegacyResult ...'
func_pattern = re.compile(r'^(static\s+(?:SHLegacyValue|SHLegacyResult)\s+\w+)\s*\(')
functions = []
func_starts = []

for i, line in enumerate(lines):
    m = func_pattern.match(line)
    if m:
        func_starts.append((i, m.group(1)))

# Determine function boundaries
for idx, (start, sig) in enumerate(func_starts):
    # Find end by matching closing brace
    end = start
    brace_depth = 0
    found_open = False
    for j in range(start, len(lines)):
        for ch in lines[j]:
            if ch == '{':
                brace_depth += 1
                found_open = True
            elif ch == '}':
                brace_depth -= 1
        if found_open and brace_depth == 0:
            end = j
            break

    func_lines = lines[start:end+1]
    func_text = '\n'.join(func_lines)
    num_lines = len(func_lines)

    # Count runtime calls
    sh_calls = len(re.findall(r'_sh_\w+', func_text))

    # Count branches (if/else/switch/case/goto)
    branches = len(re.findall(r'\b(if|else|switch|case|goto)\b', func_text))

    # Extract function name
    name_m = re.match(r'static\s+\w+\s+(\w+)', sig)
    func_name = name_m.group(1) if name_m else sig

    functions.append({
        'name': func_name,
        'lines': num_lines,
        'runtime_calls': sh_calls,
        'branches': branches,
    })

# Total runtime call categorization
prop_access = len(re.findall(r'_sh_ljs_(get|put|del|has)_by', content))
type_checks = len(re.findall(r'_sh_ljs_is_', content))
gc_calls = len(re.findall(r'_sh_(enter|leave)', content))
arith_calls = len(re.findall(r'_sh_ljs_(add|sub|mul|div|mod|inc|dec|neg|bit)', content))
obj_calls = len(re.findall(r'_sh_ljs_(create|new)_', content))
str_calls = len(re.findall(r'_sh_ljs_string_', content))
total_sh_calls = len(re.findall(r'_sh_\w+', content))
other_calls = max(0, total_sh_calls - prop_access - type_checks - gc_calls - arith_calls - obj_calls - str_calls)

report = {
    'label': '$label',
    'shermes': '$shermes_bin',
    'file': '$JS_FILE',
    'total_lines': total_lines,
    'function_count': len(functions),
    'functions': sorted(functions, key=lambda f: -f['lines']),
    'call_breakdown': {
        'total': total_sh_calls,
        'property_access': prop_access,
        'type_checks': type_checks,
        'gc_management': gc_calls,
        'arithmetic': arith_calls,
        'object_creation': obj_calls,
        'string_ops': str_calls,
        'other': other_calls,
    }
}
print(json.dumps(report))
"
}

# Main
TMPDIR_BASE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_BASE"' EXIT

# Generate and analyze A
C_FILE_A="$TMPDIR_BASE/output_a.c"
echo "Generating C code with: $SHERMES" >&2
generate_c "$SHERMES" "$JS_FILE" "$C_FILE_A"

echo "Analyzing generated C..." >&2
REPORT_A=$(analyze_c_file "$C_FILE_A" "A" "$SHERMES")

REPORT_B=""
if $COMPARE_MODE; then
  if [[ ! -x "$SHERMES_B" ]]; then
    echo "Error: Comparison binary not found or not executable: $SHERMES_B" >&2
    exit 1
  fi

  C_FILE_B="$TMPDIR_BASE/output_b.c"
  echo "Generating C code with: $SHERMES_B" >&2
  generate_c "$SHERMES_B" "$JS_FILE" "$C_FILE_B"

  echo "Analyzing generated C (B)..." >&2
  REPORT_B=$(analyze_c_file "$C_FILE_B" "B" "$SHERMES_B")
fi

# Output
if $JSON_OUTPUT; then
  if $COMPARE_MODE; then
    python3 -c "
import json
a = json.loads('''$REPORT_A''')
b = json.loads('''$REPORT_B''')

def safe_delta(va, vb):
    if va and va != 0:
        return round(((vb - va) / va) * 100, 2)
    return 0.0

deltas = {
    'total_lines_pct': safe_delta(a['total_lines'], b['total_lines']),
    'function_count_delta': b['function_count'] - a['function_count'],
    'total_calls_pct': safe_delta(a['call_breakdown']['total'], b['call_breakdown']['total']),
}

output = {
    'file': '$JS_FILE',
    'typed': $( $TYPED && echo 'True' || echo 'False' ),
    'mode': 'compare',
    'a': a,
    'b': b,
    'deltas': deltas
}
print(json.dumps(output))
"
  else
    python3 -c "
import json
a = json.loads('''$REPORT_A''')
a['typed'] = $( $TYPED && echo 'True' || echo 'False' )
a['mode'] = 'single'
print(json.dumps(a))
"
  fi
else
  python3 -c "
import json

a = json.loads('''$REPORT_A''')

print(f'Generated Code Report: $JS_FILE')
print(f'Shermes: $SHERMES')
print(f'Typed: $TYPED')
print()
print(f'=== Summary ===')
print(f'  Total lines:    {a[\"total_lines\"]}')
print(f'  Function count: {a[\"function_count\"]}')
print()
print(f'=== Per-Function Breakdown ===')
print(f'{\"Function\":40s} {\"Lines\":>8s} {\"SH Calls\":>10s} {\"Branches\":>10s}')
print(f'{\"--------\":40s} {\"-----\":>8s} {\"--------\":>10s} {\"--------\":>10s}')
for func in a['functions'][:30]:
    print(f'{func[\"name\"]:40s} {func[\"lines\"]:>8d} {func[\"runtime_calls\"]:>10d} {func[\"branches\"]:>10d}')

if len(a['functions']) > 30:
    print(f'  ... and {len(a[\"functions\"]) - 30} more functions')

cb = a['call_breakdown']
print()
print(f'=== Runtime Call Breakdown ===')
print(f'  Total:           {cb[\"total\"]:>8d}')
print(f'  Property access: {cb[\"property_access\"]:>8d}')
print(f'  Type checks:     {cb[\"type_checks\"]:>8d}')
print(f'  GC management:   {cb[\"gc_management\"]:>8d}')
print(f'  Arithmetic:      {cb[\"arithmetic\"]:>8d}')
print(f'  Object creation: {cb[\"object_creation\"]:>8d}')
print(f'  String ops:      {cb[\"string_ops\"]:>8d}')
print(f'  Other:           {cb[\"other\"]:>8d}')
"

  if $COMPARE_MODE; then
    python3 -c "
import json

a = json.loads('''$REPORT_A''')
b = json.loads('''$REPORT_B''')

def safe_delta(va, vb):
    if va and va != 0:
        return ((vb - va) / va) * 100
    return 0.0

print()
print(f'=== Comparison vs $SHERMES_B ===')
print(f'  Lines delta:     {safe_delta(a[\"total_lines\"], b[\"total_lines\"]):+.2f}%')
print(f'  Function delta:  {b[\"function_count\"] - a[\"function_count\"]:+d}')
print(f'  Calls delta:     {safe_delta(a[\"call_breakdown\"][\"total\"], b[\"call_breakdown\"][\"total\"]):+.2f}%')
"
  fi
fi
