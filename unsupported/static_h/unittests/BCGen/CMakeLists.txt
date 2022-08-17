# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

set(BCSources
  BytecodeFileFormatTest.cpp
  BytecodeFormConverterTest.cpp
  RATest.cpp
  SupportTest.cpp
  StringKindTest.cpp
  TestHelpers.cpp
  HBC.cpp
  DebugInfo.cpp
  )

add_hermes_unittest(HermesBCGenTests
  ${BCSources}
  )

target_link_libraries(HermesBCGenTests
 hermesHBCBackend
 hermesBackend
 hermesFrontend
 hermesSupport
 hermesOptimizer
 hermesVMRuntime
 dtoa
 )
