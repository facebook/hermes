# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Build Node.js NAPI test addons against Hermes NAPI headers and create
# a test target that runs them under the Hermes CLI.

set(NODE_TESTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(JS_NATIVE_API_DIR ${NODE_TESTS_ROOT}/js-native-api)
set(NODE_API_DIR ${NODE_TESTS_ROOT}/node-api)
set(HERMES_NODE_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Function to build a Node.js test addon against Hermes NAPI headers.
# Arguments:
#   TEST_CATEGORY - js-native-api or node-api
#   TEST_DIR_NAME - name of the test directory (e.g., test_number)
#   ADDON_NAME   - name of the addon (may differ from TEST_DIR_NAME)
#   SOURCES      - list of source files (relative to the test directory)
# Optional keyword arguments:
#   DEFINES      - extra compile definitions
function(add_node_test_addon TEST_CATEGORY TEST_DIR_NAME ADDON_NAME)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEFINES" ${ARGN})
  if(TEST_CATEGORY STREQUAL "js-native-api")
    set(TEST_SRC_DIR ${JS_NATIVE_API_DIR}/${TEST_DIR_NAME})
    set(COMMON_DIR ${JS_NATIVE_API_DIR})
  else()
    set(TEST_SRC_DIR ${NODE_API_DIR}/${TEST_DIR_NAME})
    set(COMMON_DIR ${NODE_API_DIR})
  endif()

  set(FULL_SOURCES "")
  foreach(SRC ${ARG_SOURCES})
    list(APPEND FULL_SOURCES ${TEST_SRC_DIR}/${SRC})
  endforeach()

  set(TARGET_NAME node_test_${TEST_CATEGORY}_${TEST_DIR_NAME}_${ADDON_NAME})
  add_library(${TARGET_NAME} SHARED ${FULL_SOURCES})
  # Place .node files in the build tree (not the source tree) to avoid
  # polluting the source directory, especially when Hermes is included
  # as a CMake sub-project.
  set(ADDON_OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${TEST_CATEGORY}/${TEST_DIR_NAME}/build/Release)
  set_target_properties(${TARGET_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "${ADDON_NAME}"
    LIBRARY_OUTPUT_DIRECTORY ${ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ADDON_OUTPUT_DIR}
  )
  # Hermes vendored NAPI headers.
  target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
  )
  # Test common headers (common.h, common-inl.h, entry_point.h).
  target_include_directories(${TARGET_NAME} PRIVATE
    ${COMMON_DIR}
  )
  # <js_native_api.h> and <node_api.h>.
  target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include/hermes/napi
  )
  target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
  # NODE_GYP_MODULE_NAME is used by entry_point.h's NAPI_MODULE macro.
  target_compile_definitions(${TARGET_NAME} PRIVATE
    NODE_GYP_MODULE_NAME=${ADDON_NAME}
  )
  if(ARG_DEFINES)
    target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_DEFINES})
  endif()
  if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES
      LINK_FLAGS "-undefined dynamic_lookup"
    )
  endif()
  # Track all addon targets for dependency tracking.
  set(NODE_TEST_ADDON_TARGETS ${NODE_TEST_ADDON_TARGETS} ${TARGET_NAME}
    PARENT_SCOPE)
endfunction()

set(NODE_TEST_ADDON_TARGETS "")

# --- js-native-api test addons ---

add_node_test_addon(js-native-api 2_function_arguments 2_function_arguments
  SOURCES 2_function_arguments.c)

add_node_test_addon(js-native-api 3_callbacks 3_callbacks
  SOURCES 3_callbacks.c)

add_node_test_addon(js-native-api 4_object_factory 4_object_factory
  SOURCES 4_object_factory.c)

add_node_test_addon(js-native-api 5_function_factory 5_function_factory
  SOURCES 5_function_factory.c)

add_node_test_addon(js-native-api 6_object_wrap myobject
  SOURCES myobject.cc)

add_node_test_addon(js-native-api 6_object_wrap myobject_basic_finalizer
  SOURCES myobject.cc
  DEFINES NAPI_EXPERIMENTAL)

add_node_test_addon(js-native-api 6_object_wrap nested_wrap
  SOURCES nested_wrap.cc
  DEFINES NAPI_VERSION=10)

add_node_test_addon(js-native-api 7_factory_wrap 7_factory_wrap
  SOURCES 7_factory_wrap.cc myobject.cc)

add_node_test_addon(js-native-api 8_passing_wrapped 8_passing_wrapped
  SOURCES 8_passing_wrapped.cc myobject.cc)

add_node_test_addon(js-native-api test_array test_array
  SOURCES test_array.c)

add_node_test_addon(js-native-api test_bigint test_bigint
  SOURCES test_bigint.c)

add_node_test_addon(js-native-api test_cannot_run_js test_cannot_run_js
  SOURCES test_cannot_run_js.c
  DEFINES NAPI_VERSION=10)

add_node_test_addon(js-native-api test_cannot_run_js test_pending_exception
  SOURCES test_cannot_run_js.c
  DEFINES NAPI_VERSION=9)

