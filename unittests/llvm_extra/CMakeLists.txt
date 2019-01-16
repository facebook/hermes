set(LLVMExtraSources
  OutlinerTest.cpp
  SuffixTreeTest.cpp
  )

add_hermes_unittest(LLVMExtraTests
  ${LLVMExtraSources}
  )

target_link_libraries(LLVMExtraTests llvm_extra)
