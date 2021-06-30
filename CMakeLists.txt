# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.7.0)

# Set the VERSION variables based on the project command
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

# find_package uses <PackageName>_ROOT variables.
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()
# Include file check macros honor CMAKE_REQUIRED_LIBRARIES.
if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

# Only interpret if() arguments as variables or keywords when unquoted.
# CMake emits a warning if this is not set.
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

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

# Allow reading the LOCATION property of a target to determine the eventual
# location of build targets. This is needed when building the debugging symbols
# bundles for Apple platforms.
if (POLICY CMP0026)
  cmake_policy(SET CMP0026 OLD)
endif()

# Has to be set before `project` as per documentation
# https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_SYSROOT.html
set(CMAKE_OSX_SYSROOT ${HERMES_APPLE_TARGET_PLATFORM})

if(HERMES_APPLE_TARGET_PLATFORM MATCHES "catalyst")
  set(CMAKE_OSX_SYSROOT "macosx")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-target x86_64-arm64-apple-ios14.0-macabi -isystem ${CMAKE_OSX_SYSROOT}/System/iOSSupport/usr/include")
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-target x86_64-arm64-apple-ios14.0-macabi -isystem ${CMAKE_OSX_SYSROOT}/System/iOSSupport/usr/include")
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

# This must be consistent with the release_version in:
# - android/build.gradle
# - npm/package.json
# - hermes-engine.podspec
project(Hermes
        VERSION 0.8.0
        LANGUAGES C CXX)
