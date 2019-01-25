set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${HERMES_JSI_DIR})

add_subdirectory(hermes)

