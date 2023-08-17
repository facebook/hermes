# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE on)

set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
set(jsinspector_SRC
  InspectorInterfaces.cpp
  )

add_hermes_library(jsinspector
  ${jsinspector_SRC})
