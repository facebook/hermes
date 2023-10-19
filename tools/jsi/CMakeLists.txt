# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

add_hermes_tool(hermes-jsi
  jsi.cpp
  ${ALL_HEADER_FILES}
  LINK_LIBS hermesapi timerStats
  )

# TODO: We have to disable RTTI here because we use LLVM libraries compiled
#       without RTTI. Disabling RTTI with EH enabled is likely to cause subtle
#       bugs, so we should try to remove this workaround.
if (GCC_COMPATIBLE)
  target_compile_options(hermes-jsi PRIVATE -fno-rtti)
endif()
