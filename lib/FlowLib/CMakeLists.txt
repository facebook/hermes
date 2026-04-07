# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Concatenate all JS files into one source file.
if(NOT CMAKE_HOST_WIN32)
  set(concatenate "cat")
else()
  set(concatenate "type")
endif()

# The concatenation order is specified by numeric prefixes in filenames.
set(flowlib_sources
  "00-prelude.js"
  "01-array.js"
  "02-string.js"
)
list(SORT flowlib_sources)

set(flowlib_sources_native "")
foreach(cmake_path IN LISTS flowlib_sources)
  file(TO_NATIVE_PATH ${cmake_path} native_path)
  list(APPEND flowlib_sources_native ${native_path})
endforeach()

add_custom_command(
  OUTPUT FlowLib.js
  COMMAND ${concatenate} ${flowlib_sources_native} > ${CMAKE_CURRENT_BINARY_DIR}/FlowLib.js
  DEPENDS ${flowlib_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_command(
  OUTPUT FlowLib.inc
  COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py ${CMAKE_CURRENT_BINARY_DIR}/FlowLib.js > ${CMAKE_CURRENT_BINARY_DIR}/FlowLib.inc
  DEPENDS ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py FlowLib.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_target(FlowLibInclude ALL DEPENDS FlowLib.inc)

add_hermes_library(hermesFlowLib FlowLib.cpp)
target_include_directories(hermesFlowLib_obj PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hermesFlowLib_obj FlowLibInclude)
