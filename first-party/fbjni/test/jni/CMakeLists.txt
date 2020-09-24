# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

enable_testing()
include(GoogleTest)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(TEST_COMPILE_OPTIONS
  "${FBJNI_COMPILE_OPTIONS}"
  -DFBJNI_DEBUG_REFS
  -UNDEBUG
)

# Set this globally for all test libraries.  It also seems to affect importing.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(inter_dso_exception_test_1 SHARED
  inter_dso_exception_test_1/Test.cpp
)
target_compile_options(inter_dso_exception_test_1 PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(inter_dso_exception_test_1
  fbjni
)

add_library(inter_dso_exception_test_2 SHARED
  inter_dso_exception_test_2/Test.cpp
)
target_compile_options(inter_dso_exception_test_2 PRIVATE ${TEST_COMPILE_OPTIONS})
target_include_directories(inter_dso_exception_test_2 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(inter_dso_exception_test_2
  fbjni
  inter_dso_exception_test_1
)

add_library(no_rtti SHARED
  no_rtti.cpp
)
target_compile_options(no_rtti PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(no_rtti
  fbjni
)

add_library(fbjni-tests SHARED
  byte_buffer_tests.cpp
  fbjni_onload.cpp
  fbjni_tests.cpp
  hybrid_tests.cpp
  iterator_tests.cpp
  primitive_array_tests.cpp
  readable_byte_channel_tests.cpp
)
target_compile_options(fbjni-tests PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(fbjni-tests
  fbjni
  inter_dso_exception_test_1
  inter_dso_exception_test_2
  no_rtti
)

add_library(doc_tests SHARED
  doc_tests.cpp
)
target_compile_options(doc_tests PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(doc_tests
  fbjni
)

add_executable(modified_utf8_test
  modified_utf8_test.cpp
)
target_compile_options(modified_utf8_test PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(modified_utf8_test
  fbjni
  gtest
  Threads::Threads
  ${CMAKE_DL_LIBS}
)
gtest_add_tests(TARGET modified_utf8_test)

add_executable(utf16toUTF8_test
  utf16toUTF8_test.cpp
)
target_compile_options(utf16toUTF8_test PRIVATE ${TEST_COMPILE_OPTIONS})
target_link_libraries(utf16toUTF8_test
  fbjni
  gtest
  Threads::Threads
  ${CMAKE_DL_LIBS}
)
gtest_add_tests(TARGET utf16toUTF8_test)
