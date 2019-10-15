# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: bash %s %S %T %hermes | %FileCheck %s
# shellcheck shell=bash disable=SC2086

SRCDIR=$1
TMPDIR=$2
HERMES=$3

HBC_FILE="$TMPDIR/out.hbc"
RAW_TRACE="$TMPDIR/raw_trace.txt"
SYM_TRACE="$TMPDIR/symbolicated_trace.txt"

"$HERMES" -gc-sanitize-handles=0 -commonjs -output-source-map \
  -emit-binary -out "$HBC_FILE" \
  "$SRCDIR"

"$HERMES" "$HBC_FILE" > "$RAW_TRACE"

# Stringize the sourcemap and raw stack trace, concatenate them along with
# the symbolicator source, and then execute that. The output will be a
# symbolicated stacktrace. This is to enable the test to run via Hermes alone
# (i.e. without introducing a node dependency).
( python3 "$SRCDIR/../stringize.py" SOURCEMAP < "$HBC_FILE.map" ;
  python3 "$SRCDIR/../stringize.py" STACKTRACE < "$RAW_TRACE" ;
  cat "$SRCDIR/../symbolicator.js.in"
) | "$HERMES" -gc-sanitize-handles=0 > "$SYM_TRACE"

cat "$SYM_TRACE"
# CHECK: Error: ERROR_FOR_TESTING
# CHECK:     at mod2fun ({{.*}}cjs-subdir-2.js:8:13)
# CHECK:     at run ({{.*}}cjs-subdir-unminified.js:9:20)
# CHECK:     at cjs_module ({{.*}}cjs-subdir-main.js:8:7)
