# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(synth
  synth.cpp
)

target_link_libraries(synth
  hermesVMRuntime
  hermesConsoleHost
  hermesapi
  traceInterpreter
)

# TODO: We have to disable RTTI here because we use LLVM libraries compiled
#       without RTTI. Disabling RTTI with EH enabled is likely to cause subtle
#       bugs, so we should try to remove this workaround.
if (GCC_COMPATIBLE)
  target_compile_options(synth PRIVATE -fno-rtti)
endif()
