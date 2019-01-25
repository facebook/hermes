set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)
# For some reason (bug?) add_unittest() is clearing LLVM_REQUIRES_RTTI, so
# we need to set this one.
set(LLVM_ENABLE_RTTI ON)

add_hermes_unittest(APITests APITest.cpp APITestFactory.cpp APILeanTest.cpp DebuggerTest.cpp SynthTraceTest.cpp)

include_directories(${HERMES_SOURCE_DIR}/API)
include_directories(${HERMES_JSI_DIR})
target_link_libraries(APITests hermesapi compileJS)

