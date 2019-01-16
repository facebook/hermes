set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

set(IRSources
  BasicBlockTest.cpp
  BuilderTest.cpp
  InstrTest.cpp
  IRUtilsTest.cpp
  IRVerifierTest.cpp
  LoopAnalysisTest.cpp
  ScopeAnalysisTest.cpp
  ScopeChainTest.cpp
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
