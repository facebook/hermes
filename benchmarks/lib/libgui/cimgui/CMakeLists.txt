# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_library(cimgui OBJECT
        cimgui.cpp
        cimgui.h
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imgui_widgets.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_demo.cpp)
target_include_directories(cimgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
