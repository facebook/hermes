# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# CMake script to run Node.js NAPI tests under Hermes.
# Invoked via cmake -P with the following variables:
#   HERMES           - path to the hermes binary
#   JS_NATIVE_API_DIR - path to the js-native-api test directory
#   NODE_API_DIR     - path to the node-api test directory
#   HARNESS_DIR      - path to the hermes harness directory
#   ADDON_BASE_DIR   - build-tree directory containing compiled .node addons

# Read harness files.
file(READ "${HARNESS_DIR}/assert.js" ASSERT_JS)
file(READ "${HARNESS_DIR}/common.js" COMMON_JS)
file(READ "${HARNESS_DIR}/require.js" REQUIRE_JS)

# Read skip list (if it exists).
set(SKIP_LIST "")
set(SKIP_LIST_FILE "${HARNESS_DIR}/hermes-skip-list.txt")
if(EXISTS "${SKIP_LIST_FILE}")
  file(STRINGS "${SKIP_LIST_FILE}" SKIP_LINES)
  foreach(LINE ${SKIP_LINES})
    # Strip comments (everything after '#') and whitespace.
    string(REGEX REPLACE "#.*" "" LINE "${LINE}")
    string(STRIP "${LINE}" LINE)
    if(NOT LINE STREQUAL "")
      list(APPEND SKIP_LIST "${LINE}")
    endif()
  endforeach()
endif()

# Collect all test JS files from both directories with category info.
# We tag each file with its category for skip list matching.
set(TEST_FILES "")
set(TEST_CATEGORIES "")

file(GLOB_RECURSE JSNAPI_TEST_FILES "${JS_NATIVE_API_DIR}/*/*.js")
foreach(F ${JSNAPI_TEST_FILES})
  get_filename_component(FNAME ${F} NAME)
  string(REGEX MATCH "^test" IS_TEST "${FNAME}")
  string(REGEX MATCH "^_hermes_" IS_GENERATED "${FNAME}")
  if(IS_TEST AND NOT IS_GENERATED)
    list(APPEND TEST_FILES ${F})
    list(APPEND TEST_CATEGORIES "js-native-api")
  endif()
endforeach()

file(GLOB_RECURSE NODEAPI_TEST_FILES "${NODE_API_DIR}/*/*.js")
foreach(F ${NODEAPI_TEST_FILES})
  get_filename_component(FNAME ${F} NAME)
  string(REGEX MATCH "^test" IS_TEST "${FNAME}")
  string(REGEX MATCH "^_hermes_" IS_GENERATED "${FNAME}")
  if(IS_TEST AND NOT IS_GENERATED)
    list(APPEND TEST_FILES ${F})
    list(APPEND TEST_CATEGORIES "node-api")
  endif()
endforeach()

# Sort test files (and keep categories in sync).
# CMake doesn't support sorting parallel lists, so we create sortable
# composite keys and then split them back.
set(SORTABLE "")
list(LENGTH TEST_FILES NUM_TESTS)
math(EXPR LAST "${NUM_TESTS} - 1")
foreach(I RANGE ${LAST})
  list(GET TEST_FILES ${I} F)
  list(GET TEST_CATEGORIES ${I} C)
  list(APPEND SORTABLE "${F}|${C}")
endforeach()
list(SORT SORTABLE)
set(TEST_FILES "")
set(TEST_CATEGORIES "")
foreach(ENTRY ${SORTABLE})
  string(REPLACE "|" ";" PARTS "${ENTRY}")
  list(GET PARTS 0 F)
  list(GET PARTS 1 C)
  list(APPEND TEST_FILES ${F})
  list(APPEND TEST_CATEGORIES ${C})
endforeach()

set(PASS_COUNT 0)
set(FAIL_COUNT 0)
set(SKIP_COUNT 0)
set(FAILED_TESTS "")

