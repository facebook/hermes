# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(IRSources
  BasicBlockTest.cpp
  BuilderTest.cpp
  FunctionTest.cpp
  InstrTest.cpp
  IRUtilsTest.cpp
  IRVerifierTest.cpp
  LoopAnalysisTest.cpp
  ScopeAnalysisTest.cpp
#  ScopeChainTest.cpp
  )

add_hermes_unittest(HermesIRTests
  ${IRSources}
  )

target_link_libraries(HermesIRTests
 hermesFrontend
 hermesOptimizer
 hermesParser
 hermesSupport
 dtoa
 )
