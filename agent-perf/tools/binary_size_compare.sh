#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# binary_size_compare.sh — A/B binary size comparison for Static Hermes.
#
# Compiles JS benchmarks with two shermes builds and compares the resulting
# binary sizes to quantify code size impact of compiler/header changes
# (e.g., inlined fast paths).
#
# Reports: total binary size, .text section size, get_by_val call site count,
# and per-call-site overhead. Optionally uses bloaty for deeper symbol-level
# analysis when available.
#
# Usage:
#   binary_size_compare.sh --baseline <shermes_a> --optimized <shermes_b> \
#                          [--js <file.js>] [--suite micro|macro|octane|all]
#                          [--typed] [--json] [--strip] [--bloaty]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/../benchmarks" 2>/dev/null && pwd)" || BENCH_DIR=""
SH_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
OCTANE_DIR="$SH_ROOT/benchmarks/octane"

# Defaults
SHERMES_A=""
SHERMES_B=""
JS_FILES=()
SUITE=""
TYPED=false
JSON_OUTPUT=false
STRIP_BINARIES=false
USE_BLOATY=false

usage() {
  cat <<EOF
Usage: $(basename "$0") --baseline <shermes_a> --optimized <shermes_b> [options]

A/B binary size comparison for Static Hermes compiled JS programs.

Options:
  --baseline <path>    Path to baseline shermes binary (A)
  --optimized <path>   Path to optimized shermes binary (B)
  --js <file.js>       JS file to compile (can specify multiple times)
  --suite <name>       Benchmark suite: micro, macro, octane, all
  --typed              Compile with --typed flag
  --strip              Compare stripped binaries (removes debug info)
  --bloaty             Use bloaty for deeper symbol/section analysis (if installed)
  --json               Output structured JSON
  -h, --help           Show this help

Output for each JS file:
  - Total binary size (bytes and delta %)
  - .text section size (code only, excludes data/debug)
  - Number of _sh_ljs_get_by_val call sites in generated code
  - Per-call-site overhead (text delta / call site count)
  - Symbol-level size comparison for key functions

Examples:
  $(basename "$0") --baseline ./build-base/bin/shermes \\
                   --optimized ./build-opt/bin/shermes \\
                   --js test.js

  $(basename "$0") --baseline \$SHERMES_A --optimized \$SHERMES_B \\
                   --suite octane --json --strip

  # With bloaty for deeper analysis (DWARF, compilation units)
  $(basename "$0") --baseline \$SHERMES_A --optimized \$SHERMES_B \\
                   --suite octane --bloaty
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --baseline) SHERMES_A="$2"; shift 2 ;;
    --optimized) SHERMES_B="$2"; shift 2 ;;
    --js) JS_FILES+=("$2"); shift 2 ;;
    --suite) SUITE="$2"; shift 2 ;;
    --typed) TYPED=true; shift ;;
    --strip) STRIP_BINARIES=true; shift ;;
    --bloaty) USE_BLOATY=true; shift ;;
    --json) JSON_OUTPUT=true; shift ;;
    -h|--help) usage ;;
    -*) echo "Error: Unknown option: $1" >&2; exit 1 ;;
    *) JS_FILES+=("$1"); shift ;;
  esac
done

if [[ -z "$SHERMES_A" || -z "$SHERMES_B" ]]; then
  echo "Error: Both --baseline and --optimized shermes paths required" >&2
  usage
fi

for bin in "$SHERMES_A" "$SHERMES_B"; do
  if [[ ! -x "$bin" ]]; then
    echo "Error: shermes binary not found or not executable: $bin" >&2
    exit 1
  fi
done

# Validate bloaty availability
if $USE_BLOATY; then
  if ! command -v bloaty &>/dev/null; then
    echo "Warning: bloaty not found, falling back to nm/objdump analysis" >&2
    USE_BLOATY=false
  fi
fi

