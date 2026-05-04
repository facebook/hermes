# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Build CTS addons against Hermes NAPI headers and create a test
# target that runs them under the Hermes CLI.

set(CTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(CTS_TESTS_DIR ${CTS_ROOT}/tests/js-native-api)
set(HERMES_IMPLEMENTOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Function to build a CTS addon against Hermes NAPI headers.
# ADDON_NAME: name of the addon (e.g., 2_function_arguments)
# SRC: source file relative to the addon's test directory
function(add_hermes_cts_addon ADDON_NAME SRC)
  set(ADDON_SRC_DIR ${CTS_TESTS_DIR}/${ADDON_NAME})
  add_library(cts_${ADDON_NAME} SHARED ${ADDON_SRC_DIR}/${SRC})
  # Place .node files in the build tree (not the source tree).
  set(CTS_ADDON_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ADDON_NAME})
  set_target_properties(cts_${ADDON_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "${ADDON_NAME}"
    LIBRARY_OUTPUT_DIRECTORY ${CTS_ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CTS_ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CTS_ADDON_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CTS_ADDON_OUTPUT_DIR}
  )
  # Use Hermes vendored NAPI headers.
  target_include_directories(cts_${ADDON_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
  )
  # The CTS common.h / entry_point.h are in the tests/js-native-api dir.
  target_include_directories(cts_${ADDON_NAME} PRIVATE
    ${CTS_TESTS_DIR}
  )
  # Include path for <js_native_api.h> and <node_api.h>.
  target_include_directories(cts_${ADDON_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include/hermes/napi
  )
  target_compile_features(cts_${ADDON_NAME} PRIVATE cxx_std_17)
  # ADDON_NAME is used by entry_point.h's NAPI_MODULE macro.
  target_compile_definitions(cts_${ADDON_NAME} PRIVATE
    ADDON_NAME=${ADDON_NAME}
  )
  if(APPLE)
    # Resolve NAPI symbols from the host process at load time.
    set_target_properties(cts_${ADDON_NAME} PROPERTIES
      LINK_FLAGS "-undefined dynamic_lookup"
    )
  endif()
endfunction()

# --- CTS Addons ---
add_hermes_cts_addon(2_function_arguments 2_function_arguments.c)

# Collect all CTS addon targets for dependency tracking.
set(CTS_ADDON_TARGETS cts_2_function_arguments)

# --- Test runner ---
# Create a test target that runs each CTS test under Hermes.
# Each test is: cat harness_preamble test.js | hermes -
# The harness sets __napiAddonDir, defines assert(), and loadAddon().

add_custom_target(check-napi-cts
  DEPENDS hermes ${CTS_ADDON_TARGETS}
  COMMAND ${CMAKE_COMMAND}
    -DHERMES=$<TARGET_FILE:hermes>
    -DCTS_TESTS_DIR=${CTS_TESTS_DIR}
    -DIMPLEMENTOR_DIR=${HERMES_IMPLEMENTOR_DIR}
    -DADDON_BASE_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -P ${HERMES_IMPLEMENTOR_DIR}/run-cts-tests.cmake
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running Node-API CTS tests under Hermes"
)
