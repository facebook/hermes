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
add_subdirectory(utils/TableGen)
add_subdirectory(utils/not)
add_subdirectory(utils/count)
add_subdirectory(utils/unittest)

set(LLVH_LIT_ARGS "-sv")
set(LLVH_LIT_TEST_PARAMS
  test_exec_root=${CMAKE_CURRENT_BINARY_DIR}/test
  FileCheck=$<TARGET_FILE:FileCheck>
  llvh-tblgen=$<TARGET_FILE:llvh-tblgen>
  )
add_lit_testsuite(check-llvh "Running LLVH regression tests"
  ${CMAKE_CURRENT_SOURCE_DIR}/test
  PARAMS ${LLVH_LIT_TEST_PARAMS}
  DEPENDS llvh-tblgen FileCheck
  ARGS ${LLVH_TEST_EXTRA_ARGS}
  )
set_target_properties(check-llvh PROPERTIES FOLDER "LLVH regression tests")