# Optional suffix like "-rc3"
set(VERSION_SUFFIX "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(LLVH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/llvh)

include(Hermes)
include(Lit)

set(HERMES_RELEASE_VERSION ${PROJECT_VERSION}${VERSION_SUFFIX})

include(FindPythonInterp)
if (NOT PYTHONINTERP_FOUND)
  message(FATAL_ERROR "Unable to find Python interpreter, required for builds and testing.
Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

# Project options.

set(HERMES_IS_ANDROID OFF CACHE BOOL
  "Building for Android")

set(HERMES_IS_MOBILE_BUILD ${HERMES_IS_ANDROID} CACHE BOOL
  "Building for a mobile device")

set(HERMESVM_GCKIND HADES
  CACHE STRING
  "HermesVM GC type: either NONCONTIG_GENERATIONAL, MALLOC, or HADES")

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

set(HERMESVM_SERIALIZE OFF CACHE BOOL
  "Enable heap serialization and deserialization")

CHECK_CXX_SOURCE_COMPILES(
        "int main() { void *p = &&label; goto *p; label: return 0; }"
        HAVE_COMPUTED_GOTO)
if(HAVE_COMPUTED_GOTO)
    set(DEFAULT_INTERPRETER_THREADING ON)
else()
    set(DEFAULT_INTERPRETER_THREADING OFF)
endif()

set(HERMESVM_INDIRECT_THREADING ${DEFAULT_INTERPRETER_THREADING} CACHE BOOL
  "Enable the indirect threaded interpreter")

set(HERMESVM_ALLOW_COMPRESSED_POINTERS ON CACHE BOOL
  "Enable compressed pointers. If this is on and the target is a 64-bit build, compressed pointers will be used.")

set(HERMESVM_ALLOW_HUGE_PAGES OFF CACHE BOOL
        "Enable huge pages to back the GC managed heap. Only useful on Linux.")

# Note that smaller heap segments will lower the maximum number of properties
# that can be stored in an object.
set(HERMESVM_HEAP_SEGMENT_SIZE_KB 4096
        CACHE STRING
        "Size of segments in the GC managed heap in KB. Must be a power of 2.")

set(HERMESVM_ALLOW_CONCURRENT_GC ON CACHE BOOL
        "Enable concurrency in the GC for 64-bit builds.")

set(HERMESVM_API_TRACE_ANDROID_REPLAY OFF CACHE BOOL
  "Simulate Android config on Linux in API tracing.")

# Hermes VM Handle sanitization (moving the heap after every alloc)
set(HERMESVM_SANITIZE_HANDLES OFF CACHE BOOL
  "Enable Handle sanitization")

set(HERMESVM_CRASH_TRACE OFF CACHE BOOL
  "Enable recording of instructions for crash debugging depending on VMExperiments")

# Enable Address Sanitizer
set(HERMES_ENABLE_ADDRESS_SANITIZER OFF CACHE BOOL
  "Enable -fsanitize=address")

# Enable Undefined Behavior Sanitizer
set(HERMES_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER OFF CACHE BOOL
  "Enable -fsanitize=undefined")

# Enable Thread Sanitizer
set(HERMES_ENABLE_THREAD_SANITIZER OFF CACHE BOOL
  "Enable -fsanitize=thread")

# Enable Trace PC Guard
set(HERMES_ENABLE_TRACE_PC_GUARD OFF CACHE BOOL
  "Enable -fsanitize-coverage=trace-pc-guard")

set(HERMES_ENABLE_CODE_COVERAGE OFF CACHE BOOL
  "Enable code coverage to be generated when running binaries")

set(HERMES_ENABLE_LIBFUZZER OFF CACHE BOOL
  "Enable libfuzzer")

set(HERMES_ENABLE_FUZZILLI OFF CACHE BOOL
  "Enable fuzzilli")

set(HERMES_ENABLE_TOOLS ON CACHE BOOL
  "Enable CLI tools")

# Enable bitcode
set(HERMES_ENABLE_BITCODE OFF CACHE BOOL
  "Include bitcode with the framework")

# Set linker flag for building the fuzzer
set(HERMES_FUZZING_FLAG "-fsanitize=fuzzer" CACHE STRING
  "Linker argument to link fuzz targets against a given fuzzer.")

# Build with -DHERMES_SLOW_DEBUG for debug builds
# This does not affect release builds
set(HERMES_SLOW_DEBUG ON CACHE BOOL
  "Enable slow checks in Debug builds")

# On CentOS:
#   sudo yum install zlib-static glibc-static ncurses-static readline-static
set(HERMES_STATIC_LINK OFF CACHE BOOL
  "Link Hermes statically. May only work on GNU/Linux.")

set(HERMES_USE_STATIC_ICU OFF CACHE BOOL
  "Force static linking of ICU. May only work on GNU/Linux.")

set(HERMES_ENABLE_DEBUGGER_DEFAULT OFF)
if (NOT HERMES_IS_MOBILE_BUILD)
    set(HERMES_ENABLE_DEBUGGER_DEFAULT ON)
endif()
set(HERMES_ENABLE_DEBUGGER ${HERMES_ENABLE_DEBUGGER_DEFAULT} CACHE BOOL
  "Build with debugger support")

set(HERMES_ENABLE_IR_INSTRUMENTATION OFF CACHE BOOL
    "Build IR instrumentation support")

set(HERMES_FACEBOOK_BUILD OFF CACHE BOOL
    "Build Facebook (rather than open-source) version of Hermes")

set(HERMESVM_EXCEPTION_ON_OOM OFF CACHE BOOL
    "GC Out-of-memory raises an exception, rather than causing a crash")

set(HERMESVM_PLATFORM_LOGGING OFF CACHE BOOL
    "hermesLog(...) is enabled, using the platform's logging mechanism")

set(HERMES_USE_FLOWPARSER OFF CACHE BOOL
  "Use libflowparser for parsing es6")

set(HERMES_ENABLE_WERROR OFF CACHE BOOL
  "Whether the build should have -Werror enabled")

set(HERMES_ENABLE_WIN10_ICU_FALLBACK ON CACHE BOOL
  "Whether to allow falling back on Win10 ICU")

set(HERMES_GITHUB_RESOURCE_DIR "" CACHE STRING
  "A directory with additional files to bundle in the GitHub release")

set(ANDROID_LINUX_PERF_PATH ""
  CACHE STRING
  "If buildling for Android, full path to <linux/perf_events.h>")

set(HERMES_MSVC_MP ON CACHE STRING
  "Enable /MP in MSVC for parallel builds")

set(EMSCRIPTEN_FASTCOMP OFF CACHE BOOL
  "Emscripten is using the fastcomp backend instead of the LLVM one")

set(HERMES_ENABLE_INTL OFF CACHE BOOL
  "Enable JS Intl support (WIP)")

set(HERMES_ENABLE_TEST_SUITE ON CACHE BOOL
  "Enable the test suite")

set(HERMES_BUILD_APPLE_FRAMEWORK ON CACHE BOOL
  "Whether to build the libhermes target as a framework bundle or dylib on Apple platforms")

set(HERMES_BUILD_APPLE_DSYM OFF CACHE BOOL
  "Whether to build a DWARF debugging symbols bundle")

if (HERMES_IS_ANDROID)
  set(HERMES_IS_MOBILE_BUILD TRUE)

  add_definitions(-DHERMES_PLATFORM_UNICODE=HERMES_PLATFORM_UNICODE_JAVA)

  # The toolchain passes -Wa,--noexecstack which is valid for compiling
  # but not for linking. Just ignore it.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")

  # JS developers aren't VM developers. Give them a faster build.
  set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g -DNDEBUG")

  # The release build can focus on size.
  set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")
endif()

if(HERMES_BUILD_APPLE_DSYM)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf")
endif()

if (HERMES_IS_MOBILE_BUILD)
  add_definitions(-DHERMES_IS_MOBILE_BUILD)
endif()

# Enable debug mode by default
if ((NOT GENERATOR_IS_MULTI_CONFIG) AND CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (HERMES_STATIC_LINK)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
  set(HERMES_USE_STATIC_ICU ON)
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Check if the linker supports --gc-sections
# We can't simply CHECK_CXX_COMPILER_FLAG("-Wl,--gc-sections" ..) because CMake
# will compile and link separately and only passes the flag during compilation.
set(OLD_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "--gc-sections")
CHECK_CXX_COMPILER_FLAG("" HAVE_GC_SECTIONS)
set(CMAKE_EXE_LINKER_FLAGS "${OLD_CMAKE_EXE_LINKER_FLAGS}")

if(HAVE_GC_SECTIONS)
  set(OPTIONAL_GC_SECTIONS "-Wl,--gc-sections")
else()
  set(OPTIONAL_GC_SECTIONS "")
endif()

# Make the HERMES_RELEASE_VERSION accessible for version printing in C++.
add_definitions(-DHERMES_RELEASE_VERSION="${HERMES_RELEASE_VERSION}")

if(HERMES_ENABLE_IR_INSTRUMENTATION)
    add_definitions(-DHERMES_ENABLE_IR_INSTRUMENTATION)
endif()

add_definitions(-DHERMESVM_GC_${HERMESVM_GCKIND})

set(HERMES_PROFILER_MODE_IN_LIT_TEST "NONE")
if(HERMESVM_PROFILER_OPCODE)
    add_definitions(-DHERMESVM_PROFILER_OPCODE)
    set(HERMES_PROFILER_MODE_IN_LIT_TEST "OPCODE")
endif()
if(HERMESVM_PROFILER_BB)
    add_definitions(-DHERMESVM_PROFILER_BB)
    set(HERMES_PROFILER_MODE_IN_LIT_TEST "BB")
endif()
if(HERMESVM_PROFILER_JSFUNCTION)
    add_definitions(-DHERMESVM_PROFILER_JSFUNCTION)
    set(HERMES_PROFILER_MODE_IN_LIT_TEST "SAMPLING")
endif()
if(HERMESVM_PROFILER_NATIVECALL)
    add_definitions(-DHERMESVM_PROFILER_NATIVECALL)
    set(HERMES_PROFILER_MODE_IN_LIT_TEST "EXTERN")
endif()
if(HERMESVM_SERIALIZE)
  add_definitions(-DHERMESVM_SERIALIZE)
endif()
if(HERMESVM_INDIRECT_THREADING)
    add_definitions(-DHERMESVM_INDIRECT_THREADING)
endif()
if(HERMESVM_ALLOW_COMPRESSED_POINTERS)
    add_definitions(-DHERMESVM_ALLOW_COMPRESSED_POINTERS)
endif()
if(HERMESVM_ALLOW_HUGE_PAGES)
    add_definitions(-DHERMESVM_ALLOW_HUGE_PAGES)
endif()
add_definitions(-DHERMESVM_HEAP_SEGMENT_SIZE_KB=${HERMESVM_HEAP_SEGMENT_SIZE_KB})
if(HERMESVM_ALLOW_CONCURRENT_GC)
    add_definitions(-DHERMESVM_ALLOW_CONCURRENT_GC)
endif()
if(HERMESVM_API_TRACE_ANDROID_REPLAY)
    add_definitions(-DHERMESVM_API_TRACE_ANDROID_REPLAY)
endif()
if(HERMESVM_SANITIZE_HANDLES)
    add_definitions(-DHERMESVM_SANITIZE_HANDLES)
endif()
if(HERMESVM_CRASH_TRACE)
    add_definitions(-DHERMESVM_CRASH_TRACE=1)
endif()
if (HERMES_ENABLE_ADDRESS_SANITIZER)
    append("-fsanitize=address" CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
    # GCC does not automatically link libpthread when using ASAN
    append("-lpthread" CMAKE_EXE_LINKER_FLAGS)
endif()
if (HERMES_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
    add_definitions(-DHERMES_UBSAN)
    # Do not enable the vptr sanitizer, as it requires RTTI.
    append("-fsanitize=undefined -fno-sanitize=vptr -fno-sanitize-recover=undefined" CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
endif()
if (HERMES_ENABLE_THREAD_SANITIZER)
    append("-fsanitize=thread" CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
endif()
if (HERMES_ENABLE_TRACE_PC_GUARD)
    append("-fsanitize-coverage=trace-pc-guard" CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
endif()
if (HERMES_ENABLE_CODE_COVERAGE)
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        append("-fprofile-instr-generate -fcoverage-mapping" CMAKE_CXX_FLAGS CMAKE_C_FLAGS)
    else()
        message(FATAL_ERROR "Code coverage flags not defined for this compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()
if(HERMES_FACEBOOK_BUILD)
    add_definitions(-DHERMES_FACEBOOK_BUILD)
endif()
if(HERMESVM_EXCEPTION_ON_OOM)
    set(HERMES_ENABLE_EH ON)
    add_definitions(-DHERMESVM_EXCEPTION_ON_OOM)
endif()
if(HERMESVM_PLATFORM_LOGGING)
    add_definitions(-DHERMESVM_PLATFORM_LOGGING)
endif()
if (NOT (ANDROID_LINUX_PERF_PATH STREQUAL ""))
  add_definitions(-DANDROID_LINUX_PERF_PATH="${ANDROID_LINUX_PERF_PATH}")
endif()

if (HERMES_ENABLE_INTL)
  if (HERMES_IS_ANDROID)
    add_definitions(-DHERMES_PLATFORM_INTL=HERMES_PLATFORM_INTL_ANDROID)
  else()
    add_definitions(-DHERMES_PLATFORM_INTL=HERMES_PLATFORM_INTL_DUMMY)
  endif()
endif()

if (HERMES_ENABLE_WERROR)
  # Turn all warnings into errors on GCC-compatible compilers.
  if (GCC_COMPATIBLE)
    append("-Werror" CMAKE_CXX_FLAGS CMAKE_C_FLAGS)
  endif()
endif()

# Collect all header files and add them to the IDE.
file(GLOB_RECURSE ALL_HEADER_FILES "*.h")

if(HERMES_SLOW_DEBUG)
  # Enable HERMES_SLOW_DEBUG in Debug mode
  set_property(DIRECTORY APPEND PROPERTY
      COMPILE_DEFINITIONS $<$<CONFIG:Debug>:HERMES_SLOW_DEBUG>)
endif()

if ((NOT GENERATOR_IS_MULTI_CONFIG) AND (CMAKE_BUILD_TYPE STREQUAL Debug))
    set(HERMES_ASSUMED_BUILD_MODE_IN_LIT_TEST "dbg")
else()
    set(HERMES_ASSUMED_BUILD_MODE_IN_LIT_TEST "opt")
endif()

if (NOT (GENERATOR_IS_MULTI_CONFIG OR CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_COMPILER_IS_GNUCXX))
    # Enable LTO if we are not multi config generator and not a DEBUG build
    # and not GCC
    # GCC currently fails to link Hermes with LTO (see t16557748)
    option(HERMES_ENABLE_LTO "Build Hermes with LTO" ON)
endif()

if (GCC_COMPATIBLE)
  # Don't export symbols unless we explicitly say so
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  # C4068 unknown pragma
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4068")
  # C4200 nonstandard extension used: zero-sized array in struct/union
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4200")
  # C4201 nonstandard extension used: nameless struct/union
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4201")
  # C4530 C++ exception handler used, but unwind semantics are not enabled
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4530")
  # Parallelize build
  if (HERMES_MSVC_MP)
    add_definitions( /MP )
  endif()
endif()

# Export a JSON file with the compilation commands that external tools can use
# to analyze the source code of the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Attempt to use system ICU first, if none specified.
# Don't need ICU on Apple systems.
if (APPLE)
  set(ICU_FOUND 1)
  set(ICU_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/icu_decls)
  set(ICU_LIBRARIES
    icucore
  )
  include_directories(${ICU_INCLUDE_DIRS})
elseif(EMSCRIPTEN)
  # Just ignore ICU on Emiscripten. For now.
  set(ICU_FOUND 1)
endif()

if (NOT ICU_FOUND)
  # Workaround: FindICU does not correctly recognize ICU include dir until
  # CMake 3.8.0.  https://github.com/Kitware/CMake/commit/cdf7e5d8
  list(APPEND icu_include_suffixes "include")

  set(CMAKE_FIND_LIBRARY_SUFFIXES_OLD "${CMAKE_FIND_LIBRARY_SUFFIXES}")
  if (HERMES_USE_STATIC_ICU)
    add_definitions(-DU_STATIC_IMPLEMENTATION)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()

  # FindICU uses ICU_ROOT variable as a hint
  # Include 'uc' twice for static libraries that depend on each other.
  find_global_package(ICU 52 COMPONENTS uc i18n data uc)

  set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES_OLD}")

  if (ICU_FOUND)
    foreach(LIB_FILE ${ICU_LIBRARIES})
      get_filename_component(LIB_DIR ${LIB_FILE} DIRECTORY)
      list(APPEND ICU_RPATH ${LIB_DIR})
    endforeach(LIB_FILE)
    list(REMOVE_DUPLICATES ICU_RPATH)
    message("icu dir: ${ICU_RPATH}")
    include_directories(${ICU_INCLUDE_DIRS})
  endif()
endif()

# ICU is available on Windows, but only since Windows 10 v1703.
# Therefore, use it only as fallback.
if (NOT ICU_FOUND AND HERMES_ENABLE_WIN10_ICU_FALLBACK AND
    WIN32 AND # Windows 32 or 64 bit
    # At least Windows 10 version 1703 (aka Creators Update)
    NOT ${CMAKE_SYSTEM_VERSION} VERSION_LESS "10.0.15063")
  add_definitions(-DUSE_WIN10_ICU)
  set(ICU_FOUND 1)
  set(ICU_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/icu_decls)
  set(ICU_LIBRARIES
    icuuc icuin
  )
  include_directories(${ICU_INCLUDE_DIRS})
  message("Using Windows 10 built-in ICU")
endif()

# If we have no ICU, then error out.
if (NOT HERMES_IS_ANDROID AND NOT ICU_FOUND)
  message(FATAL_ERROR "Unable to find ICU.")
endif()

# Declare a function that links ICU for the given target.
# This adds the correct -rpath link flag as necessary.
function(hermes_link_icu target_name)
  get_target_property(target_type ${target_name} TYPE)
  target_link_libraries(${target_name} PRIVATE ${ICU_LIBRARIES})

  if (HERMES_USE_STATIC_ICU)
    if ((NOT EMSCRIPTEN) AND target_type MATCHES "EXECUTABLE|STATIC_LIBRARY")
      target_link_libraries(${target_name} PRIVATE dl pthread)
    elseif(target_type MATCHES "MODULE_LIBRARY|SHARED_LIBRARY")
      message(WARNING "ICU cannot be statically linked against shared library target ${target_name}")
    endif()
  endif()

  if (ICU_RPATH)
    set_property(TARGET ${target_name} APPEND PROPERTY
                  INSTALL_RPATH ${ICU_RPATH})
    set_property(TARGET ${target_name} PROPERTY
                  BUILD_WITH_INSTALL_RPATH TRUE)
  endif()
endfunction()

if (APPLE)
  find_library(CORE_FOUNDATION CoreFoundation)
else()
  set(CORE_FOUNDATION "")
endif()

if (HERMES_USE_FLOWPARSER)
  if (CMAKE_SYSTEM_NAME STREQUAL Darwin AND NOT HERMES_BUILD_32_BITS)
    set(LIBFLOWPARSER ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/libflowparser-mac.a)
  elseif (CMAKE_SYSTEM_NAME STREQUAL Linux AND NOT HERMES_BUILD_32_BITS)
    set(LIBFLOWPARSER ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/libflowparser-linux.a)
  else()
    set(LIBFLOWPARSER "")
    set(HERMES_USE_FLOWPARSER OFF)
  endif()
endif()

if (HERMES_USE_FLOWPARSER)
  add_definitions(-DHERMES_USE_FLOWPARSER)
endif()

if (HERMES_ENABLE_DEBUGGER)
  add_definitions(-DHERMES_ENABLE_DEBUGGER)
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")

set(HERMES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HERMES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(EXISTS ${HERMES_SOURCE_DIR}/API/jsi)
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/API/jsi)
elseif(EXISTS ${FBSOURCE_DIR}/xplat/jsi)
  set(HERMES_JSI_DIR ${FBSOURCE_DIR}/xplat/jsi)
elseif(EXISTS ${HERMES_SOURCE_DIR}/../jsi)
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/../jsi)
else()
  message(FATAL_ERROR "Unable to find jsi.")
endif()

include_directories(
  external/llvh/include
  external/llvh/gen/include
  ${CMAKE_CURRENT_BINARY_DIR}/external/llvh/include
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/public
  ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external
  )

if(HERMES_IS_ANDROID)
  find_package(fbjni REQUIRED CONFIG)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/llvh)
add_subdirectory(utils/hermes-lit)
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(public)
add_subdirectory(external)
add_subdirectory(API)
add_subdirectory(android/intltest/java/com/facebook/hermes/test)

if(HERMES_ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

# Make sure JSI is compiled with PIC
set(save_CMAKE_POSITION_INDEPENDENT_CODE ${CMAKE_POSITION_INDEPENDENT_CODE})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(${HERMES_JSI_DIR}/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
set(CMAKE_POSITION_INDEPENDENT_CODE ${save_CMAKE_POSITION_INDEPENDENT_CODE})

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/facebook)
    add_subdirectory(facebook)
endif()

# Configure the test suites
#
if(HERMES_ENABLE_TEST_SUITE)
  if(NOT HERMES_ENABLE_TOOLS)
    message(FATAL_ERROR, "Running the test-suite requires the CLI tools to be built.")
  endif()

  add_subdirectory(unittests)

  list(APPEND HERMES_TEST_DEPS
    HermesUnitTests
    hermes
    hermesc
    hvm
    interp-dispatch-bench
    hdb
    hbcdump
    hbc-attribute
    hbc-deltaprep
    hbc-diff
    dependency-extractor
    node-hermes
    )

  set(coverage_directory "")
  if (HERMES_ENABLE_CODE_COVERAGE)
    set(coverage_directory ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/coverage)
  endif()

  set(HERMES_LIT_TEST_PARAMS
    test_exec_root=${HERMES_BINARY_DIR}/test
    unittests_dir=${HERMES_BINARY_DIR}/unittests
    debugger_enabled=${HERMES_ENABLE_DEBUGGER}
    use_flowparser=${HERMES_USE_FLOWPARSER}
    hbc_deltaprep=${HERMES_TOOLS_OUTPUT_DIR}/hbc-deltaprep
    dependency_extractor=${HERMES_TOOLS_OUTPUT_DIR}/dependency-extractor
    FileCheck=${HERMES_TOOLS_OUTPUT_DIR}//FileCheck
    hermes=${HERMES_TOOLS_OUTPUT_DIR}/hermes
    hermesc=${HERMES_TOOLS_OUTPUT_DIR}/hermesc
    hdb=${HERMES_TOOLS_OUTPUT_DIR}/hdb
    hbcdump=${HERMES_TOOLS_OUTPUT_DIR}/hbcdump
    hbc-deltaprep=${HERMES_TOOLS_OUTPUT_DIR}/hbc-deltaprep
    hbc_diff=${HERMES_TOOLS_OUTPUT_DIR}/hbc-diff
    node-hermes=${HERMES_TOOLS_OUTPUT_DIR}/node-hermes
    build_mode=${HERMES_ASSUMED_BUILD_MODE_IN_LIT_TEST}
    exception_on_oom_enabled=${HERMESVM_EXCEPTION_ON_OOM}
    serialize_enabled=${HERMESVM_SERIALIZE}
    profiler=${HERMES_PROFILER_MODE_IN_LIT_TEST}
    gc=${HERMESVM_GCKIND}
    ubsan=${HERMES_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER}
    coverage=${coverage_directory}
    )

  set(LLVH_LIT_ARGS "-sv")

  add_lit_testsuite(check-hermes "Running the Hermes regression tests"
    ${HERMES_SOURCE_DIR}/test
    ${HERMES_SOURCE_DIR}/unittests
    PARAMS ${HERMES_LIT_TEST_PARAMS}
    DEPENDS ${HERMES_TEST_DEPS}
    ARGS ${HERMES_TEST_EXTRA_ARGS}
    )
  set_target_properties(check-hermes PROPERTIES FOLDER "Hermes regression tests")
endif()

# This is how github release files are built.

set(HERMES_GITHUB_DIR ${HERMES_BINARY_DIR}/github)
string(TOLOWER ${CMAKE_SYSTEM_NAME} HERMES_GITHUB_SYSTEM_NAME)

if(HERMES_ENABLE_TOOLS)
  set(HERMES_CLI_GITHUB_FILE hermes-cli-${HERMES_GITHUB_SYSTEM_NAME}-v${HERMES_RELEASE_VERSION}.tar.gz)
  set(HERMES_GITHUB_BUNDLE_DIR ${HERMES_BINARY_DIR}/bundle)

  # If the github release should include extra files (like dlls)
  if (HERMES_GITHUB_RESOURCE_DIR STREQUAL "")
    set(HERMES_GITHUB_EXTRAS "")
  else()
    if (IS_DIRECTORY ${HERMES_GITHUB_RESOURCE_DIR})
      file(GLOB HERMES_GITHUB_EXTRAS "${HERMES_GITHUB_RESOURCE_DIR}/*")
    else()
      message(FATAL_ERROR "Extra resource dir not found: ${HERMES_GITHUB_RESOURCE_DIR}")
    endif()
  endif()

  # We need this as a separate target because Ninja doesn't run PRE_BUILD/PRE_LINKs in time
  add_custom_command(
    OUTPUT ${HERMES_GITHUB_BUNDLE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HERMES_GITHUB_BUNDLE_DIR})
  add_custom_target(make_bundle_dir DEPENDS ${HERMES_GITHUB_BUNDLE_DIR})

  add_custom_command(
    OUTPUT ${HERMES_GITHUB_DIR}/${HERMES_CLI_GITHUB_FILE}
    WORKING_DIRECTORY ${HERMES_GITHUB_BUNDLE_DIR}
    DEPENDS hermes hermesc hdb hbcdump make_bundle_dir
    VERBATIM
    COMMAND
      # We need bin/hermes or Release/bin/hermes.exe in a predictable location
      ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hermes> $<TARGET_FILE:hermesc> $<TARGET_FILE:hdb> $<TARGET_FILE:hbcdump> ${HERMES_GITHUB_EXTRAS} .
    COMMAND
      ${CMAKE_COMMAND} -E tar zcf ${HERMES_GITHUB_DIR}/${HERMES_CLI_GITHUB_FILE} .
  )

  add_custom_target(
    github-cli-release
    DEPENDS ${HERMES_GITHUB_DIR}/${HERMES_CLI_GITHUB_FILE})
endif()
