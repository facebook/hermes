#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This test compiles hermes bytecode from a large js bundle several times and
# verifies that the output is always the same.

set -x -e -o pipefail

exec 2>&1

# hermesc binary
HERMESC=$1
# Path to the large js bundle.
JS_BUNDLE=$2
# Number of iterations that we compile from the same source and compare.
NUM_COMP=5

FIRST_HBC="$(mktemp)" || { echo "Failed to create first hbc temp file"; exit 1; }
# Compile first bytecode.
"$HERMESC" -O -target=HBC -emit-binary -Wno-undefined-variable -out "$FIRST_HBC" "$JS_BUNDLE" 2>/dev/null

SECOND_HBC="$(mktemp)" || { echo "Failed to create second hbc temp file"; exit 1; }
for (( i=1; i<=$((NUM_COMP)); i++ ))
do
    # Compile second bytecode.
    "$HERMESC" -O -target=HBC -emit-binary -Wno-undefined-variable -out "$SECOND_HBC" "$JS_BUNDLE" 2>/dev/null
    # Exits with non-zero status if the bytecode files differ.
    diff "$FIRST_HBC" "$SECOND_HBC" || { echo "Bytecode diff failed with status $?."; exit 1; }
done
echo "Bytecode looks deterministic. Success!"
