set(source_files
  ConsoleHost.cpp
  MemorySizeParser.cpp
)

add_llvm_library(hermesConsoleHost STATIC ${source_files}
  LINK_LIBS hermesVMRuntime hermesInstrumentation
)
