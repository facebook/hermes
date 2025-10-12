# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_library(libshared SHARED
  hermes_api.cpp
)

target_link_libraries(libshared
  PUBLIC
    jsi
    nodeApi
  PRIVATE
    hermesNodeApi
    hermesParser
    compileJS
    traceInterpreter
    timerStats
    hermesvm_a
)
target_link_options(libshared PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

# Force the linker to pull every object from hermesNodeApi.a so the exported
# napi_* entry points remain visible from the shared hermes library.
if(APPLE)
  target_link_options(libshared PRIVATE
    "LINKER:-force_load,$<TARGET_FILE:hermesNodeApi>"
  )
elseif(MSVC)
  target_link_options(libshared PRIVATE
    "LINKER:/WHOLEARCHIVE:$<TARGET_FILE:hermesNodeApi>"
  )
else()
  target_link_options(libshared PRIVATE
    "LINKER:--whole-archive,$<TARGET_FILE:hermesNodeApi>,--no-whole-archive"
  )
endif()

set_target_properties(libshared PROPERTIES
  # To make sure that the resulting DLL name is hermes.dll
  OUTPUT_NAME hermes
)

# Registers new test module.
# The caller must provide a set of SOURCES files. It can also provide an
# optional set of DEFINES as the macro definitions such as the "NAPI_VERSION=10"
# used for the target.
function(add_node_api_module MODULE_TARGET)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "SOURCES;DEFINES")

  # Ensure unique module target name.
  get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

  if(NOT ${MODULE_TARGET} STREQUAL ${FOLDER_NAME})
    # Prefix the target name are with the test folder name to avoid duplicate
    # target names between different tests. It is only done when the module
    # target name is different from the test folder name since the folder name
    # is usually unique between tests.
    set(MODULE_TARGET "${FOLDER_NAME}_${MODULE_TARGET}")
  endif()

  add_library(${MODULE_TARGET} MODULE)
  target_sources(${MODULE_TARGET} PRIVATE ${ARG_SOURCES})
  target_link_libraries(${MODULE_TARGET} PRIVATE libshared)
  target_compile_definitions(${MODULE_TARGET} PRIVATE ${ARG_DEFINES})

  set(MODULE_OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/build/$<IF:$<CONFIG:Debug>,Debug,Release>)
  set_target_properties(${MODULE_TARGET} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    SUFFIX .node
    PREFIX ""
  )
endfunction()

add_subdirectory(test)

# Turn on EH and RTTI for APITests
set(HERMES_ENABLE_EH_RTTI ON)

# For some reason (bug?) add_unittest() is clearing LLVM_REQUIRES_RTTI, so
# we need to set this one.
set(LLVM_ENABLE_RTTI ON)

if(WIN32)
  set(CHILD_PROCESS_SRC child_process_windows.cpp)
  set(NODE_LITE_PLATFORM_SRC node_lite_windows.cpp)
else()
  set(CHILD_PROCESS_SRC child_process_posix.cpp)
  set(NODE_LITE_PLATFORM_SRC node_lite_posix.cpp)
endif()

add_executable(node_lite
  ${CHILD_PROCESS_SRC}
  child_process.h
  compat.h
  node_lite.cpp
  node_lite.h
  node_lite_hermes.cpp
  ${NODE_LITE_PLATFORM_SRC}
  string_utils.cpp
  string_utils.h
)
target_link_libraries(node_lite PRIVATE libshared)
if(UNIX AND NOT APPLE)
  target_link_libraries(node_lite PRIVATE dl)
endif()
target_include_directories(node_lite
  PRIVATE
    ../../API/hermes_node_api/node_api
)

target_compile_definitions(node_lite
  PRIVATE
    NODE_API_EXPERIMENTAL_NO_WARNING
)

target_include_directories(node_lite
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)

# copy hermes.dll next to the node_lite.exe
add_custom_command(TARGET node_lite POST_BUILD
  VERBATIM
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:libshared>
    $<TARGET_FILE_DIR:node_lite>
)

add_hermes_unittest(NodeApiTests
  ${CHILD_PROCESS_SRC}
  child_process.h
  string_utils.cpp
  string_utils.h
  test_basics.cpp
  test_main.cpp
  test_main.h
)

target_link_libraries(NodeApiTests libshared)

# transform test JS files to be Hermes-compatible
add_dependencies(NodeApiTests transformJSFiles node_lite)
