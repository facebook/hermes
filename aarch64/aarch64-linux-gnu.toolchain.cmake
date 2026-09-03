# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Cross-compile for aarch64 Linux, for running under qemu-user on an x86-64
# host. This is the only way to exercise the arm64 JIT without arm64 hardware.
#
# Usage: -DCMAKE_TOOLCHAIN_FILE=path/to/aarch64-linux-gnu.toolchain.cmake
#
# Requires (Debian/Ubuntu):
#   apt install qemu-user-static gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
# clang supplies the compiler; the gcc-aarch64-linux-gnu package supplies the
# aarch64 glibc, libstdc++ and binutils that clang drives.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Use clang as a cross compiler, driving the GNU aarch64 binutils/libs.
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER_TARGET aarch64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET aarch64-linux-gnu)
set(CMAKE_ASM_COMPILER_TARGET aarch64-linux-gnu)

# NOTE: deliberately no --sysroot. On Debian/Ubuntu the cross packages install
# linker scripts (e.g. /usr/aarch64-linux-gnu/lib/libm.so) that reference
# absolute host paths; passing a sysroot makes the linker prepend it to those
# paths and the libraries are then "not found inside" the sysroot.

# Look for target libraries and headers in the cross directory, but keep using
# host programs (python, etc.) for anything the build needs to execute.
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
