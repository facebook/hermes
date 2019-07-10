# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

add_subdirectory(hermes)
add_subdirectory(hermesc)
add_subdirectory(hdb)
add_subdirectory(hbcdump)
add_subdirectory(hvm)
add_subdirectory(hvm-bench)
add_subdirectory(repl)
add_subdirectory(hbc-diff)
add_subdirectory(hbc-deltaprep)
add_subdirectory(hbc-attribute)
add_subdirectory(jsi)

if (DEFINED LIBFUZZER_PATH)
  add_subdirectory(fuzzer)
  message(STATUS "Testing hermes with libfuzzer ${LIBFUZZER_PATH}")
else()
  message(STATUS "Testing hermes without libfuzzer")
endif()
