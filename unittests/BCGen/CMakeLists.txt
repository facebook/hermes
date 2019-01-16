set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

set(BCSources
  BytecodeFormConverterTest.cpp
  RATest.cpp
  SupportTest.cpp
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
