# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_subdirectory(cimgui)
add_subdirectory(sokol)
add_subdirectory(stb)

set(HERMES_BUILD "NOT-SET" CACHE STRING "Hermes build directory")
set(HERMES_SRC "NOT-SET" CACHE STRING "Hermes source directory")
set(SHERMES ${HERMES_BUILD}/bin/shermes CACHE STRING "shermes tool path")

include_directories(${HERMES_BUILD}/lib/config)
include_directories(${HERMES_SRC}/include)
include_directories(${HERMES_SRC}/Public)
include_directories(${HERMES_SRC}/API)
include_directories(${HERMES_SRC}/API/jsi)

add_library(gui gui.cpp gui.h)
target_link_libraries(gui sokol cimgui stb)
target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(shgui gui.cpp gui.h shgui.cpp)
target_link_libraries(shgui sokol cimgui stb)
