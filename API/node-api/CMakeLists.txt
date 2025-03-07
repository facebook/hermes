# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hermesNapi INTERFACE)
target_include_directories(hermesNapi INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(DIRECTORY "${PROJECT_SOURCE_DIR}/API/node-api/" DESTINATION include
  FILES_MATCHING PATTERN "*.h")
