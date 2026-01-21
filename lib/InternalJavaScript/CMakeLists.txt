# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(IMPORT_HOST_COMPILERS)
  file(TO_CMAKE_PATH "${IMPORT_HOST_COMPILERS}" IMPORT_HOST_COMPILERS_CMAKE)
  include(${IMPORT_HOST_COMPILERS_CMAKE})
  set(shermes_EXE imported-shermes)
  set(hermesc_EXE imported-hermesc)
else()
  set(shermes_EXE ${HERMES_TOOLS_OUTPUT_DIR}/shermes${CMAKE_EXECUTABLE_SUFFIX})
  set(hermesc_EXE ${HERMES_TOOLS_OUTPUT_DIR}/hermesc${CMAKE_EXECUTABLE_SUFFIX})
endif()

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

# Compile the one source file to a C file.
add_custom_command(
  OUTPUT internal_unit.c
  COMMAND ${shermes_EXE} -O -g0 -w -emit-c -exported-unit=internal_unit -o=${CMAKE_CURRENT_BINARY_DIR}/internal_unit.c ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${shermes_EXE} InternalBytecode.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# Native compiled InternalJavaScript target
# Always build it to prevent bitrot, except on MSVC where the generated C code
# is not compatible with MSVC's C compiler.
if(HERMESVM_INTERNAL_JAVASCRIPT_NATIVE OR NOT MSVC)
  add_hermes_library(hermesInternalUnit internal_unit.c)
  target_compile_options(hermesInternalUnit_obj PRIVATE -w)
  target_include_directories(hermesInternalUnit_obj PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Bytecode compiled InternalJavaScript target
add_hermes_library(hermesInternalBytecode
    InternalBytecode.cpp
)

# Allow InternalBytecode to find its .inc file
target_include_directories(hermesInternalBytecode_obj
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

set(JS_COMPILER_FLAGS "-O" "-Wno-undefined-variable" "-g0")

add_custom_command(
  OUTPUT InternalBytecode.hbc
  COMMAND ${hermesc_EXE} ${JS_COMPILER_FLAGS} -emit-binary -out=${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.hbc ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.js
  DEPENDS ${hermesc_EXE} InternalBytecode.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# X.inc is the hex dump of the compiled bytecode artifact that can be included
# in the InternalBytecode.cpp as C/C++ hex literal to initialize a byte array.
add_custom_command(
  OUTPUT InternalBytecode.inc
  COMMAND ${Python_EXECUTABLE} xxd.py ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.hbc > ${CMAKE_CURRENT_BINARY_DIR}/InternalBytecode.inc
  DEPENDS xxd.py InternalBytecode.hbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_target(InternalBytecodeInclude ALL DEPENDS InternalBytecode.inc)

# Ensure hermesInternalBytecode_obj waits for InternalBytecodeInclude to complete.
# This prevents race conditions with Visual Studio where the .inc file might be
# accessed while still being generated.
add_dependencies(hermesInternalBytecode_obj InternalBytecodeInclude)

# Switch native version and bytecode version
if(HERMESVM_INTERNAL_JAVASCRIPT_NATIVE)
add_library(hermesInternalJavaScript_obj ALIAS hermesInternalUnit_obj)
else()
add_library(hermesInternalJavaScript_obj ALIAS hermesInternalBytecode_obj)
endif()
