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

add_hermes_tool(hermes
  hermes.cpp
  repl.cpp
  LINK_OBJLIBS hermesCompilerDriver_obj hermesConsoleHost_obj
  LINK_LIBS hermesvm ${LIBREADLINE}
  )

if(HAVE_LIBREADLINE)
  target_compile_definitions(hermes PRIVATE HAVE_LIBREADLINE)
  set_target_properties(hermes PROPERTIES UNITY_BUILD false)
endif()

install(TARGETS hermes
  RUNTIME DESTINATION bin
)
