#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# collect_pmu.sh — Collect hardware PMU (Performance Monitoring Unit)
# counters for Static Hermes using perf stat with shermes -exec.
#
# Collects: instructions, cycles, L1-dcache-load-misses,
# L1-icache-load-misses, branch-misses, dTLB-load-misses.
# Computes derived ratios: IPC, miss rates.
#
# Usage:
#   collect_pmu.sh <shermes_binary> <workload.js> [--typed] [--json]
#                  [--compare <shermes_b>]
#                  [--shermes-args "extra args for shermes"]

set -euo pipefail

# Defaults
SHERMES=""
JS_FILE=""
SHERMES_B=""
TYPED=false
JSON_OUTPUT=false
COMPARE_MODE=false
SHERMES_EXTRA_ARGS=""

usage() {
  cat <<EOF
Usage: $(basename "$0") <shermes_binary> <workload.js> [options]

Collect hardware PMU counters for Static Hermes native binaries.

Arguments:
  <shermes_binary>       Path to shermes compiler binary
  <workload.js>          JavaScript workload file

Options:
  --typed                Compile with --typed flag
  --json                 Output structured JSON
  --compare <shermes_b>  A/B comparison mode with second shermes binary
  --shermes-args "ARGS"  Extra arguments to pass to shermes
  -h, --help             Show this help

Counters collected:
  instructions, cycles, L1-dcache-load-misses, L1-icache-load-misses,
  branch-misses, dTLB-load-misses

Derived metrics:
  IPC (instructions per cycle), cache miss rates, branch miss rate

Requires:
  perf (Linux perf_events). Exits with error if not available.

Examples:
  $(basename "$0") ./build/bin/shermes test.js --json
  $(basename "$0") ./build/bin/shermes bench.js --typed --compare ./build2/bin/shermes
EOF
  exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --typed) TYPED=true; shift ;;
    --json) JSON_OUTPUT=true; shift ;;
    --compare) COMPARE_MODE=true; SHERMES_B="$2"; shift 2 ;;
    --shermes-args) SHERMES_EXTRA_ARGS="$2"; shift 2 ;;
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

# Require perf
if ! command -v perf &>/dev/null; then
  echo "Error: perf is required but not found in PATH" >&2
  exit 1
fi

PMU_EVENTS="instructions:u,cycles:u,L1-dcache-load-misses:u,L1-icache-load-misses:u,branch-misses:u,dTLB-load-misses:u"

# Build shermes command for -exec mode
build_shermes_cmd() {
  local shermes_bin="$1"
  local js_file="$2"
  local -a cmd=("$shermes_bin")
  if $TYPED; then
    cmd+=("--typed")
  fi
  # shellcheck disable=SC2206
  cmd+=($SHERMES_EXTRA_ARGS)
  cmd+=("-exec" "$js_file")
  echo "${cmd[@]}"
}

# Collect PMU counters using shermes -exec, output parsed values
collect_pmu_counters() {
  local shermes_bin="$1"
  local js_file="$2"
  local -a cmd
  # shellcheck disable=SC2086
  read -ra cmd <<< "$(build_shermes_cmd "$shermes_bin" "$js_file")"
  local raw_output
  raw_output=$(perf stat -e "$PMU_EVENTS" -x',' "${cmd[@]}" 2>&1 >/dev/null)

  # Parse CSV output: value,unit,event_name,...
  python3 -c "
import sys

raw = '''$raw_output'''
counters = {}
for line in raw.strip().split('\n'):
    parts = line.split(',')
    if len(parts) < 3:
        continue
    val_str = parts[0].strip()
    event = parts[2].strip()
    # Remove :u suffix for cleaner names
    event = event.replace(':u', '')
    try:
        val = int(val_str)
    except ValueError:
        val = 0
    counters[event] = val

instructions = counters.get('instructions', 0)
cycles = counters.get('cycles', 0)
l1d_misses = counters.get('L1-dcache-load-misses', 0)
l1i_misses = counters.get('L1-icache-load-misses', 0)
branch_misses = counters.get('branch-misses', 0)
dtlb_misses = counters.get('dTLB-load-misses', 0)

ipc = instructions / cycles if cycles > 0 else 0.0

print(f'{instructions} {cycles} {l1d_misses} {l1i_misses} {branch_misses} {dtlb_misses} {ipc:.4f}')
"
}

# Format PMU results as JSON
format_pmu_json() {
  local label="$1"
  local shermes_bin="$2"
  local instructions="$3"
  local cycles="$4"
  local l1d_misses="$5"
  local l1i_misses="$6"
  local branch_misses="$7"
  local dtlb_misses="$8"
  local ipc="$9"

  cat <<ENDJSON
  {
    "label": "$label",
    "shermes": "$shermes_bin",
    "counters": {
      "instructions": $instructions,
      "cycles": $cycles,
      "L1_dcache_load_misses": $l1d_misses,
      "L1_icache_load_misses": $l1i_misses,
      "branch_misses": $branch_misses,
      "dTLB_load_misses": $dtlb_misses
    },
    "derived": {
      "ipc": $ipc,
      "l1d_miss_per_1k_insn": $(python3 -c "print(f'{$l1d_misses / max($instructions, 1) * 1000:.4f}')"),
      "l1i_miss_per_1k_insn": $(python3 -c "print(f'{$l1i_misses / max($instructions, 1) * 1000:.4f}')"),
      "branch_miss_per_1k_insn": $(python3 -c "print(f'{$branch_misses / max($instructions, 1) * 1000:.4f}')"),
      "dtlb_miss_per_1k_insn": $(python3 -c "print(f'{$dtlb_misses / max($instructions, 1) * 1000:.4f}')")
    }
  }
ENDJSON
}

