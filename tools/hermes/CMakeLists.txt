# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

find_library(LIBREADLINE_FOUND readline)
find_library(LIBTINFO_FOUND tinfo)

if(LIBREADLINE_FOUND)
  set(LIBREADLINE ${LIBREADLINE_FOUND})
endif()
if(LIBTINFO_FOUND)
  list(APPEND LIBREADLINE ${LIBTINFO_FOUND})
endif()
set(CMAKE_REQUIRED_LIBRARIES ${LIBREADLINE})
check_function_exists(readline HAVE_LIBREADLINE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(hermes
  hermes.cpp
  repl.cpp
  ${ALL_HEADER_FILES}
  )

if(HAVE_LIBREADLINE)
  target_compile_definitions(hermes PRIVATE HAVE_LIBREADLINE)
  set_target_properties(hermes PROPERTIES UNITY_BUILD false)
endif()

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
