set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hermesc
  hermesc.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hermesc
  hermesCompilerDriver
  hermesAST
  hermesCPPBackend
  hermesHBCBackend
  hermesBackend
  hermesInst
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
)

hermes_link_icu(hermesc)
