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

set(internal_bytecode_sources
  header.js
  Promise.js
  AsyncFn.js
  footer.js
)

add_custom_command(
  OUTPUT InternalBytecode.js
  COMMAND ${concatenate} ${internal_bytecode_sources} > ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${internal_bytecode_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# Compile the one source file to Hermes bytecode.
add_custom_command(
  OUTPUT InternalBytecode.hbc
  COMMAND ${hermesc_EXE} ${JS_COMPILER_FLAGS} -fno-enable-tdz -emit-binary -out=${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.hbc ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${hermesc_EXE} InternalBytecode.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)


# X.inc is the hex dump of the compiled bytecode artifact that can be included
# in the InternalBytecode.cpp as C/C++ hex literal to initialize a byte array.
add_custom_command(
  OUTPUT InternalBytecode.inc
  COMMAND ${PYTHON_EXECUTABLE} xxd.py ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.hbc > ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.inc
  DEPENDS xxd.py InternalBytecode.hbc
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
