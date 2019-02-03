#!/bin/bash

cmake ~/fbsource/xplat/hermes \
    -DHERMES_FACEBOOK_BUILD=OFF -DLLVM_ENABLE_LTO=OFF \
    -DLLVM_BUILD_DIR=$PWD/../llvm_arm_clang  -DLLVM_SRC_DIR=$PWD/../llvm \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android-clang-arm-api13-gnustl.cmake \
    -DICU_ROOT=/Users/tmikov/3rd/swift-libiconv-libicu-android.git/armeabi-v7a -G Ninja
