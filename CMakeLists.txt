cmake_minimum_required(VERSION 2.8.8)

include(CheckCXXSourceCompiles)

# The hermes project is built using CMake and the LLVM's build system.
# It expects LLVM_SRC_DIR and LLVM_BUILD_DIR to be set. For example:
#
# cmake -G Ninja -DLLVM_BUILD_DIR=$XXX -DLLVM_SRC_DIR=$YYY
#
list(APPEND CMAKE_MODULE_PATH "${LLVM_BUILD_DIR}/lib/cmake/llvm/")

# Enable debug mode by default
if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

CHECK_CXX_SOURCE_COMPILES(
  "int main() { void *p = &&label; goto *p; label: return 0; }"
  HAVE_COMPUTED_GOTO)

if(HAVE_COMPUTED_GOTO)
  set(DEFAULT_INTERPRETER_THREADING ON)
else()
  set(DEFAULT_INTERPRETER_THREADING OFF)
endif()
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(DEFAULT_GCCELL_ID ON)
else()
  set(DEFAULT_GCCELL_ID OFF)
endif()

# Project options.
set(HERMESVM_GCKIND GENERATIONAL
  CACHE STRING
  "HermesVM GC type: either GENERATIONAL, NONCONTIG_GENERATIONAL, or MALLOC")
set(HERMESVM_GC_GENERATIONAL_MARKSWEEPCOMPACT OFF
  CACHE BOOL
  "HermesVM GC: only allocate into the old generation, mimicking a Mark-Sweep-Compact collector")

# Hermes VM opcode stats profiling
set(HERMESVM_PROFILER_OPCODE OFF CACHE BOOL
  "Enable opcode stats profiling in hermes VM")

# Hermes VM basic block profiling
set(HERMESVM_PROFILER_BB OFF CACHE BOOL
  "Enable basic block profiling in hermes VM")

# Hermes VM JS Function profiling
set(HERMESVM_PROFILER_JSFUNCTION OFF CACHE BOOL
  "Enable JS Function profiling in hermes VM")

# Hermes VM native call profiling
set(HERMESVM_PROFILER_NATIVECALL OFF CACHE BOOL
  "Enable native call profiling in hermes VM")

set(HERMESVM_INDIRECT_THREADING ${DEFAULT_INTERPRETER_THREADING} CACHE BOOL
  "Enable the indirect threaded interpreter")

set(HERMESVM_GCCELL_ID ${DEFAULT_GCCELL_ID} CACHE BOOL
  "Include a 64-bit id in every GCCell")

# Hermes VM Handle sanitization (moving the heap after every alloc)
set(HERMESVM_SANITIZE_HANDLES OFF CACHE BOOL
  "Enable Handle sanitization")

set(HERMES_FACEBOOK_BUILD OFF CACHE BOOL
    "Build Facebook (rather than open-source) version of Hermes")

set(HERMESVM_JIT OFF CACHE BOOL
  "Enable the JIT")

set(HERMESVM_JIT_DISASSEMBLER OFF CACHE BOOL
  "Enable the JIT disassembler")

set(HERMESVM_SYNTH_REPLAY OFF CACHE BOOL
  "Enable native JSI function recording")

set(HERMES_USE_FLOWPARSER OFF CACHE BOOL
  "Use libflowparser for parsing es6")

set(ANDROID_LINUX_PERF_PATH ""
  CACHE STRING
  "If buildling for Android, full path to <linux/perf_events.h>")

add_definitions(-DHERMESVM_GC_${HERMESVM_GCKIND})
if(HERMESVM_GC_GENERATIONAL_MARKSWEEPCOMPACT)
    add_definitions(-DHERMESVM_GC_GENERATIONAL_MARKSWEEPCOMPACT)
endif()
if(HERMESVM_PROFILER_OPCODE)
    add_definitions(-DHERMESVM_PROFILER_OPCODE)
endif()
if(HERMESVM_PROFILER_BB)
    add_definitions(-DHERMESVM_PROFILER_BB)
endif()
if(HERMESVM_PROFILER_JSFUNCTION)
    add_definitions(-DHERMESVM_PROFILER_JSFUNCTION)
