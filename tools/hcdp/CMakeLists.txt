# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)
add_hermes_tool(hcdp
  hcdp.cpp
  IPC.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hcdp hermesSerialExecutor libhermes)

install(TARGETS hcdp
  RUNTIME DESTINATION bin
)
