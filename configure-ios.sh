#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

PLATFORM_NAME="${PLATFORM_NAME:-iphoneos}"
CURRENT_ARCH="${CURRENT_ARCH}"

if [ -z "$CURRENT_ARCH" ] || [ "$CURRENT_ARCH" == "undefined_arch" ]; then
    # Xcode 10 beta sets CURRENT_ARCH to "undefined_arch", this leads to incorrect linker arg.
    # it's better to rely on platform name as fallback because architecture differs between simulator and device
    if [[ "$PLATFORM_NAME" == *"simulator"* ]]; then
        CURRENT_ARCH="x86_64"
    else
        CURRENT_ARCH="armv7"
    fi
fi

if [[ "$CONFIGURATION" = *Debug* ]]; then
    BUILD_TYPE="--build-type=Debug"
else
    BUILD_TYPE="--distribute"
fi

CMAKE_FLAGS=" \
    -DCMAKE_OSX_SYSROOT:STRING=$PLATFORM_NAME \
    -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
    -DHERMES_ENABLE_FUZZING:BOOLEAN=false \
    -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
    -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true \
    -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true
    -DCMAKE_INSTALL_PREFIX:PATH=../destroot"

cd $PODS_TARGET_SRCROOT
./utils/build/configure.py "$BUILD_TYPE" --cmake-flags "$CMAKE_FLAGS" --build-system="Ninja" build 

cd build
ninja install/strip