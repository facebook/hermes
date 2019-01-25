# RUN: bash %s %S %T %hbc-deltaprep %hermes
# shellcheck shell=bash

# Exit on any failure.
set -e
set -o pipefail

SRCDIR=$1
TMPDIR=$2
DELTAPREP=$3
HERMES=$4

# Compile the source and delta-prep it, then compare the original.
${HERMES} -emit-binary -out "${TMPDIR}/file1.hbc" "${SRCDIR}/file1.js"
${DELTAPREP} "${TMPDIR}/file1.hbc" -form=delta -out "${TMPDIR}/file1-delta.hbc"
! diff -q "${TMPDIR}/file1.hbc" "${TMPDIR}/file1-delta.hbc"

# Convert back and verify it matches the original.
${DELTAPREP} "${TMPDIR}/file1-delta.hbc" -form=execution -out "${TMPDIR}/file1-roundtrip.hbc"
! diff -q "${TMPDIR}/file1-delta.hbc" "${TMPDIR}/file1-roundtrip.hbc"
diff -q "${TMPDIR}/file1.hbc" "${TMPDIR}/file1-roundtrip.hbc"
