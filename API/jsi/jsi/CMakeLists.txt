set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jsi
        jsi.cpp)

include_directories(..)


set(jsi_compile_flags "")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  list(APPEND jsi_compile_flags "-Wno-non-virtual-dtor")
endif()
target_compile_options(jsi PUBLIC ${jsi_compile_flags})
