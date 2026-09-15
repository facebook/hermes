# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# CMake script to run Node-API CTS tests under Hermes.
# Invoked via cmake -P with the following variables:
#   HERMES          - path to the hermes binary
#   CTS_TESTS_DIR   - path to the CTS tests/js-native-api directory
#   IMPLEMENTOR_DIR - path to the hermes implementor directory
#   ADDON_BASE_DIR  - build-tree directory containing compiled .node addons
#   QEMU_RUN_PREFIX - optional command prefix for cross-compile runs
#                     (e.g., "qemu-arm -L /path/to/sysroot"); empty for native

# Split QEMU_RUN_PREFIX into argv tokens; empty list when not cross-compiling.
set(QEMU_PREFIX_LIST "")
if(QEMU_RUN_PREFIX)
  separate_arguments(QEMU_PREFIX_LIST UNIX_COMMAND "${QEMU_RUN_PREFIX}")
endif()

# Read harness files.
file(READ "${IMPLEMENTOR_DIR}/assert.js" ASSERT_JS)
file(READ "${IMPLEMENTOR_DIR}/load-addon.js" LOAD_ADDON_JS)

# Read skip list (if it exists).
set(SKIP_LIST "")
set(SKIP_LIST_FILE "${IMPLEMENTOR_DIR}/hermes-skip-list.txt")
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

# Find all test.js files.
file(GLOB_RECURSE TEST_FILES "${CTS_TESTS_DIR}/*/test.js")
list(SORT TEST_FILES)

set(PASS_COUNT 0)
set(FAIL_COUNT 0)
set(SKIP_COUNT 0)
set(FAILED_TESTS "")

foreach(TEST_FILE ${TEST_FILES})
  get_filename_component(TEST_DIR ${TEST_FILE} DIRECTORY)
  get_filename_component(TEST_NAME ${TEST_DIR} NAME)

  # Check if test is in the skip list.
  list(FIND SKIP_LIST "${TEST_NAME}" SKIP_INDEX)
  if(NOT SKIP_INDEX EQUAL -1)
    math(EXPR SKIP_COUNT "${SKIP_COUNT} + 1")
    message(STATUS "SKIP: ${TEST_NAME} (in skip list)")
    continue()
  endif()

  # Addon directory in the build tree.
  set(ADDON_DIR "${ADDON_BASE_DIR}/${TEST_NAME}")

  # Check that the addon .node file exists. If not, skip.
  if(NOT EXISTS "${ADDON_DIR}/${TEST_NAME}.node")
    math(EXPR SKIP_COUNT "${SKIP_COUNT} + 1")
    message(STATUS "SKIP: ${TEST_NAME} (addon not built)")
    continue()
  endif()

  # Read the test JS.
  file(READ "${TEST_FILE}" TEST_JS)

  # Create the combined script:
  #   1. Set __napiAddonDir to the build-tree addon directory
  #   2. Define assert()
  #   3. Define loadAddon()
  #   4. Run the test
  set(COMBINED_JS
    "var __napiAddonDir = '${ADDON_DIR}';\n${ASSERT_JS}\n${LOAD_ADDON_JS}\n${TEST_JS}"
  )

  # Write temp file to the build tree (not the source tree).
  set(TEMP_FILE "${ADDON_DIR}/_hermes_test_runner.js")
  file(WRITE "${TEMP_FILE}" "${COMBINED_JS}")

  # Run hermes. When cross-compiling, QEMU_PREFIX_LIST is prepended so the
  # cross-compiled hermes runs under qemu instead of being executed directly
  # by the host.
  execute_process(
    COMMAND ${QEMU_PREFIX_LIST} "${HERMES}" "${TEMP_FILE}"
    RESULT_VARIABLE EXIT_CODE
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE STDERR
    TIMEOUT 120
  )

  # Clean up temp file.
  file(REMOVE "${TEMP_FILE}")

  if(EXIT_CODE EQUAL 0)
    math(EXPR PASS_COUNT "${PASS_COUNT} + 1")
    message(STATUS "PASS: ${TEST_NAME}")
  else()
    math(EXPR FAIL_COUNT "${FAIL_COUNT} + 1")
    list(APPEND FAILED_TESTS "${TEST_NAME}")
    message(STATUS "FAIL: ${TEST_NAME}")
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
message(STATUS "=== Node-API CTS Results ===")
message(STATUS "Total: ${TOTAL}  Pass: ${PASS_COUNT}  Fail: ${FAIL_COUNT}  Skip: ${SKIP_COUNT}")

if(FAILED_TESTS)
  message(STATUS "")
  message(STATUS "Failed tests:")
  foreach(T ${FAILED_TESTS})
    message(STATUS "  - ${T}")
  endforeach()
  message(FATAL_ERROR "Some CTS tests failed")
endif()
