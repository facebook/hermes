# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(rustgen
  rustgen.cpp
  ${ALL_HEADER_FILES}
  )