# Collect JS files from suite if specified
if [[ -n "$SUITE" ]]; then
  if [[ "$SUITE" == "micro" || "$SUITE" == "all" ]] && [[ -d "$BENCH_DIR/micro" ]]; then
    while IFS= read -r f; do JS_FILES+=("$f"); done < <(find "$BENCH_DIR/micro" -name '*.js' -type f | sort)
  fi
  if [[ "$SUITE" == "macro" || "$SUITE" == "all" ]] && [[ -d "$BENCH_DIR/macro" ]]; then
    while IFS= read -r f; do JS_FILES+=("$f"); done < <(find "$BENCH_DIR/macro" -name '*.js' -type f | sort)
  fi
  if [[ "$SUITE" == "octane" || "$SUITE" == "all" ]] && [[ -d "$OCTANE_DIR" ]]; then
    while IFS= read -r f; do JS_FILES+=("$f"); done < <(find "$OCTANE_DIR" -name '*.js' -type f | sort)
  fi
fi

if [[ ${#JS_FILES[@]} -eq 0 ]]; then
  echo "Error: No JS files specified. Use --js <file> or --suite <name>" >&2
  exit 1
fi

# Create temp directory
TMPDIR_SIZE=$(mktemp -d)
trap 'rm -rf "$TMPDIR_SIZE"' EXIT

# Compile a JS file to a native binary
compile_js() {
  local shermes_bin="$1"
  local js_file="$2"
  local output="$3"
  local -a flags=()
  if $TYPED; then flags+=("--typed"); fi
  "$shermes_bin" "${flags[@]}" -o "$output" "$js_file" 2>/dev/null
  if $STRIP_BINARIES && command -v strip &>/dev/null; then
    strip "$output"
  fi
}

# Get total file size in bytes
file_size() {
  stat -c%s "$1" 2>/dev/null || stat -f%z "$1" 2>/dev/null
}

# Get .text section size using objdump or size
text_section_size() {
  local binary="$1"
  if command -v objdump &>/dev/null; then
    objdump -h "$binary" 2>/dev/null | awk '/.text/{print strtonum("0x"$3)}'
  elif command -v size &>/dev/null; then
    size "$binary" 2>/dev/null | tail -1 | awk '{print $1}'
  else
    echo "0"
  fi
}

# Count get_by_val call sites in the generated C code
count_get_by_val_calls() {
  local shermes_bin="$1"
  local js_file="$2"
  local c_file="$TMPDIR_SIZE/temp_emit.c"
  local -a flags=()
  if $TYPED; then flags+=("--typed"); fi
  "$shermes_bin" "${flags[@]}" -emit-c -o "$c_file" "$js_file" 2>/dev/null || true
  if [[ -f "$c_file" ]]; then
    grep -c '_sh_ljs_get_by_val_rjs\b' "$c_file" 2>/dev/null || echo "0"
  else
    echo "0"
  fi
}

# Get sizes of key symbols using nm
key_symbol_sizes() {
  local binary="$1"
  if ! command -v nm &>/dev/null; then
    echo "{}"
    return
  fi
  # Extract text symbols with sizes, filter for key functions
  nm --print-size --size-sort --radix=d "$binary" 2>/dev/null | \
    grep -E '(get_by_val|get_by_id|_sh_ljs_call|_sh_ljs_add|_sh_ljs_construct|_main|_0_global)' | \
    awk '{print $2, $4}' | head -20 || true
}

# Run bloaty A/B comparison on two binaries
# Outputs section-level and symbol-level diffs
bloaty_compare() {
  local binary_a="$1"
  local binary_b="$2"
  local bench_name="$3"

  if $JSON_OUTPUT; then
    # Capture bloaty sections output as JSON-compatible data
    local sections_output
    sections_output=$(bloaty -n 0 -d sections "$binary_b" -- "$binary_a" 2>/dev/null) || true
    local symbols_output
    symbols_output=$(bloaty -n 20 -d symbols "$binary_b" -- "$binary_a" 2>/dev/null) || true
    local compileunits_output
    compileunits_output=$(bloaty -n 10 -d compileunits "$binary_b" -- "$binary_a" 2>/dev/null) || true

    # Emit as a JSON string field (escaped)
    local escaped_sections
    escaped_sections=$(printf '%s' "$sections_output" | python3 -c "import sys,json; print(json.dumps(sys.stdin.read()))")
    local escaped_symbols
    escaped_symbols=$(printf '%s' "$symbols_output" | python3 -c "import sys,json; print(json.dumps(sys.stdin.read()))")
    local escaped_compileunits
    escaped_compileunits=$(printf '%s' "$compileunits_output" | python3 -c "import sys,json; print(json.dumps(sys.stdin.read()))")

    printf '      "bloaty": {\n'
    printf '        "sections": %s,\n' "$escaped_sections"
    printf '        "symbols": %s,\n' "$escaped_symbols"
    printf '        "compileunits": %s\n' "$escaped_compileunits"
    printf '      }\n'
  else
    echo "  --- bloaty analysis ---"
    echo "  Sections:"
    bloaty -n 0 -d sections "$binary_b" -- "$binary_a" 2>/dev/null | sed 's/^/    /' || echo "    (bloaty sections failed)"
    echo ""
    echo "  Top 20 symbols by delta:"
    bloaty -n 20 -d symbols "$binary_b" -- "$binary_a" 2>/dev/null | sed 's/^/    /' || echo "    (bloaty symbols failed)"
    echo ""
    echo "  Top 10 compilation units by delta:"
    bloaty -n 10 -d compileunits "$binary_b" -- "$binary_a" 2>/dev/null | sed 's/^/    /' || echo "    (bloaty compileunits failed)"
    echo ""
  fi
}

# Main comparison loop
JSON_TMPFILE=""
if $JSON_OUTPUT; then
  JSON_TMPFILE=$(mktemp)
  echo "{" >> "$JSON_TMPFILE"
  echo "  \"baseline\": \"$SHERMES_A\"," >> "$JSON_TMPFILE"
  echo "  \"optimized\": \"$SHERMES_B\"," >> "$JSON_TMPFILE"
  echo "  \"typed\": $TYPED," >> "$JSON_TMPFILE"
  echo "  \"stripped\": $STRIP_BINARIES," >> "$JSON_TMPFILE"
  echo "  \"results\": [" >> "$JSON_TMPFILE"
fi

first_result=true
total_text_a=0
total_text_b=0
total_size_a=0
total_size_b=0
total_callsites=0

for js_file in "${JS_FILES[@]}"; do
  bench_name=$(basename "$js_file" .js)

  binary_a="$TMPDIR_SIZE/${bench_name}_a"
  binary_b="$TMPDIR_SIZE/${bench_name}_b"

  # Compile with both
  if ! compile_js "$SHERMES_A" "$js_file" "$binary_a" 2>/dev/null; then
    echo "Warning: Failed to compile $js_file with baseline, skipping" >&2
    continue
  fi
  if ! compile_js "$SHERMES_B" "$js_file" "$binary_b" 2>/dev/null; then
    echo "Warning: Failed to compile $js_file with optimized, skipping" >&2
    continue
  fi

  # Measurements
  size_a=$(file_size "$binary_a")
  size_b=$(file_size "$binary_b")
  text_a=$(text_section_size "$binary_a")
  text_b=$(text_section_size "$binary_b")

  # Count call sites (use baseline shermes for consistent count)
  callsites=$(count_get_by_val_calls "$SHERMES_A" "$js_file")

  # Compute deltas
  size_delta=$((size_b - size_a))
  text_delta=$((text_b - text_a))

  size_delta_pct="0.00"
  text_delta_pct="0.00"
  per_callsite_bytes="0"

  if [[ "$size_a" -gt 0 ]]; then
    size_delta_pct=$(python3 -c "print(f'{($size_delta / $size_a) * 100:.2f}')")
  fi
  if [[ "$text_a" -gt 0 ]]; then
    text_delta_pct=$(python3 -c "print(f'{($text_delta / $text_a) * 100:.2f}')")
  fi
  if [[ "$callsites" -gt 0 && "$text_delta" -gt 0 ]]; then
    per_callsite_bytes=$((text_delta / callsites))
  fi

  # Accumulate totals
  total_size_a=$((total_size_a + size_a))
  total_size_b=$((total_size_b + size_b))
  total_text_a=$((total_text_a + text_a))
  total_text_b=$((total_text_b + text_b))
  total_callsites=$((total_callsites + callsites))

  if $JSON_OUTPUT; then
    if $first_result; then
      first_result=false
    else
      echo "    ," >> "$JSON_TMPFILE"
    fi
    cat <<ENDJSON >> "$JSON_TMPFILE"
    {
      "name": "$bench_name",
      "file": "$js_file",
      "baseline": {
        "total_bytes": $size_a,
        "text_bytes": $text_a
      },
      "optimized": {
        "total_bytes": $size_b,
        "text_bytes": $text_b
      },
      "delta": {
        "total_bytes": $size_delta,
        "total_pct": $size_delta_pct,
        "text_bytes": $text_delta,
        "text_pct": $text_delta_pct
      },
      "get_by_val_callsites": $callsites,
      "per_callsite_overhead_bytes": $per_callsite_bytes
ENDJSON
    if $USE_BLOATY; then
      echo "      ," >> "$JSON_TMPFILE"
      bloaty_compare "$binary_a" "$binary_b" "$bench_name" >> "$JSON_TMPFILE"
    fi
    echo "    }" >> "$JSON_TMPFILE"
  else
    echo "=== $bench_name ==="
    printf "  %-25s %12s %12s %12s %8s\n" "" "Baseline" "Optimized" "Delta" "Delta%"
    printf "  %-25s %12d %12d %+12d %7s%%\n" "Total binary (bytes)" "$size_a" "$size_b" "$size_delta" "$size_delta_pct"
    printf "  %-25s %12d %12d %+12d %7s%%\n" ".text section (bytes)" "$text_a" "$text_b" "$text_delta" "$text_delta_pct"
    printf "  %-25s %12d\n" "get_by_val call sites" "$callsites"
    if [[ "$per_callsite_bytes" -gt 0 ]]; then
      printf "  %-25s %12d bytes\n" "Per-callsite overhead" "$per_callsite_bytes"
    fi
    echo ""
    if $USE_BLOATY; then
      bloaty_compare "$binary_a" "$binary_b" "$bench_name"
    fi
  fi

  # Cleanup binaries to save disk
  rm -f "$binary_a" "$binary_b"
done

# Summary
total_size_delta=$((total_size_b - total_size_a))
total_text_delta=$((total_text_b - total_text_a))
total_size_delta_pct="0.00"
total_text_delta_pct="0.00"
total_per_callsite="0"

if [[ "$total_size_a" -gt 0 ]]; then
  total_size_delta_pct=$(python3 -c "print(f'{($total_size_delta / $total_size_a) * 100:.2f}')")
fi
if [[ "$total_text_a" -gt 0 ]]; then
  total_text_delta_pct=$(python3 -c "print(f'{($total_text_delta / $total_text_a) * 100:.2f}')")
fi
if [[ "$total_callsites" -gt 0 && "$total_text_delta" -gt 0 ]]; then
  total_per_callsite=$((total_text_delta / total_callsites))
fi

if $JSON_OUTPUT; then
  cat <<ENDJSON >> "$JSON_TMPFILE"
  ],
  "summary": {
    "total_files": ${#JS_FILES[@]},
    "baseline_total_bytes": $total_size_a,
    "optimized_total_bytes": $total_size_b,
    "baseline_text_bytes": $total_text_a,
    "optimized_text_bytes": $total_text_b,
    "delta_total_bytes": $total_size_delta,
    "delta_total_pct": $total_size_delta_pct,
    "delta_text_bytes": $total_text_delta,
    "delta_text_pct": $total_text_delta_pct,
    "total_get_by_val_callsites": $total_callsites,
    "avg_per_callsite_overhead_bytes": $total_per_callsite,
    "bloaty_enabled": $USE_BLOATY
  }
}
ENDJSON
  python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" < "$JSON_TMPFILE"
  rm -f "$JSON_TMPFILE"
else
  echo "========================================"
  echo "SUMMARY (${#JS_FILES[@]} files)"
  echo "========================================"
  printf "  %-30s %12s %12s %+12s %8s\n" "" "Baseline" "Optimized" "Delta" "Delta%"
  printf "  %-30s %12d %12d %+12d %7s%%\n" "Total binary size" "$total_size_a" "$total_size_b" "$total_size_delta" "$total_size_delta_pct"
  printf "  %-30s %12d %12d %+12d %7s%%\n" ".text section total" "$total_text_a" "$total_text_b" "$total_text_delta" "$total_text_delta_pct"
  printf "  %-30s %12d\n" "Total get_by_val call sites" "$total_callsites"
  if [[ "$total_per_callsite" -gt 0 ]]; then
    printf "  %-30s %12d bytes\n" "Avg per-callsite overhead" "$total_per_callsite"
  fi
fi
