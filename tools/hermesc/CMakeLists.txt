# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(hermesc
  hermesc.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hermesc
  hermesCompilerDriver
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesInst
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
)

# The Hermes compiler is used as part of the build of the VM.
# During cross-compilation, the compiler needs to be built for the host system,
# then used to build the VM for the target system.
if(NOT CMAKE_CROSSCOMPILING)
  # Namespace added to avoid clashing the host binary with the target binary.
  export(TARGETS hermesc FILE ${CMAKE_BINARY_DIR}/ImportHermesc.cmake NAMESPACE native-)
endif()

install(TARGETS hermesc
  RUNTIME DESTINATION bin
)
