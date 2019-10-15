#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android-arm-api13-gnustl.cmake \
    -DLLVM_TARGETS_TO_BUILD= -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off \
    -DLLVM_TABLEGEN=/Users/tmikov/work/hws/llvm_build/bin/llvm-tblgen \
    -DCLANG_TABLEGEN=/Users/tmikov/work/hws/llvm_build/bin/clang-tblgen \
    -GNinja ../llvm
