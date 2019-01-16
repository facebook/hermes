set(source_files
  RegexParser.cpp
  Executor.cpp
)

add_llvm_library(hermesRegex
    STATIC ${source_files}
    LINK_LIBS hermesPlatformUnicode
)
