add_hermes_executable(llvh-tblgen
  TableGen.cpp
  Example.cpp

  LINK_LIBS LLVHSupport LLVHTableGen
  )