endif()
if(HERMESVM_PROFILER_NATIVECALL)
    add_definitions(-DHERMESVM_PROFILER_NATIVECALL)
endif()
if(HERMESVM_INDIRECT_THREADING)
    add_definitions(-DHERMESVM_INDIRECT_THREADING)
endif()
if(HERMESVM_GCCELL_ID)
    add_definitions(-DHERMESVM_GCCELL_ID)
endif()
if(HERMESVM_SANITIZE_HANDLES)
    add_definitions(-DHERMESVM_SANITIZE_HANDLES)
endif()
if(HERMES_FACEBOOK_BUILD)
    add_definitions(-DHERMES_FACEBOOK_BUILD)
endif()
if(HERMESVM_JIT)
  add_definitions(-DHERMESVM_JIT)
endif()
if(HERMESVM_JIT_DISASSEMBLER)
  add_definitions(-DHERMESVM_JIT_DISASSEMBLER)
endif()
if(HERMESVM_SYNTH_REPLAY)
  add_definitions(-DHERMESVM_SYNTH_REPLAY)
endif()
if (NOT (ANDROID_LINUX_PERF_PATH STREQUAL ""))
  add_definitions(-DANDROID_LINUX_PERF_PATH="${ANDROID_LINUX_PERF_PATH}")
endif()

# Build the hermes compiler by default.
set(LLVM_BUILD_TOOLS ON)
# Enable warnings.
set(LLVM_ENABLE_WARNINGS ON)

# Turn all warnings into errors on some compilers:
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
set(LLVM_ENABLE_WERROR ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
set(LLVM_ENABLE_WERROR ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
# Warn.
endif()

# Collect all header files and add them to the IDE.
file(GLOB_RECURSE ALL_HEADER_FILES "*.h")

# Pick up a workaround for a CMake problem from LLVM r282552.
if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()
# Enable transitive library dependencies
if(POLICY CMP0022)
    cmake_policy(SET CMP0022 NEW)
endif()

# Don't complain about mixing plain and keyword target_link_libraries commands.
# Keyword style is when you specify whether library symbols are re-exported,
# e.g. target_link_libraries(target PRIVATE lib).
# LLVM currently uses plain-style target_link_libraries calls so we must
# allow mixing.
if (POLICY CMP0023)
  cmake_policy(SET CMP0023 OLD)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    # Enable LLVM assertions in debug mode
    option(LLVM_ENABLE_ASSERTIONS "Enable assertions" ON)
    # Enable HERMES_SLOW_DEBUG in debug mode
    add_definitions(-DHERMES_SLOW_DEBUG)
    set(HERMES_BUILD_MODE "dbg")
else()
    set(HERMES_BUILD_MODE "opt")
endif()

if (NOT (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_COMPILER_IS_GNUCXX))
    # Enable LTO if we are not a DEBUG build and not GCC
    # GCC currently fails to link Hermes with LTO (see t16557748)
    option(LLVM_ENABLE_LTO "Build Hermes with LTO" ON)
endif()

# Superss warnings about unknown warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option")
# Suppress an uninteresting warning about C++17 name mangling changes.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")

# Export a JSON file with the compilation commands that external tools can use
# to analyze the source code of the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LLVM_MAIN_SRC_DIR ${LLVM_SRC_DIR})
include(LLVMConfig)
include(AddLLVM)
include(HandleLLVMOptions)

include_directories("${LLVM_SRC_DIR}/include"
                    "${LLVM_BUILD_DIR}/include")

link_directories("${LLVM_SRC_DIR}/lib"
                 "${LLVM_BUILD_DIR}/${CMAKE_CFG_INTDIR}/lib")

# Attempt to use system ICU first, if none specified.
# Don't need ICU on Apple systems.
if (NOT ICU_PATH AND NOT APPLE)
  find_package(ICU 52 REQUIRED COMPONENTS uc i18n)
elseif (ICU_PATH)
  set(ICU_FOUND 1)
  set(ICU_INCLUDE_DIRS ${ICU_PATH}/include)
  set(ICU_LIBRARIES
    ${ICU_PATH}/lib/libicuuc.so
    ${ICU_PATH}/lib/libicui18n.so
    ${ICU_PATH}/lib/libicudata.so
  )
  set(ICU_RPATH ${ICU_PATH}/lib)
  include_directories(${ICU_INCLUDE_DIRS})
else() # Apple
  set(ICU_FOUND 1)
  set(ICU_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/icu_decls)
  set(ICU_LIBRARIES
    icucore
  )
  include_directories(${ICU_INCLUDE_DIRS})
endif()

# If we have no ICU, then error out.
if (NOT ICU_FOUND)
  message(FATAL_ERROR "Unable to find ICU.")
endif()

# Declare a function that links ICU for the given target.
# This adds the correct -rpath link flag as necessary.
function(hermes_link_icu target_name)
  target_link_libraries(${target_name} PRIVATE ${ICU_LIBRARIES})
  if (ICU_RPATH)
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                  LINK_FLAGS " -Wl,-rpath,${ICU_RPATH} ")
  endif()
endfunction()

if (APPLE)
  find_library(CORE_FOUNDATION CoreFoundation)
else()
  set(CORE_FOUNDATION "")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Darwin AND NOT LLVM_BUILD_32_BITS)
  set(LIBFLOWPARSER ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/libflowparser-mac.a)
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux AND NOT LLVM_BUILD_32_BITS)
  set(LIBFLOWPARSER ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/libflowparser-linux.a)
