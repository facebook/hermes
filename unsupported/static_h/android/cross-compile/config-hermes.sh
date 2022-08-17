#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake ~/fbsource/xplat/hermes \
    -DHERMES_FACEBOOK_BUILD=OFF \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_TOOLCHAIN_FILE=/opt/android_ndk/android-ndk-r15c/build/cmake/android.toolchain.cmake \
    -DICU_ROOT=/Users/tmikov/3rd/swift-libiconv-libicu-android.git/armeabi-v7a -G Ninja $*
