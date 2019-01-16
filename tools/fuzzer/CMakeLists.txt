set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(fuzzer
  fuzzer.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(fuzzer
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
  hermesVMRuntime
  dtoa
)

# link libFuzzer.a from the libfuzzer path.
target_link_libraries(fuzzer ${LIBFUZZER_PATH})
