#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ "$#" -ne 3 ]; then
  echo "Usage: run_fnc.sh FNC SOURCE OUTPUT"
  exit 1
fi

FNRUNTIME_DIR=$(dirname ${0})/runtime

"${1}" < "${2}" | c++ -x c++ -I "${FNRUNTIME_DIR}" "${FNRUNTIME_DIR}/FNRuntime.cpp" - -O3 -std=c++14 -o "${3}"
