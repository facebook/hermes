#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# perf_proxies.sh — Quick multi-dimensional performance health check
# for Static Hermes. Measures generated C code size, native binary size,
# runtime call density, compilation latency, and function metrics.
#
# Usage:
#   perf_proxies.sh <shermes_binary> <js_file_or_dir> [--typed] [--json]
#   perf_proxies.sh <shermes_binary> <js_file_or_dir> --compare <shermes_b>

set -euo pipefail

SHERMES=""
JS_INPUT=""
SHERMES_B=""
TYPED=false
JSON_OUTPUT=false
COMPARE_MODE=false

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes> <js_file_or_dir> [options]

Quick performance proxies for Static Hermes.

Metrics measured:
  1. Generated C code size (lines and bytes)
  2. Native binary size (bytes)
  3. Runtime call density (count of _sh_* calls in generated C)
  4. Compilation latency (ms for each pipeline stage)
  5. Function count and average size

Options:
  --typed                Compile with --typed flag
  --compare <shermes_b>  A/B comparison mode
  --json                 Output structured JSON
  -h, --help             Show this help
EOF
  exit 0
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --typed) TYPED=true; shift ;;
    --compare) COMPARE_MODE=true; SHERMES_B="$2"; shift 2 ;;
    --json) JSON_OUTPUT=true; shift ;;
    -h|--help) usage ;;
    -*) echo "Unknown option: $1" >&2; exit 1 ;;
    *)
      if [[ -z "$SHERMES" ]]; then
        SHERMES="$1"
      elif [[ -z "$JS_INPUT" ]]; then
        JS_INPUT="$1"
      fi
      shift
      ;;
  esac
done

if [[ -z "$SHERMES" ]] || [[ -z "$JS_INPUT" ]]; then
  echo "Error: shermes binary and JS file/directory required" >&2
  usage
fi

# Collect JS files
collect_js_files() {
  local input="$1"
  if [[ -d "$input" ]]; then
    find "$input" -name '*.js' -type f | sort
  else
    echo "$input"
  fi
}

# Measure proxies for a single JS file
measure_proxies() {
  local shermes_bin="$1"
  local js_file="$2"
  local tmpdir
  tmpdir=$(mktemp -d)

  local c_file="$tmpdir/output.c"
  local native_binary="$tmpdir/output"
  local -a flags=()
  if $TYPED; then flags+=("--typed"); fi

  # Stage 1: JS -> C
  local start_c end_c
  start_c=$(date +%s%N)
  "$shermes_bin" -emit-c "${flags[@]}" -o "$c_file" "$js_file" 2>/dev/null
  end_c=$(date +%s%N)
  local c_latency_ms=$(( (end_c - start_c) / 1000000 ))

  # C code metrics
  local c_lines c_bytes
  c_lines=$(wc -l < "$c_file")
  c_bytes=$(stat -c%s "$c_file" 2>/dev/null || stat -f%z "$c_file" 2>/dev/null)

  # Runtime call density
  local sh_call_count
  sh_call_count=$(grep -c '_sh_' "$c_file" 2>/dev/null || echo 0)

  # Function count (static SHLegacyValue _N_ pattern)
  local func_count
  func_count=$(grep -c '^static SHLegacyValue\|^static SHLegacyResult' "$c_file" 2>/dev/null || echo 0)
  if [[ "$func_count" -eq 0 ]]; then func_count=1; fi
  local avg_lines_per_func=$((c_lines / func_count))

  # Runtime call categorization
  local prop_access_calls type_check_calls gc_calls arith_calls obj_calls str_calls other_calls
  prop_access_calls=$(grep -cE '_sh_ljs_(get|put|del|has)_by' "$c_file" 2>/dev/null || echo 0)
  type_check_calls=$(grep -cE '_sh_ljs_is_' "$c_file" 2>/dev/null || echo 0)
  gc_calls=$(grep -cE '_sh_(enter|leave)' "$c_file" 2>/dev/null || echo 0)
  arith_calls=$(grep -cE '_sh_ljs_(add|sub|mul|div|mod|inc|dec|neg|bit)' "$c_file" 2>/dev/null || echo 0)
  obj_calls=$(grep -cE '_sh_ljs_(create|new)_' "$c_file" 2>/dev/null || echo 0)
  str_calls=$(grep -cE '_sh_ljs_string_' "$c_file" 2>/dev/null || echo 0)
  other_calls=$((sh_call_count - prop_access_calls - type_check_calls - gc_calls - arith_calls - obj_calls - str_calls))
  if [[ "$other_calls" -lt 0 ]]; then other_calls=0; fi

  # Stage 2: C -> native binary (full compilation)
  local start_native end_native
  start_native=$(date +%s%N)
  "$shermes_bin" "${flags[@]}" -o "$native_binary" "$js_file" 2>/dev/null
  end_native=$(date +%s%N)
  local total_latency_ms=$(( (end_native - start_native) / 1000000 ))
  local cc_latency_ms=$((total_latency_ms - c_latency_ms))
  if [[ "$cc_latency_ms" -lt 0 ]]; then cc_latency_ms=0; fi

  # Native binary size
  local native_bytes
  native_bytes=$(stat -c%s "$native_binary" 2>/dev/null || stat -f%z "$native_binary" 2>/dev/null)

  rm -rf "$tmpdir"

  # Output
  echo "$js_file $c_lines $c_bytes $native_bytes $sh_call_count $func_count $avg_lines_per_func $c_latency_ms $cc_latency_ms $total_latency_ms $prop_access_calls $type_check_calls $gc_calls $arith_calls $obj_calls $str_calls $other_calls"
}

