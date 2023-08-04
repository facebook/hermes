# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB hermesinspector_SRC CONFIGURE_DEPENDS *.cpp chrome/*.cpp)

add_library(hermes_inspector
        STATIC
        ${hermesinspector_SRC})

target_link_libraries(hermes_inspector
  hermesapi
)
