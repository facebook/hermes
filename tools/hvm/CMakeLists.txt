set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hvm
  hvm.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hvm
  hermesVMRuntime
  hermesConsoleHost
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
  dtoa
  ${CORE_FOUNDATION}
)

hermes_link_icu(hvm)

