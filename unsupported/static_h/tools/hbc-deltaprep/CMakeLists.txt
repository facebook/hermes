# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(hbc-deltaprep
  hbc-deltaprep.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hbc-deltaprep
  hermesVMRuntime
  hermesHBCBackend
  hermesSupport
)
