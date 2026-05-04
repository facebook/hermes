cmake_minimum_required(VERSION 3.15...3.31)
project(node-api-cts)

set(NODE_API_HEADERS_DIR ${PROJECT_SOURCE_DIR}/node_modules/node-api-headers)

if(NOT EXISTS ${NODE_API_HEADERS_DIR})
  message(FATAL_ERROR "Expected ${NODE_API_HEADERS_DIR} to exist")
endif()

if(MSVC)
  set(NODE_API_LIB ${PROJECT_BINARY_DIR}/node.lib)
  set(NODE_API_DEF ${NODE_API_HEADERS_DIR}/def/node_api.def)
  execute_process(COMMAND ${CMAKE_AR} /def:${NODE_API_DEF} /out:${NODE_API_LIB} ${CMAKE_STATIC_LINKER_FLAGS})
endif()

function(add_node_api_cts_addon ADDON_NAME SRC)
  add_library(${ADDON_NAME} SHARED ${SRC})
  set_target_properties(${ADDON_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    # Co-locate the output binary with the source file
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # (for MSVC)
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
  )
  if(APPLE)
    set_target_properties(${ADDON_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif()
  target_include_directories(${ADDON_NAME} PRIVATE ${NODE_API_HEADERS_DIR}/include)
  target_link_libraries(${ADDON_NAME} PRIVATE ${NODE_API_LIB})
  target_compile_features(${ADDON_NAME} PRIVATE cxx_std_17)
  target_compile_definitions(${ADDON_NAME} PRIVATE ADDON_NAME=${ADDON_NAME})
endfunction()

file(GLOB_RECURSE cmake_dirs RELATIVE ${CMAKE_SOURCE_DIR} tests/*/CMakeLists.txt)

foreach(cmake_file ${cmake_dirs})
  get_filename_component(subdir ${cmake_file} DIRECTORY)
  add_subdirectory(${subdir})
endforeach()