set(TEST_INDEX 0)
foreach(TEST_FILE ${TEST_FILES})
  list(GET TEST_CATEGORIES ${TEST_INDEX} TEST_CATEGORY)
  math(EXPR TEST_INDEX "${TEST_INDEX} + 1")

  get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
  get_filename_component(TEST_DIR_NAME ${TEST_DIR} NAME)
  get_filename_component(TEST_FNAME ${TEST_FILE} NAME)

  # Build a unique test ID: dir_name/filename (without .js).
  string(REGEX REPLACE "\\.js$" "" TEST_BASE "${TEST_FNAME}")
  set(TEST_ID "${TEST_DIR_NAME}/${TEST_BASE}")

  # Check if test is in the skip list.
  # Skip list entries can be:
  #   "dir_name"                    - skip all tests in dir (both categories)
  #   "dir_name/test_base"          - skip specific test file (both categories)
  #   "js-native-api/dir_name"      - skip all tests in dir (js-native-api only)
  #   "js-native-api/dir_name/test" - skip specific test (js-native-api only)
  #   "node-api/dir_name"           - skip all tests in dir (node-api only)
  #   "node-api/dir_name/test"      - skip specific test (node-api only)
  set(SKIP_FOUND FALSE)
  # Check category-qualified entries first (most specific).
  list(FIND SKIP_LIST "${TEST_CATEGORY}/${TEST_ID}" SKIP_IDX)
  if(NOT SKIP_IDX EQUAL -1)
    set(SKIP_FOUND TRUE)
  endif()
  if(NOT SKIP_FOUND)
    list(FIND SKIP_LIST "${TEST_CATEGORY}/${TEST_DIR_NAME}" SKIP_IDX)
    if(NOT SKIP_IDX EQUAL -1)
      set(SKIP_FOUND TRUE)
    endif()
  endif()
  # Then check unqualified entries (backwards compatible).
  if(NOT SKIP_FOUND)
    list(FIND SKIP_LIST "${TEST_ID}" SKIP_IDX)
    if(NOT SKIP_IDX EQUAL -1)
      set(SKIP_FOUND TRUE)
    endif()
  endif()
  if(NOT SKIP_FOUND)
    list(FIND SKIP_LIST "${TEST_DIR_NAME}" SKIP_IDX)
    if(NOT SKIP_IDX EQUAL -1)
      set(SKIP_FOUND TRUE)
    endif()
  endif()

  if(SKIP_FOUND)
    math(EXPR SKIP_COUNT "${SKIP_COUNT} + 1")
    message(STATUS "SKIP: ${TEST_CATEGORY}/${TEST_ID} (in skip list)")
    continue()
  endif()

  # Addon directory in the build tree.
  set(ADDON_DIR "${ADDON_BASE_DIR}/${TEST_CATEGORY}/${TEST_DIR_NAME}/build/Release")

  # Check that build/Release directory exists (addon was built).
  if(NOT EXISTS "${ADDON_DIR}")
    math(EXPR SKIP_COUNT "${SKIP_COUNT} + 1")
    message(STATUS "SKIP: ${TEST_CATEGORY}/${TEST_ID} (addon not built)")
    continue()
  endif()

  # Read the test JS.
  file(READ "${TEST_FILE}" TEST_JS)

  # Create the combined script:
  #   1. Set __testDir to the test directory
  #   2. Set __addonDir to the build-tree addon directory
  #   3. Define assert module (__assert)
  #   4. Define common module (__common, __gc)
  #   5. Define require() shim
  #   6. Run the test
  set(COMBINED_JS
    "var __testDir = '${TEST_DIR}';\nvar __addonDir = '${ADDON_DIR}';\n${ASSERT_JS}\n${COMMON_JS}\n${REQUIRE_JS}\n${TEST_JS}"
  )

  # Write temp file to the build tree (not the source tree).
  set(TEMP_FILE "${ADDON_BASE_DIR}/${TEST_CATEGORY}/${TEST_DIR_NAME}/_hermes_node_test_runner.js")
  file(WRITE "${TEMP_FILE}" "${COMBINED_JS}")

  # Detect 'use strict' directive at the top of the test source
  # (after leading comments and blank lines). When the harness prepends
  # code, the directive loses its top-of-script position, so we pass
  # -strict to hermes instead.
  set(STRICT_FLAG "")
  if(TEST_JS MATCHES "^([ \t]*(//[^\n]*)?\n)*[ \t]*['\"]use strict['\"]")
    set(STRICT_FLAG "-strict")
  endif()

  # Run hermes. HERMES_ARGS is an optional semicolon-separated list
  # of extra flags (e.g., -gc-sanitize-handles=1).
  execute_process(
    COMMAND "${HERMES}" ${HERMES_ARGS} ${STRICT_FLAG} "${TEMP_FILE}"
    RESULT_VARIABLE EXIT_CODE
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE STDERR
    TIMEOUT 30
  )

  # Clean up temp file.
  file(REMOVE "${TEMP_FILE}")

  if(EXIT_CODE EQUAL 0)
    math(EXPR PASS_COUNT "${PASS_COUNT} + 1")
    message(STATUS "PASS: ${TEST_CATEGORY}/${TEST_ID}")
  else()
    math(EXPR FAIL_COUNT "${FAIL_COUNT} + 1")
    list(APPEND FAILED_TESTS "${TEST_CATEGORY}/${TEST_ID}")
    message(STATUS "FAIL: ${TEST_CATEGORY}/${TEST_ID}")
    if(STDERR)
      message(STATUS "  stderr: ${STDERR}")
    endif()
    if(STDOUT)
      message(STATUS "  stdout: ${STDOUT}")
    endif()
  endif()
endforeach()

# Summary.
math(EXPR TOTAL "${PASS_COUNT} + ${FAIL_COUNT} + ${SKIP_COUNT}")
message(STATUS "")
message(STATUS "=== Node.js NAPI Test Results ===")
message(STATUS "Total: ${TOTAL}  Pass: ${PASS_COUNT}  Fail: ${FAIL_COUNT}  Skip: ${SKIP_COUNT}")

if(FAILED_TESTS)
  message(STATUS "")
  message(STATUS "Failed tests:")
  foreach(T ${FAILED_TESTS})
    message(STATUS "  - ${T}")
  endforeach()
  message(FATAL_ERROR "Some Node.js NAPI tests failed")
endif()
