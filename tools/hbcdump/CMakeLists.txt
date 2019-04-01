set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hbcdump
  hbcdump.cpp
  HBCParser.cpp
  ProfileAnalyzer.cpp
  StructuredPrinter.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hbcdump
  hermesHBCBackend
  hermesSupport
)

hermes_link_icu(hbcdump)
