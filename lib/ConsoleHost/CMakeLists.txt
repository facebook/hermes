set(source_files
  ConsoleHost.cpp
  MemorySizeParser.cpp
  RandomSeedParser.cpp
)

add_llvm_library(hermesConsoleHost STATIC ${source_files}
  LINK_LIBS hermesVMRuntime hermesInstrumentation
)
