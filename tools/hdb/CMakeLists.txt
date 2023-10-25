# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
add_hermes_tool(hdb hdb.cpp LINK_LIBS hermesvm_a)

set_target_properties(hdb PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  )

install(TARGETS hdb
  RUNTIME DESTINATION bin
)
