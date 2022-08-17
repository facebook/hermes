add_definitions(-DIEEE_8087 -DLong=int)
add_definitions(-DNO_HEX_FP)
add_definitions(-DNO_INFNAN_CHECK)
add_definitions(-DMULTIPLE_THREADS)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
        set(CMAKE_C_FLAGS
                "${CMAKE_C_FLAGS} -Wno-parentheses -Wno-unused-label -Wno-sign-compare -Wno-implicit-fallthrough")
endif()

add_hermes_library(dtoa STATIC
        dtoa.c
        g_fmt.c
        locks.cpp)
