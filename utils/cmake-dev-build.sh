#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Thin wrapper script for setting up a CMake build directory which has options
# to ensure that large portions of code aren't commented out.
# For example, enables HV32 and the debugger to make sure they can compile
# while you're developing.
# Usage: use it just like cmake.
#   cmake-dev-build.sh /path/to/static_h -GNinja any other args here

cmake \
  -DHERMES_ENABLE_DEBUGGER=ON \
  -DHERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32 \
  "$@"
