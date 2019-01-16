set(source_files
  CompilerDriver.cpp
)

add_llvm_library(hermesCompilerDriver STATIC ${source_files}
  LINK_LIBS
  hermesAST
  hermesInst
  hermesCPPBackend
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesFlowParser
  hermesSupport
  zip
)
