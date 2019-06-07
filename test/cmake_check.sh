#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

cm_lists=$(mktemp /tmp/cmake_check.XXXXXX)
trap 'rm "$cm_lists"' EXIT

hermes=$1
if [ ! -d "$hermes/include/hermes" ]
then
  echo "Hermes directory not supplied"
  exit 1
fi

find "$hermes" -name CMakeLists.txt -exec cat {} + > "$cm_lists"
# Exclude paths in hidden directories.
for path in $(find "$hermes" -name "*.cpp" -not -path '*/\.*')
do
    # Except some paths
    case "$path" in
        $hermes/facebook/*) continue;;
        $hermes/first-party/*) continue;;
        $hermes/API/hermes/synthtest/tests/*) continue;;
    esac
    base=$(basename "$path")
    if ! grep -Fq "$base" "$cm_lists"
    then
        echo "missing $path"
        exit 1
    fi
done
