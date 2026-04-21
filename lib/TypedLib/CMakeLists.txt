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
set(typedlib_sources
  "00-prelude.js"
  "01-array.js"
  "02-string.js"
  "03-map.js"
  "04-set.js"
)
list(SORT typedlib_sources)

set(typedlib_sources_native "")
foreach(cmake_path IN LISTS typedlib_sources)
  file(TO_NATIVE_PATH ${cmake_path} native_path)
  list(APPEND typedlib_sources_native ${native_path})
endforeach()

add_custom_command(
  OUTPUT TypedLib.js
  COMMAND ${concatenate} ${typedlib_sources_native} > ${CMAKE_CURRENT_BINARY_DIR}/TypedLib.js
  DEPENDS ${typedlib_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_command(
  OUTPUT TypedLib.inc
  COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py ${CMAKE_CURRENT_BINARY_DIR}/TypedLib.js > ${CMAKE_CURRENT_BINARY_DIR}/TypedLib.inc
  DEPENDS ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py TypedLib.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_target(TypedLibInclude ALL DEPENDS TypedLib.inc)

add_hermes_library(hermesTypedLib TypedLib.cpp)
target_include_directories(hermesTypedLib_obj PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(hermesTypedLib_obj TypedLibInclude)
