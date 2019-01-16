set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hbc-deltaprep
  hbc-deltaprep.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hbc-deltaprep
  hermesVMRuntime
  hermesHBCBackend
  hermesSupport
)

hermes_link_icu(hbc-deltaprep)
