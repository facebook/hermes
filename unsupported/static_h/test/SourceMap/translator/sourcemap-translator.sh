# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: bash %s %S %T %hermes
# shellcheck shell=bash disable=SC2086

# This test is an end-to-end merge input source-map with hermes debuginfo symbolication test.
# First it runs original non-transformed file throw.js which throws an exception, and records the backtrace.
# It then runs the uglifyjs generated throw.min.js with input source map and records that backtrace
# as the "symbolicated trace".
# Finally it diffs both traces(ignoring full path/column number) to ensure they are the same.
# This tests that generated js code backtrace merged with input source map is identical
# to the original source file backtrace.

SRCDIR=$1
TMPDIR=$2
HERMES=$3

# Perform a symbolication test.
# Argument 1: a name for the test
# Argument 2: parameters to pass to hermes driver.
# Argument 3: input js file to evaluate.
# Argument 4: generated minified file correspoinding to input js file.
# Argument 5: source map for input js file.
run_1_test() {
  NAME=$1;
  ARGS=$2;
  ORIGINAL_JS_FILE=$3;
  GENERATED_JS_FILE=$4;
  JS_FILE_SOURCE_MAP=$5;

  ORIG_TRACE="$TMPDIR/${NAME}_original_trace.txt"
  SYM_TRACE="$TMPDIR/${NAME}_symbolicated_trace.txt"

  # Run the original JS, record the backtrace.
  "$HERMES" $ARGS "$ORIGINAL_JS_FILE" &> "$ORIG_TRACE"

  # Run the generated JS with input source file, record the backtrace symbolicated with merged debuginfo.
  "$HERMES" $ARGS "$GENERATED_JS_FILE" -source-map "$JS_FILE_SOURCE_MAP" &> "$SYM_TRACE"

  # Check if original and symbolicated traces using input source map match using stacktrace_diff.py.
  # stacktrace_diff.py matches stack frames ignoring file full path and column number. This is necessary because:
  # 1. Ignore full path: Original trace's file full path depends on the running matchine while file path
  #   in source map is hard-coded with full path stripped.
  # 2. Ignore column number: Uglifyjs generated source map does not have full fidelity range mapping
  #   for every single tokens so if a generated line/column is in the middle of a mapping range the symbolicated
  #   trace will report the beginning of the range in original source most likely diff in column numbers.
  python3 "$SRCDIR/stacktrace_diff.py" "$ORIG_TRACE" "$SYM_TRACE"
}

# -gc-sanitize-handles=0 because ASAN build is far too slow without it.
run_1_test "default"   "-gc-sanitize-handles=0 -target=HBC" "$SRCDIR/throw.js" "$SRCDIR/throw.min.js" "$SRCDIR/throw.min.js.map"
run_1_test "optimized"   "-O -gc-sanitize-handles=0 -target=HBC" "$SRCDIR/throw.js" "$SRCDIR/throw.min.js" "$SRCDIR/throw.min.js.map"
