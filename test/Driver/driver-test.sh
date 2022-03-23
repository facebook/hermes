# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: sh %s %S %t %hermes
# RUN: sh %s %S %t %hermesc

# shellcheck disable=SC2148

SRCDIR=$1
TMPFILE=$2
HERMES=$3

# Return values from hermes and hermesc.
Success=0
InvalidFlags=1
ParsingFailed=2
# shellcheck disable=SC2034
VerificationFailed=3
LoadGlobalsFailed=4
InputFileError=5
OutputFileError=6
# shellcheck disable=SC2034
OptimizationFailed=7

set -x

expect() {
  EXPECTED=$1
  shift
  eval "$@" 2>"$TMPFILE"
  if [[ $? != "$EXPECTED" ]]; then
    echo "Command '$CMD' produced wrong exit status" >&2
    exit 1
  fi
  true
}

cd "${SRCDIR}" || exit 1
expect "${Success}" "${HERMES}" test.js.in -target=HBC -emit-binary > "$TMPFILE"
expect "${InvalidFlags}" "${HERMES}" -lazy -commonjs test.js.in
expect "${InvalidFlags}" "${HERMES}" -nonsenseflag test.js.in
expect "${ParsingFailed}" "${HERMES}" bogus.js.in -target=HBC -emit-binary > "$TMPFILE"
expect "${LoadGlobalsFailed}" "${HERMES}" test.js.in -include-globals bogus.js.in -emit-binary -target=HBC > "$TMPFILE"
expect "${InputFileError}" "${HERMES}" ./not/a/valid/path.js -target=HBC -emit-binary > "$TMPFILE"
expect "${OutputFileError}" "${HERMES}" test.js.in -target=HBC -emit-binary -out ./not/a/valid/path.hbc
