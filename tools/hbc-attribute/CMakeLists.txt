set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(hbc-attribute
  hbc-attribute.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(hbc-attribute
  hermesHBCBackend
)

hermes_link_icu(hbc-attribute)