add_node_test_addon(js-native-api test_constructor test_constructor
  SOURCES test_constructor.c test_null.c)

add_node_test_addon(js-native-api test_conversions test_conversions
  SOURCES test_conversions.c test_null.c)

add_node_test_addon(js-native-api test_dataview test_dataview
  SOURCES test_dataview.c)

add_node_test_addon(js-native-api test_date test_date
  SOURCES test_date.c)

add_node_test_addon(js-native-api test_error test_error
  SOURCES test_error.c)

add_node_test_addon(js-native-api test_exception test_exception
  SOURCES test_exception.c)

# test_finalizer: uses NAPI_EXPERIMENTAL APIs (node_api_post_finalizer)
# not available in our vendored headers - cannot compile.
# add_node_test_addon(js-native-api test_finalizer test_finalizer
#   SOURCES test_finalizer.c
#   DEFINES NAPI_EXPERIMENTAL)

add_node_test_addon(js-native-api test_function test_function
  SOURCES test_function.c)

add_node_test_addon(js-native-api test_general test_general
  SOURCES test_general.c)

add_node_test_addon(js-native-api test_handle_scope test_handle_scope
  SOURCES test_handle_scope.c)

add_node_test_addon(js-native-api test_instance_data test_instance_data
  SOURCES test_instance_data.c)

add_node_test_addon(js-native-api test_new_target test_new_target
  SOURCES test_new_target.c)

add_node_test_addon(js-native-api test_number test_number
  SOURCES test_number.c test_null.c)

# test_object: uses NAPI_EXPERIMENTAL APIs (napi_create_object_with_properties)
# not available in our vendored headers - cannot compile.
# add_node_test_addon(js-native-api test_object test_object
#   SOURCES test_object.c test_null.c
#   DEFINES NAPI_EXPERIMENTAL)

add_node_test_addon(js-native-api test_object test_exceptions
  SOURCES test_exceptions.c)

add_node_test_addon(js-native-api test_promise test_promise
  SOURCES test_promise.c)

add_node_test_addon(js-native-api test_properties test_properties
  SOURCES test_properties.c)

add_node_test_addon(js-native-api test_reference test_reference
  SOURCES test_reference.c)

add_node_test_addon(js-native-api test_reference test_finalizer
  SOURCES test_finalizer.c)

add_node_test_addon(js-native-api test_reference_double_free
  test_reference_double_free
  SOURCES test_reference_double_free.c)

# test_sharedarraybuffer: uses NAPI_EXPERIMENTAL APIs (node_api_is_sharedarraybuffer)
# not available in our vendored headers - cannot compile.
# add_node_test_addon(js-native-api test_sharedarraybuffer test_sharedarraybuffer
#   SOURCES test_sharedarraybuffer.c)

add_node_test_addon(js-native-api test_string test_string
  SOURCES test_string.c test_null.c
  DEFINES NAPI_VERSION=10)

add_node_test_addon(js-native-api test_symbol test_symbol
  SOURCES test_symbol.c)

add_node_test_addon(js-native-api test_typedarray test_typedarray
  SOURCES test_typedarray.c)

# --- node-api test addons ---

add_node_test_addon(node-api 1_hello_world binding
  SOURCES binding.c)

add_node_test_addon(node-api test_buffer test_buffer
  SOURCES test_buffer.c
  DEFINES NAPI_VERSION=10)

add_node_test_addon(node-api test_buffer test_finalizer_buf
  SOURCES test_finalizer.c)

add_node_test_addon(node-api test_exception test_exception
  SOURCES test_exception.c)

add_node_test_addon(node-api test_general test_general
  SOURCES test_general.c)

add_node_test_addon(node-api test_init_order test_init_order
  SOURCES test_init_order.cc)

# test_instance_data (node-api): uses <uv.h> which we don't have.
# add_node_test_addon(node-api test_instance_data test_instance_data_nodeapi
#   SOURCES test_instance_data.c)

add_node_test_addon(node-api test_null_init test_null_init
  SOURCES test_null_init.c)

add_node_test_addon(node-api test_reference_by_node_api_version
  test_reference_all_types
  SOURCES test_reference_by_node_api_version.c
  DEFINES NAPI_VERSION=10)

add_node_test_addon(node-api test_reference_by_node_api_version
  test_reference_obj_only
  SOURCES test_reference_by_node_api_version.c
  DEFINES NAPI_VERSION=9)

# --- Test runner ---
add_custom_target(check-napi-node-tests
  DEPENDS hermes ${NODE_TEST_ADDON_TARGETS}
  COMMAND ${CMAKE_COMMAND}
    -DHERMES=$<TARGET_FILE:hermes>
    -DJS_NATIVE_API_DIR=${JS_NATIVE_API_DIR}
    -DNODE_API_DIR=${NODE_API_DIR}
    -DHARNESS_DIR=${HERMES_NODE_TESTS_DIR}
    -DADDON_BASE_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -P ${HERMES_NODE_TESTS_DIR}/run-node-tests.cmake
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running Node.js NAPI tests under Hermes"
)
