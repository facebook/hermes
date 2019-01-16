add_llvm_library(hermesCPPUtil
  STATIC
  CPPUtil.cpp
  LINK_LIBS hermesVMRuntime
)
