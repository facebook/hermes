set(source_files
  Builtins.cpp
  InstDecode.cpp
  InstDecode2.cpp
)

add_llvm_library(hermesInst STATIC ${source_files}
  LINK_LIBS
  hermesSupport
)
