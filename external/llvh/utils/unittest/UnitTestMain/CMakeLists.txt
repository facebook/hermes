add_hermes_library(llvh-gtest_main
  TestMain.cpp

  LINK_LIBS
  llvh-gtest

  LINK_OBJLIBS
  LLVHSupport # Depends on llvh::cl
  )
