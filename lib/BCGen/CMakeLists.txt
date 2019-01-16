add_llvm_library(hermesBackend
  STATIC
  RegAlloc.cpp
  BCOpt.cpp
  Lowering.cpp
  Exceptions.cpp
  LINK_LIBS hermesFrontend hermesOptimizer
)

add_subdirectory(CPP)
add_subdirectory(HBC)
