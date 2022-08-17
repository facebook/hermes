# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SET(CMAKE_SYSTEM_NAME Android)
SET(CMAKE_SYSTEM_VERSION 15)

SET(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
SET(CMAKE_ANDROID_NDK /opt/android_ndk/android-ndk-r15c)
SET(CMAKE_ANDROID_STL_TYPE gnustl_shared)
SET(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

SET(CMAKE_POSITION_INDEPENDENT_CODE True)
