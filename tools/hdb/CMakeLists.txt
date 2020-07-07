# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

include_directories(${HERMES_SOURCE_DIR}/API)
include_directories(${HERMES_JSI_DIR})
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
add_hermes_tool(hdb
  hdb.cpp
  ${ALL_HEADER_FILES}
  )

set_target_properties(hdb PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  )

target_link_libraries(hdb
  hermesapi
)

hermes_strip_apple_debug_symbols(hdb)

install(TARGETS hdb
  RUNTIME DESTINATION bin
)
