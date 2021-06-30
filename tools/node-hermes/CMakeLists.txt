# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

include_directories(${HERMES_SOURCE_DIR}/API)
include_directories(${HERMES_JSI_DIR})

set(HERMES_ENABLE_EH ON)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

add_subdirectory(nodelib)
add_hermes_tool(node-hermes
  node-hermes.cpp
  ${ALL_HEADER_FILES}
  LINK_LIBS hermesapi hermesNodeBytecode
  )
