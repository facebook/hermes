#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This test compiles hermes bytecode with -base-bytecode.

set -x -e -o pipefail

exec 2>&1

# hermesc binary
HERMESC=$1
# Path to the base JS bundle.
BASE_JS=$2
# Path to the base JS bundle.
UPDATE_JS=$3

OUTDIR="$(mktemp -d)"
BASE_HBC="$OUTDIR/base.hbc"

UPDATE_DUMP="$OUTDIR/update.dump"
UPDATE_HBC="$OUTDIR/update.hbc"
UPDATE_HBC_DUMP="$OUTDIR/update.hbc.dump"

# Generate base HBC
"$HERMESC" "$BASE_JS" -emit-binary -out "$BASE_HBC" \
  -O -g0 -fstrip-function-names -w -verify-ir=0

# Dump bytecode directly.
"$HERMESC" "$UPDATE_JS" -dump-bytecode \
  -base-bytecode "$BASE_HBC" \
  -O -g0 -fstrip-function-names -w -verify-ir=0 > "$UPDATE_DUMP"

# Dump bytecode of serialized HBC file.
"$HERMESC" "$UPDATE_JS" -emit-binary -out "$UPDATE_HBC" \
  -base-bytecode "$BASE_HBC" \
  -O -g0 -fstrip-function-names -w -verify-ir=0
"$HERMESC" "$UPDATE_HBC" -dump-bytecode > "$UPDATE_HBC_DUMP"

diff --text <(head "$UPDATE_DUMP") <(head "$UPDATE_HBC_DUMP") || \
  { echo "Bytecode dump diff failed with status $?"; exit 1;}

echo "Test passed"
