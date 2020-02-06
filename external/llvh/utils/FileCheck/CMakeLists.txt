add_hermes_tool(FileCheck
  FileCheck.cpp
  )

target_link_libraries(FileCheck PRIVATE LLVHSupport)
