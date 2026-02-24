add_hermes_library(gtest_main
  TestMain.cpp

  LINK_LIBS
  gtest

  LINK_OBJLIBS
  LLVHSupport # Depends on llvh::cl
  )
