# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

find_library(LIBREADLINE_FOUND readline)

if(LIBREADLINE_FOUND)
  set(HAVE_LIBREADLINE 1)
  set(LIBREADLINE ${LIBREADLINE_FOUND})
endif()

configure_file(ReplConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/ReplConfig.h
  ESCAPE_QUOTES)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hermes-repl
  repl.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hermes-repl
  hermesVMRuntime
  hermesCompilerDriver
  hermesConsoleHost
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
  dtoa
  ${CORE_FOUNDATION}
  ${LIBREADLINE}
)

hermes_link_icu(hermes-repl)
