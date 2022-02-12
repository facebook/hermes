# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: bash %s %S %T %hermes
# shellcheck shell=bash disable=SC2086

# This test is an end-to-end source-map symbolication test. First it runs a
# file thrower.js which throws an exception, and records the backtrace (which
# has line and column numbers through its debug info). It then compiles the
# same file to bytecode with a source map (thus omitting the debug info from
# the binary), runs the bytecode file, and records that backtrace as the "raw"
# trace. It then symbolicates the raw trace using the source-map project from
# Mozilla, and compares the original and symbolicated trace. This tests that
# a source-map symbolicated backtrace is identical to the one output when
# debug info is present.

SRCDIR=$1
TMPDIR=$2
HERMES=$3

# Perform a symbolication test.
# Argument 1: a name for the test
# Argument 2: parameters to pass to hermes driver.
run_1_test() {
  NAME=$1;
  ARGS=$2;
  HBC_FILE=$3;
  SRC_FILES=("${@:4}");

  ORIG_TRACE="$TMPDIR/${NAME}_original_trace.txt"
  RAW_TRACE="$TMPDIR/${NAME}_raw_trace.txt"
  SYM_TRACE="$TMPDIR/${NAME}_symbolicated_trace.txt"

  # Run the JS directly, record the backtrace.
  "$HERMES" $ARGS "${SRC_FILES[@]}" > "$ORIG_TRACE"

  # Compile the JS to bytecode and have it emit a source map.
  # Run the bytecode and store its trace (which will contain addresses).
  "$HERMES" $ARGS -output-source-map -emit-binary -out "$HBC_FILE" "${SRC_FILES[@]}"
  "$HERMES" "$HBC_FILE" > "$RAW_TRACE"

  # Ensure that the two backtraces are different so the symbolicator has
  # real work to do.
  if diff -q "$ORIG_TRACE" "$RAW_TRACE" > /dev/null ; then
    echo "Raw and original backtraces should not be the same"
    echo "Raw trace: $RAW_TRACE"
    echo "Original trace: $ORIG_TRACE"
    echo "Artifacts in $TMPDIR"
    exit 1
  fi

  # Stringize the sourcemap and raw stack trace, concatenate them along with
  # the symbolicator source, and then execute that. The output will be a
  # symbolicated stacktrace. This is to enable the test to run via Hermes alone
  # (i.e. without introducing a node dependency).
  ( python3 $SRCDIR/stringize.py SOURCEMAP < "$HBC_FILE.map" ;
    python3 $SRCDIR/stringize.py STACKTRACE < "$RAW_TRACE" ;
    cat $SRCDIR/symbolicator.js.in
  ) | "$HERMES" - -target=HBC -gc-sanitize-handles=0 > "$SYM_TRACE"

  # Ensure that the original and symbolicated stack trace are the same!
  if ! diff "$ORIG_TRACE" "$SYM_TRACE" ; then
    echo "Symbolicated stack trace differed from debug-info version.";
    echo "Symbolicated trace: $SYM_TRACE"
    echo "Original trace: $ORIG_TRACE"
    echo "Artifacts in $TMPDIR";
    exit 1
  fi
}

(cat "$SRCDIR/stack-formatter.js.in" "$SRCDIR/thrower.js") > "$TMPDIR/thrower-with-formatter.js"

# -gc-sanitize-handles=0 because ASAN build is far too slow without it.
run_1_test "default"   "-gc-sanitize-handles=0 -target=HBC" "$TMPDIR/thrower.hbc" "$SRCDIR/thrower.js"
run_1_test "optimized" "-O -gc-sanitize-handles=0 -target=HBC" "$TMPDIR/thrower.hbc" "$SRCDIR/thrower.js"
run_1_test "default_cjs" "-commonjs -gc-sanitize-handles=0 -target=HBC" "$TMPDIR/cjs-1.hbc" "$SRCDIR/cjs-1.js" "$SRCDIR/cjs-2.js"
run_1_test "optimized_cjs" "-O -commonjs -gc-sanitize-handles=0 -target=HBC" "$TMPDIR/cjs-1.hbc" "$SRCDIR/cjs-1.js" "$SRCDIR/cjs-2.js"
run_1_test "formatter_optimized" "-w -O -gc-sanitize-handles=0 -target=HBC" "$TMPDIR/thrower-with-formatter.hbc" "$TMPDIR/thrower-with-formatter.js"