# Main
mapfile -t JS_FILES < <(collect_js_files "$JS_INPUT")

if [[ ${#JS_FILES[@]} -eq 0 ]]; then
  echo "Error: No JS files found in: $JS_INPUT" >&2
  exit 1
fi

# Collect data
declare -a RESULTS=()
for js_file in "${JS_FILES[@]}"; do
  echo "Measuring: $js_file" >&2
  result=$(measure_proxies "$SHERMES" "$js_file")
  RESULTS+=("$result")
done

# A/B comparison data
declare -a RESULTS_B=()
if $COMPARE_MODE; then
  for js_file in "${JS_FILES[@]}"; do
    echo "Measuring (B): $js_file" >&2
    result=$(measure_proxies "$SHERMES_B" "$js_file")
    RESULTS_B+=("$result")
  done
fi

# Output results
if $JSON_OUTPUT; then
  JSON_TMPFILE=$(mktemp)
  echo "{" >> "$JSON_TMPFILE"
  echo "  \"shermes\": \"$SHERMES\"," >> "$JSON_TMPFILE"
  echo "  \"typed\": $TYPED," >> "$JSON_TMPFILE"
  echo "  \"files\": [" >> "$JSON_TMPFILE"
  for i in "${!RESULTS[@]}"; do
    read -r file c_lines c_bytes native_bytes sh_calls func_count avg_lines c_lat cc_lat total_lat prop_calls type_calls gc_calls arith_calls obj_calls str_calls other_calls <<< "${RESULTS[$i]}"
    [[ $i -gt 0 ]] && echo "    ," >> "$JSON_TMPFILE"
    cat <<ENDJSON >> "$JSON_TMPFILE"
    {
      "file": "$file",
      "generated_c": {
        "lines": $c_lines,
        "bytes": $c_bytes,
        "function_count": $func_count,
        "avg_lines_per_function": $avg_lines
      },
      "native_binary_bytes": $native_bytes,
      "runtime_calls": {
        "total": $sh_calls,
        "property_access": $prop_calls,
        "type_checks": $type_calls,
        "gc_management": $gc_calls,
        "arithmetic": $arith_calls,
        "object_creation": $obj_calls,
        "string_ops": $str_calls,
        "other": $other_calls
      },
      "latency_ms": {
        "js_to_c": $c_lat,
        "c_to_native": $cc_lat,
        "total": $total_lat
      }
    }
ENDJSON
  done
  echo "  ]" >> "$JSON_TMPFILE"

  if $COMPARE_MODE; then
    echo "  ,\"comparison\": {" >> "$JSON_TMPFILE"
    echo "    \"shermes_b\": \"$SHERMES_B\"," >> "$JSON_TMPFILE"
    echo "    \"files\": [" >> "$JSON_TMPFILE"
    for i in "${!RESULTS_B[@]}"; do
      read -r file c_lines c_bytes native_bytes sh_calls func_count avg_lines c_lat cc_lat total_lat prop_calls type_calls gc_calls arith_calls obj_calls str_calls other_calls <<< "${RESULTS_B[$i]}"
      read -r _ c_lines_a _ native_bytes_a sh_calls_a _ _ _ _ _ _ _ _ _ _ _ _ <<< "${RESULTS[$i]}"
      c_delta=$(python3 -c "print(f'{(($c_lines - $c_lines_a) / max($c_lines_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      native_delta=$(python3 -c "print(f'{(($native_bytes - $native_bytes_a) / max($native_bytes_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      calls_delta=$(python3 -c "print(f'{(($sh_calls - $sh_calls_a) / max($sh_calls_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      [[ $i -gt 0 ]] && echo "      ," >> "$JSON_TMPFILE"
      echo "      {\"file\": \"$file\", \"c_lines_delta_pct\": $c_delta, \"native_size_delta_pct\": $native_delta, \"runtime_calls_delta_pct\": $calls_delta}" >> "$JSON_TMPFILE"
    done
    echo "    ]" >> "$JSON_TMPFILE"
    echo "  }" >> "$JSON_TMPFILE"
  fi

  echo "}" >> "$JSON_TMPFILE"
  python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" < "$JSON_TMPFILE"
  rm -f "$JSON_TMPFILE"
else
  printf "%-40s %8s %10s %10s %8s %5s %6s %6s %6s %6s\n" \
    "File" "C Lines" "C Bytes" "Bin Bytes" "SH Calls" "Funcs" "Avg/F" "C ms" "CC ms" "Tot ms"
  printf "%-40s %8s %10s %10s %8s %5s %6s %6s %6s %6s\n" \
    "----" "-------" "-------" "---------" "--------" "-----" "-----" "----" "-----" "------"
  for result in "${RESULTS[@]}"; do
    read -r file c_lines c_bytes native_bytes sh_calls func_count avg_lines c_lat cc_lat total_lat _ <<< "$result"
    basename=$(basename "$file")
    printf "%-40s %8d %10d %10d %8d %5d %6d %6d %6d %6d\n" \
      "$basename" "$c_lines" "$c_bytes" "$native_bytes" "$sh_calls" "$func_count" "$avg_lines" "$c_lat" "$cc_lat" "$total_lat"
  done

  if $COMPARE_MODE; then
    echo ""
    echo "=== Comparison Deltas ==="
    printf "%-40s %10s %12s %12s\n" "File" "C Lines %" "Bin Size %" "SH Calls %"
    for i in "${!RESULTS_B[@]}"; do
      read -r file c_lines _ native_bytes sh_calls _ _ _ _ _ _ <<< "${RESULTS_B[$i]}"
      read -r _ c_lines_a _ native_bytes_a sh_calls_a _ _ _ _ _ _ <<< "${RESULTS[$i]}"
      basename=$(basename "$file")
      c_delta=$(python3 -c "print(f'{(($c_lines - $c_lines_a) / max($c_lines_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      native_delta=$(python3 -c "print(f'{(($native_bytes - $native_bytes_a) / max($native_bytes_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      calls_delta=$(python3 -c "print(f'{(($sh_calls - $sh_calls_a) / max($sh_calls_a, 1)) * 100:.2f}')" 2>/dev/null || echo "0.00")
      printf "%-40s %10s %12s %12s\n" "$basename" "${c_delta}%" "${native_delta}%" "${calls_delta}%"
    done
  fi
fi
