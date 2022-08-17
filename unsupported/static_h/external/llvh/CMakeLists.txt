set(HERMES_LLVM True)
add_definitions(-DHERMES_LLVM)

set(LLVM_MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LLVM_MAIN_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(LLVM_SOURCE_DIR ${LLVM_MAIN_SRC_DIR})
set(LLVM_BINARY_DIR ${LLVM_MAIN_BIN_DIR})

set(PACKAGE_NAME "LLVH")
set(PACKAGE_STRING "LLVH 8.0.0svn")
set(PACKAGE_VERSION "8.0.0svn")

set(LLVM_ENABLE_THREADS 1)
if (LLVH_REVERSE_ITERATION)
  set(LLVM_ENABLE_REVERSE_ITERATION 1)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(config-ix)

configure_file(include/llvh/Config/config.h.cmake include/llvh/Config/config.h)
configure_file(include/llvh/Config/llvm-config.h.cmake include/llvh/Config/llvm-config.h)

# Some sources are not compatible with unity builds, so if unity builds are
# globally enabled, turn them off for LLVH.
set(CMAKE_UNITY_BUILD OFF)

add_subdirectory(lib)
add_subdirectory(utils/FileCheck)
add_subdirectory(utils/not)
add_subdirectory(utils/count)
add_subdirectory(utils/unittest)
