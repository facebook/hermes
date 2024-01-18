# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.27)
project(sqtest)

set(CMAKE_CXX_STANDARD 17)

add_library(sqmock STATIC sqmock.cpp sqmock.h)

add_executable(sqread-cpp sqread-cpp.cpp)
target_link_libraries(sqread-cpp sqmock)

add_custom_command(
        OUTPUT sqread${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND ${SHERMES} $<$<CONFIG:Debug>:-g3> -typed
        -o ${CMAKE_CURRENT_BINARY_DIR}/sqread${CMAKE_EXECUTABLE_SUFFIX}
        -L ${CMAKE_CURRENT_BINARY_DIR}
        -lsqmock -lc++
        sqread.js
        DEPENDS sqread.js sqmock
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(sqread-bin DEPENDS sqread${CMAKE_EXECUTABLE_SUFFIX})
