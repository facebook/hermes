# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

set(HERMES_ENABLE_EH_RTTI OFF)

add_subdirectory(node_api)

add_hermes_library(hermesNodeApi STATIC
  hermes_node_api.h
  hermes_node_api.cpp
)

target_link_libraries(hermesNodeApi
  PUBLIC nodeApi
  PRIVATE hermesVMRuntime
)

target_include_directories(hermesNodeApi
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