else()
  set(LIBFLOWPARSER "")
  set(HERMES_USE_FLOWPARSER OFF)
endif()

if (HERMES_USE_FLOWPARSER)
  add_definitions(-DHERMES_USE_FLOWPARSER)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(HERMES_ENABLE_DEBUGGER 1)
else()
  set(HERMES_ENABLE_DEBUGGER "")
endif()

if (${HERMES_ENABLE_DEBUGGER})
  add_definitions(-DHERMES_ENABLE_DEBUGGER)
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

set(HERMES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HERMES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})


set(LLVM_RUNTIME_OUTPUT_INTDIR ${HERMES_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)

if(EXISTS ${HERMES_SOURCE_DIR}/API/jsi)
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/API/jsi)
else()
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/../jsi)
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/public
  ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external
  )

add_subdirectory(tools)
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(external)
add_subdirectory(unittests)
add_subdirectory(${HERMES_JSI_DIR}/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
add_subdirectory(API)

# Configure the test suites
#
list(APPEND HERMES_TEST_DEPS
  HermesUnitTests
  hermes
  hermesc
  hvm
  hdb
  hermes-repl
  hbc-deltaprep
  llvm-config
  )

set(HERMES_TEST_PARAMS
  test_exec_root=${HERMES_BINARY_DIR}/test
  unittests_dir=${HERMES_BINARY_DIR}/unittests
  debugger_enabled=${HERMES_ENABLE_DEBUGGER}
  use_flowparser=${HERMES_USE_FLOWPARSER}
  jit_enabled=${HERMESVM_JIT}
  jit_disassembler_enabled=${HERMESVM_JIT_DISASSEMBLER}
  hbc_deltaprep=${HERMES_BINARY_DIR}/bin/hbc-deltaprep
  FileCheck=${LLVM_BUILD_DIR}/bin/FileCheck
  hermes=${HERMES_BINARY_DIR}/bin/hermes
  hermesc=${HERMES_BINARY_DIR}/bin/hermesc
  hdb=${HERMES_BINARY_DIR}/bin/hdb
  repl=${HERMES_BINARY_DIR}/bin/hermes-repl
  hbc-deltaprep=${HERMES_BINARY_DIR}/bin/hbc-deltaprep
  build_mode=${HERMES_BUILD_MODE}
  )

set(LLVM_LIT_ARGS "-sv")

add_lit_testsuite(check-hermes "Running the Hermes regression tests"
  ${HERMES_SOURCE_DIR}/test
  ${HERMES_SOURCE_DIR}/unittests
  PARAMS ${HERMES_TEST_PARAMS}
  DEPENDS ${HERMES_TEST_DEPS}
  ARGS ${HERMES_TEST_EXTRA_ARGS}
  )
set_target_properties(check-hermes PROPERTIES FOLDER "Hermes regression tests")
