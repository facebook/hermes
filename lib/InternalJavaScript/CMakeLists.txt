# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(IMPORT_SHERMES)
  file(TO_CMAKE_PATH "${IMPORT_SHERMES}" IMPORT_SHERMES_CMAKE)
  include(${IMPORT_SHERMES_CMAKE})
  set(shermes_EXE imported-shermes)
else()
  set(shermes_EXE shermes)
endif()

add_hermes_library(hermesInternalUnit internal_unit.c)
target_compile_options(hermesInternalUnit_obj PRIVATE -w)
target_include_directories(hermesInternalUnit_obj PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Concatenate all JS files into one source file for compilation.
# This way there is only one RuntimeModule made for them.
if(NOT CMAKE_HOST_WIN32)
  set(concatenate "cat")
else()
  set(concatenate "type")
endif()

# The concatenation order is specified by the numeric prefixes in the JS filenames.
file(GLOB internal_bytecode_sources "*.js")
list(SORT internal_bytecode_sources)

# Convert path from cmake style (`/`) to native style (`\` on Win and `/` elsewhere) for Win.
set(internal_bytecode_sources_native_path "")
foreach(cmake_path IN LISTS internal_bytecode_sources)
  file(TO_NATIVE_PATH ${cmake_path} native_path)
  list(APPEND internal_bytecode_sources_native_path ${native_path})
endforeach()

add_custom_command(
  OUTPUT InternalBytecode.js
  COMMAND ${concatenate} ${internal_bytecode_sources_native_path} > ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${internal_bytecode_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# Compile the one source file to Hermes bytecode.
add_custom_command(
  OUTPUT internal_unit.c
  COMMAND ${shermes_EXE} -O -g0 -w -emit-c -exported-unit=internal_unit -o=${CMAKE_CURRENT_BINARY_DIR}/internal_unit.c ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${shermes_EXE} InternalBytecode.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)
