# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Turn on EH and RTTI for NodeApiJsiTest
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
# For some reason (bug?) add_unittest() is clearing LLVM_REQUIRES_RTTI, so
# we need to set this one.
set(LLVM_ENABLE_RTTI ON)

add_hermes_unittest(NodeApiJsiTest
  NodeApiJsiTestFactory.cpp
  ${HERMES_JSI_DIR}/jsi/test/testlib.cpp
)

target_link_libraries(NodeApiJsiTest
  hermesNodeApiJsiRuntime
)

# copy hermes.dll next to the NodeApiJsiTest.exe
add_dependencies(NodeApiJsiTest libshared)
add_custom_command(TARGET NodeApiJsiTest POST_BUILD
  VERBATIM
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:libshared>
  $<TARGET_FILE_DIR:NodeApiJsiTest>)
