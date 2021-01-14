# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_definitions(-DHERMES_CMAKE_BUILD)

message(STATUS "CMAKE_HOST_SYSTEM_NAME = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "HERMES_APPLE_TARGET_PLATFORM = ${HERMES_APPLE_TARGET_PLATFORM}")

# Apple cross-compilation is different from other cross-compilation, so instead
# we use HERMES_APPLE_TARGET_PLATFORM to tell if a host hermesc is required.
if(HERMES_APPLE_TARGET_PLATFORM)
  set(CMAKE_CROSSCOMPILING TRUE)
endif()

message(STATUS "CMAKE_CROSSCOMPILING = ${CMAKE_CROSSCOMPILING}")

if(CMAKE_CROSSCOMPILING)
  set(IMPORT_HERMESC "IMPORTFILE-NOTFOUND" CACHE FILEPATH "hermesc export file")
  file(TO_CMAKE_PATH "${IMPORT_HERMESC}" IMPORT_HERMESC_CMAKE)
  include(${IMPORT_HERMESC_CMAKE})
  set(hermesc_EXE native-hermesc)
else()
  set(hermesc_EXE hermesc)
endif()

add_hermes_library(hermesInternalBytecode
    InternalBytecode.cpp
)

set(JS_COMPILER_FLAGS "-O" "-Wno-undefined-variable")

# Concatenate all JS files into one source file for compilation.
# This way there is only one RuntimeModule made for them.
# The concatenation order is set specifically so that order is controlled here.
if(NOT WIN32)
  set(concatenate "cat")
else()
  set(concatenate "type")
endif()

set(js_lib_sources
  header.js
  Promise.js
  footer.js
)

add_custom_command(
  OUTPUT JSLib.js
  COMMAND ${concatenate} ${js_lib_sources} > ${CMAKE_CURRENT_BINARY_DIR}/JSLib.js
  DEPENDS ${js_lib_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_command(
  OUTPUT JSLib.hbc
  COMMAND ${hermesc_EXE} ${JS_COMPILER_FLAGS} -emit-binary -out=${CMAKE_CURRENT_BINARY_DIR}/JSLib.hbc ${CMAKE_CURRENT_BINARY_DIR}/JSLib.js
  DEPENDS ${hermesc_EXE} JSLib.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# X.inc is a "C array" source file containing Hermes bytecode.
# It's built by compiling X with hermesc, and then piping that through xxd -i.

add_custom_command(
  OUTPUT InternalBytecode.inc
  COMMAND ${PYTHON_EXECUTABLE} xxd.py ${CMAKE_CURRENT_BINARY_DIR}/JSLib.hbc > ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.inc
  DEPENDS xxd.py JSLib.hbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

set_source_files_properties(InternalBytecode.cpp
  PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.inc
)

# Allow InternalBytecode to find its .inc file
target_include_directories(hermesInternalBytecode
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
