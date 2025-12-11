# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This CMakeLists.txt generates precompiled bytecode for JSI extensions.
# The bytecode is embedded as a C array and loaded at runtime.

if(IMPORT_HOST_COMPILERS)
  file(TO_CMAKE_PATH "${IMPORT_HOST_COMPILERS}" IMPORT_HOST_COMPILERS_CMAKE)
  include(${IMPORT_HOST_COMPILERS_CMAKE})
  set(hermesc_EXE imported-hermesc)
else()
  set(hermesc_EXE ${HERMES_TOOLS_OUTPUT_DIR}/hermesc)
endif()

# Concatenate all JS files into one source file for compilation.
# This way there is only one RuntimeModule made for them.
if(NOT CMAKE_HOST_WIN32)
  set(concatenate "cat")
else()
  set(concatenate "type")
endif()

# The concatenation order is specified by the numeric prefixes in the JS filenames.
file(GLOB extensions_js_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.js")
list(SORT extensions_js_sources)

# Convert path from cmake style (`/`) to native style (`\` on Win and `/` elsewhere) for Win.
set(extensions_js_sources_native_path "")
foreach(cmake_path IN LISTS extensions_js_sources)
  file(TO_NATIVE_PATH ${cmake_path} native_path)
  list(APPEND extensions_js_sources_native_path ${native_path})
endforeach()

add_custom_command(
  OUTPUT ExtensionsBytecode.js
  COMMAND ${concatenate} ${extensions_js_sources_native_path} > ${CMAKE_CURRENT_BINARY_DIR}/ExtensionsBytecode.js
  DEPENDS ${extensions_js_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

set(JS_COMPILER_FLAGS "-O" "-Wno-undefined-variable" "-g0")

add_custom_command(
  OUTPUT ExtensionsBytecode.hbc
  COMMAND ${hermesc_EXE} ${JS_COMPILER_FLAGS} -emit-binary -out=${CMAKE_CURRENT_BINARY_DIR}/ExtensionsBytecode.hbc ${CMAKE_CURRENT_BINARY_DIR}/ExtensionsBytecode.js
  DEPENDS ${hermesc_EXE} ExtensionsBytecode.js
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

# ExtensionsBytecode.inc is the hex dump of the compiled bytecode artifact that can be included
# in ExtensionsBytecode.cpp as C/C++ hex literal to initialize a byte array.
add_custom_command(
  OUTPUT ExtensionsBytecode.inc
  COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py ${CMAKE_CURRENT_BINARY_DIR}/ExtensionsBytecode.hbc > ${CMAKE_CURRENT_BINARY_DIR}/ExtensionsBytecode.inc
  DEPENDS ${PROJECT_SOURCE_DIR}/lib/InternalJavaScript/xxd.py ExtensionsBytecode.hbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_target(ExtensionsBytecodeInclude ALL DEPENDS ExtensionsBytecode.inc)
