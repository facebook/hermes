# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

find_library(LIBREADLINE_FOUND readline)
find_library(LIBTINFO_FOUND tinfo)

if(LIBREADLINE_FOUND AND LIBTINFO_FOUND)
  set(HAVE_LIBREADLINE 1)
  set(LIBREADLINE ${LIBREADLINE_FOUND} ${LIBTINFO_FOUND})
endif()

configure_file(ReplConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/ReplConfig.h
  ESCAPE_QUOTES)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(hermes
  hermes.cpp
  repl.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hermes
  hermesVMRuntime
  hermesCompilerDriver
  hermesConsoleHost
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSourceMap
  hermesSupport
  dtoa
  hermesInstrumentation
  ${LIBREADLINE}
)

install(TARGETS hermes
  RUNTIME DESTINATION bin
)
