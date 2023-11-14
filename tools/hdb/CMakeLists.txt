# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)
add_hermes_tool(hdb
  hdb.cpp
  ${ALL_HEADER_FILES}
  )

# TODO: Even though this does not use llvh at all, a quirk of our build is that
#       pthread is found and provided by llvh.
target_link_libraries(hdb libhermes LLVHSupport)

install(TARGETS hdb
  RUNTIME DESTINATION bin
)
