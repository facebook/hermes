include_directories(${HERMES_SOURCE_DIR}/API)
include_directories(${HERMES_SOURCE_DIR}/../jsi)
set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)
add_llvm_tool(hdb
  hdb.cpp
  ${ALL_HEADER_FILES}
  )

set_target_properties(hdb PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  )

target_link_libraries(hdb
  hermesapi
)
