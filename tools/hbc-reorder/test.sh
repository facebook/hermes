#!/bin/bash
set -eu

die() {
  echo "$*" >&2
  exit 1
}

hermes="$1"
original="$2"
reordered="$3"
expected=$'1\n2\n3\n4\n5'

cmp "$original" "$reordered" &&
  die "The bundles are equal. One should be reordered."

for bundle in "$original" "$reordered"
do
  out=$("$hermes" "$bundle") || die "Can't run $bundle"
  [[ "$out" == "$expected" ]] ||
    die "Expected '$expected' but got '$out' running $bundle"
done

