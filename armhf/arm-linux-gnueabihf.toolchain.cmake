# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Usage: -DCMAKE_TOOLCHAIN_FILE=path/to/arm-linux-gnueabihf.toolchain.cmake

# Specify the system name and processor
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
# Specify the Clang compiler
set(CMAKE_C_COMPILER clang-16)
set(CMAKE_CXX_COMPILER clang++-16)
# Specify the target triple for Clang
set(CMAKE_C_COMPILER_TARGET arm-linux-gnueabihf)
set(CMAKE_CXX_COMPILER_TARGET arm-linux-gnueabihf)
set(CMAKE_ASM_COMPILER_TARGET arm-linux-gnueabihf)
# Ensure the correct architecture is used and enable Thumb-2
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -march=armv7-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -march=armv7-a")
