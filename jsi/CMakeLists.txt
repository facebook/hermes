set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jsi
        jsi.cpp
        adapter.cpp)

include_directories(..)


target_compile_options(jsi PUBLIC -Wno-non-virtual-dtor)
