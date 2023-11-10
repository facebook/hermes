# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_executable(rtcpp raytracer.cpp)
target_include_directories(rtcpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/)

add_executable(rtcpp-gui raytracer.cpp)
target_compile_definitions(rtcpp-gui PRIVATE -DUSE_GUI=1)
target_link_libraries(rtcpp-gui gui)
