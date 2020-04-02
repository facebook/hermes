add_hermes_library(gtest_main
  TestMain.cpp

  LINK_LIBS
  gtest

  LINK_LIBS
  LLVHSupport # Depends on llvm::cl
  )
