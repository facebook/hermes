# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(SHERMES_CC_INCLUDE_PATH
  "${CMAKE_BINARY_DIR}/lib/config:${CMAKE_SOURCE_DIR}/include" CACHE STRING
  "Include path when invoking CC")
set(SHERMES_CC_LIB_PATH "${CMAKE_BINARY_DIR}/lib" CACHE STRING
  "Library path when invoking CC")

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_hermes_tool(shermes
  shermes.cpp
  compile.cpp compile.h
  LINK_LIBS hermescompiler)

# A target building both the shermes compiler and the libraries potentially
# needed for linking of a compiled binary.
# Note that it is not a given that we always want to link against these
# libraries, we might want to link against release libraries from a debug
# compiler, so this is not the default for shermes.
add_custom_target(shermes-dep)
add_dependencies(shermes-dep shermes hermesvm_a hermesvmlean_a)
