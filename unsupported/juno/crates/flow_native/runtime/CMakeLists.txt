# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.21)
project(runtime)

set(CMAKE_CXX_STANDARD 17)

add_library(FNRuntime FNRuntime.cpp FNRuntime.h)
