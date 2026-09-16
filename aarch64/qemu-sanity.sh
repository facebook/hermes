#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#
# Sanity checks for a cross-compiled arm64 Hermes running under qemu-user.
# Usage: aarch64/qemu-sanity.sh [build-dir]   (default: cmake-build-arm64)
#
# These are smoke tests, not a substitute for running on real hardware: they
# confirm that the JIT compiles, that its output matches the interpreter, and
# that emitted code is actually arm64 being executed.

set -u
BUILD="${1:-cmake-build-arm64}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QEMU="qemu-aarch64-static -L /usr/aarch64-linux-gnu"
HERMES="$ROOT/$BUILD/bin/hermes"
STRESS="$ROOT/aarch64/jit-stress.js"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0
ok()   { echo "  PASS  $1"; pass=$((pass + 1)); }
bad()  { echo "  FAIL  $1"; [ $# -gt 1 ] && echo "        $2"; fail=$((fail + 1)); }

# hermes has no eval-a-string flag, so snippets go through a temp file.
snippet=0
js() {
  snippet=$((snippet + 1))
  printf '%s\n' "$1" > "$TMP/s$snippet.js"
  echo "$TMP/s$snippet.js"
}

echo "=== arm64 JIT sanity (qemu-user) ==="
echo "build:  $BUILD"

# 0. Prerequisites.
command -v qemu-aarch64-static >/dev/null || { echo "qemu-aarch64-static not installed"; exit 1; }
[ -x "$HERMES" ] || { echo "missing $HERMES -- build it first"; exit 1; }

# 1. The binary really is aarch64.
if file "$HERMES" | grep -q "ARM aarch64"; then
  ok "hermes binary is ARM aarch64"
else
  bad "hermes binary is not aarch64" "$(file "$HERMES")"
fi

# 2. It runs at all under qemu.
hello="$(js 'print(6*7)')"
if [ "$($QEMU "$HERMES" "$hello" 2>&1)" = "42" ]; then
  ok "runs under qemu (interpreter)"
else
  bad "cannot run under qemu" "$($QEMU "$HERMES" "$hello" 2>&1 | head -3)"
fi

# 3. The JIT is compiled in and enabled (not silently a no-op build).
jitstatus="$($QEMU "$HERMES" -Xjit=force -Xdump-jitcode=2 "$(js 'function f(){return 1} f()')" 2>&1)"
ncompiled="$(printf '%s' "$jitstatus" | grep -c 'successfully compiled')"
if [ "$ncompiled" -gt 0 ]; then
  ok "JIT compiles functions ($ncompiled compiled)"
else
  bad "JIT compiled nothing -- is HERMESVM_ALLOW_JIT=2 set?" "$(printf '%s' "$jitstatus" | head -3)"
fi

# 4. Emitted code is arm64, and it is really being executed (not just emitted).
code="$($QEMU "$HERMES" -Xjit=force -Xdump-jitcode=1 "$(js 'function f(a){return a+1} f(1)')" 2>&1)"
if printf '%s' "$code" | grep -qE '\b(stp|ldp|blr|fmov|cbz|movk)\b'; then
  ok "emitted code contains arm64 instructions"
else
  bad "no arm64 mnemonics in dumped code" "$(printf '%s' "$code" | head -5)"
fi
# A JIT'ed function whose result depends on emitted arithmetic executing right.
jitval="$($QEMU "$HERMES" -Xjit=force -Xjit-crash-on-error \
  "$(js 'function f(a,b){var s=0;for(var i=0;i<a;++i)s+=i*b;return s} print(f(1000,3))')" 2>&1)"
if [ "$jitval" = "1498500" ]; then
  ok "JIT'ed code executes and computes correctly"
else
  bad "JIT'ed arithmetic wrong or crashed" "got: $(printf '%s' "$jitval" | head -3)"
fi

# 5. Differential test: JIT output must match the interpreter exactly.
$QEMU "$HERMES" "$STRESS" > "$TMP/interp.txt" 2> "$TMP/interp.err"
irc=$?
$QEMU "$HERMES" -Xjit=force -Xjit-crash-on-error "$STRESS" > "$TMP/jit.txt" 2> "$TMP/jit.err"
jrc=$?
if [ $irc -ne 0 ]; then
  bad "stress workload failed in the interpreter (rc=$irc)" "$(head -3 "$TMP/interp.err")"
elif [ $jrc -ne 0 ]; then
  bad "stress workload failed under the JIT (rc=$jrc)" "$(head -3 "$TMP/jit.err")"
elif cmp -s "$TMP/interp.txt" "$TMP/jit.txt"; then
  ok "JIT output matches interpreter ($(wc -l < "$TMP/interp.txt") lines)"
else
  bad "JIT/interpreter output differs" "$(diff "$TMP/interp.txt" "$TMP/jit.txt" | head -6 | tr '\n' ' ')"
fi

# 6. Same, with in-code assertions enabled (default on in Debug, explicit here).
$QEMU "$HERMES" -Xjit=force -Xjit-crash-on-error -Xjit-emit-asserts \
  "$STRESS" > "$TMP/jitassert.txt" 2> "$TMP/jitassert.err"
if [ $? -eq 0 ] && cmp -s "$TMP/interp.txt" "$TMP/jitassert.txt"; then
  ok "matches with -Xjit-emit-asserts"
else
  bad "failure with -Xjit-emit-asserts" "$(head -3 "$TMP/jitassert.err")"
fi

# 7. Threshold-based compilation (not just force), exercising the tier-up path.
tier="$($QEMU "$HERMES" -Xjit=on -Xjit-threshold=1 -Xjit-crash-on-error \
  "$(js 'function f(n){var s=0;for(var i=0;i<n;++i)s+=i;return s}
for(var k=0;k<50;++k)f(100); print(f(100))')" 2>&1)"
if [ "$tier" = "4950" ]; then
  ok "tier-up path (-Xjit=on) works"
else
  bad "tier-up path failed" "got: $(printf '%s' "$tier" | head -3)"
fi

# 8. Counters increment, i.e. JIT'ed calls really happen at runtime.
counters="$($QEMU "$HERMES" -Xjit=force -Xjit-emit-counters \
  "$(js 'function g(){return 1} function f(){var s=0;for(var i=0;i<200;++i)s+=g();return s} print(f())')" 2>&1)"
ncall="$(printf '%s' "$counters" | sed -n 's/^NumCall: \([0-9]*\).*/\1/p')"
if [ -n "$ncall" ] && [ "$ncall" -gt 0 ]; then
  ok "JIT call counters increment (NumCall=$ncall)"
else
  bad "JIT counters not incrementing" "$(printf '%s' "$counters" | head -3)"
fi

echo
echo "  $pass passed, $fail failed"
[ $fail -eq 0 ]