# Format PMU results as text
format_pmu_text() {
  local label="$1"
  local shermes_bin="$2"
  local instructions="$3"
  local cycles="$4"
  local l1d_misses="$5"
  local l1i_misses="$6"
  local branch_misses="$7"
  local dtlb_misses="$8"
  local ipc="$9"

  echo "=== $label ($shermes_bin) ==="
  printf "  %-30s %'20d\n" "Instructions:" "$instructions"
  printf "  %-30s %'20d\n" "Cycles:" "$cycles"
  printf "  %-30s %20s\n" "IPC:" "$ipc"
  printf "  %-30s %'20d\n" "L1-dcache-load-misses:" "$l1d_misses"
  printf "  %-30s %'20d\n" "L1-icache-load-misses:" "$l1i_misses"
  printf "  %-30s %'20d\n" "Branch misses:" "$branch_misses"
  printf "  %-30s %'20d\n" "dTLB-load-misses:" "$dtlb_misses"
  echo ""
  echo "  Derived rates (per 1K instructions):"
  printf "    L1-D miss rate:   %s\n" "$(python3 -c "print(f'{$l1d_misses / max($instructions, 1) * 1000:.4f}')")"
  printf "    L1-I miss rate:   %s\n" "$(python3 -c "print(f'{$l1i_misses / max($instructions, 1) * 1000:.4f}')")"
  printf "    Branch miss rate: %s\n" "$(python3 -c "print(f'{$branch_misses / max($instructions, 1) * 1000:.4f}')")"
  printf "    dTLB miss rate:   %s\n" "$(python3 -c "print(f'{$dtlb_misses / max($instructions, 1) * 1000:.4f}')")"
}

# Main
echo "Collecting PMU counters for: $JS_FILE" >&2
echo "Using shermes: $SHERMES" >&2
read -r insn_a cyc_a l1d_a l1i_a br_a dtlb_a ipc_a <<< "$(collect_pmu_counters "$SHERMES" "$JS_FILE")"

if $COMPARE_MODE; then
  if [[ ! -x "$SHERMES_B" ]]; then
    echo "Error: Comparison binary not found or not executable: $SHERMES_B" >&2
    exit 1
  fi

  echo "Collecting PMU counters (B) with: $SHERMES_B" >&2
  read -r insn_b cyc_b l1d_b l1i_b br_b dtlb_b ipc_b <<< "$(collect_pmu_counters "$SHERMES_B" "$JS_FILE")"

  if $JSON_OUTPUT; then
    python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "typed": $TYPED,
  "mode": "compare",
  "a":
$(format_pmu_json "A" "$SHERMES" "$insn_a" "$cyc_a" "$l1d_a" "$l1i_a" "$br_a" "$dtlb_a" "$ipc_a"),
  "b":
$(format_pmu_json "B" "$SHERMES_B" "$insn_b" "$cyc_b" "$l1d_b" "$l1i_b" "$br_b" "$dtlb_b" "$ipc_b"),
  "deltas": {
    "instructions_pct": $(python3 -c "print(f'{(($insn_b - $insn_a) / max($insn_a, 1)) * 100:.4f}')"),
    "cycles_pct": $(python3 -c "print(f'{(($cyc_b - $cyc_a) / max($cyc_a, 1)) * 100:.4f}')"),
    "ipc_delta": $(python3 -c "print(f'{$ipc_b - $ipc_a:.4f}')"),
    "l1d_misses_pct": $(python3 -c "print(f'{(($l1d_b - $l1d_a) / max($l1d_a, 1)) * 100:.4f}')"),
    "branch_misses_pct": $(python3 -c "print(f'{(($br_b - $br_a) / max($br_a, 1)) * 100:.4f}')")
  }
}
ENDJSON
  else
    format_pmu_text "A" "$SHERMES" "$insn_a" "$cyc_a" "$l1d_a" "$l1i_a" "$br_a" "$dtlb_a" "$ipc_a"
    echo ""
    format_pmu_text "B" "$SHERMES_B" "$insn_b" "$cyc_b" "$l1d_b" "$l1i_b" "$br_b" "$dtlb_b" "$ipc_b"
    echo ""
    echo "=== Deltas (B vs A) ==="
    printf "  Instructions: %s%%\n" "$(python3 -c "print(f'{(($insn_b - $insn_a) / max($insn_a, 1)) * 100:.4f}')")"
    printf "  Cycles:       %s%%\n" "$(python3 -c "print(f'{(($cyc_b - $cyc_a) / max($cyc_a, 1)) * 100:.4f}')")"
    printf "  IPC delta:    %s\n" "$(python3 -c "print(f'{$ipc_b - $ipc_a:.4f}')")"
  fi
else
  if $JSON_OUTPUT; then
    python3 -c "import sys,json; print(json.dumps(json.load(sys.stdin)))" <<ENDJSON
{
  "file": "$JS_FILE",
  "typed": $TYPED,
  "mode": "single",
  "result":
$(format_pmu_json "single" "$SHERMES" "$insn_a" "$cyc_a" "$l1d_a" "$l1i_a" "$br_a" "$dtlb_a" "$ipc_a")
}
ENDJSON
  else
    format_pmu_text "Result" "$SHERMES" "$insn_a" "$cyc_a" "$l1d_a" "$l1i_a" "$br_a" "$dtlb_a" "$ipc_a"
  fi
fi
