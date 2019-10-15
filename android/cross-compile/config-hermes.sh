#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake ~/fbsource/xplat/hermes \
    -DHERMES_FACEBOOK_BUILD=OFF -DLLVM_ENABLE_LTO=OFF \
    -DLLVM_BUILD_DIR=$PWD/../llvm_arm_clang  -DLLVM_SRC_DIR=$PWD/../llvm \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android-clang-arm-api13-gnustl.cmake \
    -DICU_ROOT=/Users/tmikov/3rd/swift-libiconv-libicu-android.git/armeabi-v7a -G Ninja
