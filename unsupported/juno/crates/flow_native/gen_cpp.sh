#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ "$#" -ne 1 ]; then
  echo "Usage: gen_cpp.sh SOURCE"
  exit 1
fi

FNRUNTIME_DIR=$(dirname ${0})/runtime
BIN_DIR=$(dirname ${0})/../../target/debug

name="${1%.js}"
name="${name##*/}"
"$BIN_DIR/fnc" "${1}" | clang-format > "${name}.cpp"
