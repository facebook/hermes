# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.13.0)

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
        VERSION 0.12.0
        LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(LLVH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/llvh)

include(Hermes)
include(Lit)

# This is not a cache variable so that it is recomputed when the project
# version is updated.
if(NOT DEFINED HERMES_RELEASE_VERSION)
  set(HERMES_RELEASE_VERSION ${PROJECT_VERSION})
endif()

find_package(Python COMPONENTS Interpreter)
if (NOT Python_Interpreter_FOUND)
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
  "HermesVM GC type: either MALLOC or HADES")

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

if(APPLE OR WIN32)
    set(DEFAULT_CONTIGUOUS_HEAP OFF)
else()
    set(DEFAULT_CONTIGUOUS_HEAP ON)
endif()

set(HERMESVM_ALLOW_CONTIGUOUS_HEAP ${DEFAULT_CONTIGUOUS_HEAP} CACHE BOOL
  "If this is on and compressed pointers are used, the heap will be allocated in a contiguous 4GB region.")

set(HERMESVM_ALLOW_HUGE_PAGES OFF CACHE BOOL
        "Enable huge pages to back the GC managed heap. Only useful on Linux.")

# Note that smaller heap segments will lower the maximum number of properties
# that can be stored in an object.
set(HERMESVM_HEAP_SEGMENT_SIZE_KB 4096
        CACHE STRING
        "Size of segments in the GC managed heap in KB. Must be a power of 2.")

set(HERMESVM_ALLOW_CONCURRENT_GC ON CACHE BOOL
        "Enable concurrency in the GC for 64-bit builds.")

set(HERMESVM_ALLOW_INLINE_ASM ON CACHE BOOL
        "Allow the use of inline assembly in VM code.")

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
  "Enables code coverage to be collected from binaries. Coverage output will be placed in a subdirectory called \"coverage\" of the build directory.")

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

set(HERMES_UNICODE_LITE OFF CACHE BOOL
  "Enable to use internal no-op unicode functionality instead of relying on underlying system libraries")

set(HERMES_ENABLE_DEBUGGER ON CACHE BOOL
  "Build with debugger support")

set(HERMES_MEMORY_INSTRUMENTATION ${HERMES_ENABLE_DEBUGGER} CACHE BOOL
  "Build with memory instrumentation support")

set(HERMES_ENABLE_IR_INSTRUMENTATION OFF CACHE BOOL
    "Build IR instrumentation support")

set(HERMES_FACEBOOK_BUILD OFF CACHE BOOL
    "Build Facebook (rather than open-source) version of Hermes")

set(HERMESVM_EXCEPTION_ON_OOM OFF CACHE BOOL
    "GC Out-of-memory raises an exception, rather than causing a crash")

set(HERMESVM_PLATFORM_LOGGING OFF CACHE BOOL
    "hermesLog(...) is enabled, using the platform's logging mechanism")

set(HERMES_RUN_WASM OFF CACHE BOOL
    "Emit Asm.js/Wasm unsafe compiler intrinsics")

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

set(HERMES_BUILD_NODE_HERMES OFF CACHE BOOL "Whether to build node-hermes")

set(HERMES_BUILD_LEAN_LIBHERMES OFF CACHE BOOL "Exclude the Hermes compiler from libhermes.")

set(HERMES_BUILD_SHARED_JSI OFF CACHE BOOL "Build JSI as a shared library.")

if (HERMES_IS_ANDROID)
  add_definitions(-DHERMES_PLATFORM_UNICODE=HERMES_PLATFORM_UNICODE_JAVA)
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
  append("-static" CMAKE_EXE_LINKER_FLAGS)
  set(HERMES_USE_STATIC_ICU ON)
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Check if the linker supports --gc-sections and ICF.
# We can't simply CHECK_CXX_COMPILER_FLAG("-Wl,--gc-sections" ..) because CMake
# will compile and link separately and only passes the flag during compilation.
# TODO: Use check_linker_flag once we have CMake 3.18.
set(OLD_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${OLD_CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
CHECK_CXX_COMPILER_FLAG("" HAVE_GC_SECTIONS)
set(CMAKE_EXE_LINKER_FLAGS "${OLD_CMAKE_EXE_LINKER_FLAGS} -Wl,--icf=safe")
CHECK_CXX_COMPILER_FLAG("" HAVE_ICF)
set(CMAKE_EXE_LINKER_FLAGS "${OLD_CMAKE_EXE_LINKER_FLAGS}")

if(HAVE_GC_SECTIONS)
  add_flag_if_supported("-ffunction-sections" FUNCTION_SECTIONS)
  add_flag_if_supported("-fdata-sections" DATA_SECTIONS)
  list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:--gc-sections")
endif()

if(HAVE_ICF)
  add_flag_if_supported("-faddrsig" ADDRSIG)
  list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:--icf=safe")
endif()

# Make the HERMES_RELEASE_VERSION accessible for version printing in C++.
if(HERMES_RELEASE_VERSION)
    add_definitions(-DHERMES_RELEASE_VERSION="${HERMES_RELEASE_VERSION}")
endif()

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
if(HERMESVM_INDIRECT_THREADING)
    add_definitions(-DHERMESVM_INDIRECT_THREADING)
endif()
if(HERMESVM_ALLOW_COMPRESSED_POINTERS)
    add_definitions(-DHERMESVM_ALLOW_COMPRESSED_POINTERS)
endif()
if(HERMESVM_ALLOW_CONTIGUOUS_HEAP)
    add_definitions(-DHERMESVM_ALLOW_CONTIGUOUS_HEAP)
endif()
if(HERMESVM_ALLOW_HUGE_PAGES)
    add_definitions(-DHERMESVM_ALLOW_HUGE_PAGES)
endif()
add_definitions(-DHERMESVM_HEAP_SEGMENT_SIZE_KB=${HERMESVM_HEAP_SEGMENT_SIZE_KB})
if(HERMESVM_ALLOW_CONCURRENT_GC)
    add_definitions(-DHERMESVM_ALLOW_CONCURRENT_GC)
endif()
if(HERMESVM_ALLOW_INLINE_ASM)
    add_definitions(-DHERMESVM_ALLOW_INLINE_ASM)
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
if(HERMES_RUN_WASM)
    add_definitions(-DHERMES_RUN_WASM)
endif()
if (NOT (ANDROID_LINUX_PERF_PATH STREQUAL ""))
  add_definitions(-DANDROID_LINUX_PERF_PATH="${ANDROID_LINUX_PERF_PATH}")
endif()

if (HERMES_ENABLE_INTL)
  add_definitions(-DHERMES_ENABLE_INTL)
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
# Don't need ICU on Apple, Emscripten, and Android.
if (APPLE OR EMSCRIPTEN OR HERMES_IS_ANDROID)
  set(ICU_FOUND 1)
endif()

if (NOT ICU_FOUND)
  if (NOT ICU_ROOT)
    set(FB_ICU_ROOT_A "/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-5-glibc-2.23/9bc6787")
    set(FB_ICU_ROOT_B "/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-4.8.1-glibc-2.17/c3f970a/")
    if(EXISTS ${FB_ICU_ROOT_A})
      set(ICU_ROOT ${FB_ICU_ROOT_A})
    elseif(EXISTS ${FB_ICU_ROOT_B})
      set(ICU_ROOT ${FB_ICU_ROOT_B})
    endif()
  endif()
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
if (NOT ICU_FOUND)
  message(FATAL_ERROR "Unable to find ICU.")
endif()

# Declare a function that links ICU for the given target.
# This adds the correct -rpath link flag as necessary.
function(hermes_link_icu target_name)
  get_target_property(target_type ${target_name} TYPE)
  target_link_libraries(${target_name} ${ICU_LIBRARIES})

  if (HERMES_USE_STATIC_ICU)
    if ((NOT EMSCRIPTEN) AND target_type MATCHES "EXECUTABLE|STATIC_LIBRARY")
      target_link_libraries(${target_name} dl pthread)
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
  find_library(FOUNDATION Foundation)
else()
  set(CORE_FOUNDATION "")
  set(FOUNDATION "")
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

if (HERMES_MEMORY_INSTRUMENTATION)
  add_definitions(-DHERMES_MEMORY_INSTRUMENTATION)
endif()

# Disables the GCC 64-to-32 bit truncation diagnostic if the compiler supports
# -Wshorten-64-to-32.
check_cxx_compiler_flag("-Wshorten-64-to-32" CXX_SUPPORTS_SHORTEN_64_TO_32)
if (${CXX_SUPPORTS_SHORTEN_64_TO_32})
  add_definitions(-DHERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32)
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")

# JSI_DIR has always priority over other folders.
# It's used by React Native when building Hermes from source so
# the copy of JSI inside `react-native/ReactCommon/jsi` can be used.
if(JSI_DIR)
  if(EXISTS ${JSI_DIR})
    set(HERMES_JSI_DIR ${JSI_DIR})
  else()
    message(FATAL_ERROR "You specified a JSI directory with -DJSI_DIR but JSI can't be found there.")
  endif()
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/API/jsi)
  set(HERMES_JSI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API/jsi)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../jsi)
  set(HERMES_JSI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../jsi)
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
  ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external
  )

if(HERMES_IS_ANDROID)
  find_package(fbjni REQUIRED CONFIG)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/llvh)
add_subdirectory(utils/hermes-lit)
add_subdirectory(lib)
add_subdirectory(public)
add_subdirectory(external)
add_subdirectory(API)
add_subdirectory(android/intltest/java/com/facebook/hermes/test)
add_subdirectory(unsupported)

if(HERMES_ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

# Make sure JSI is compiled with PIC
set(save_CMAKE_POSITION_INDEPENDENT_CODE ${CMAKE_POSITION_INDEPENDENT_CODE})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(save_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS ${HERMES_BUILD_SHARED_JSI})
add_subdirectory(${HERMES_JSI_DIR}/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
set(BUILD_SHARED_LIBS ${save_BUILD_SHARED_LIBS})
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
    )

  if(HERMES_BUILD_NODE_HERMES)
    list(APPEND HERMES_TEST_DEPS node-hermes)
  endif()

  set(coverage_directory "")
  if (HERMES_ENABLE_CODE_COVERAGE)
    set(coverage_directory ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/coverage)
  endif()

  set(HERMES_LIT_TEST_PARAMS_BASE
    test_exec_root=${CMAKE_CURRENT_BINARY_DIR}/test
    unittests_dir=${CMAKE_CURRENT_BINARY_DIR}/unittests
    debugger_enabled=${HERMES_ENABLE_DEBUGGER}
    intl_enabled=${HERMES_ENABLE_INTL}
    use_flowparser=${HERMES_USE_FLOWPARSER}
    hbc_deltaprep=${HERMES_TOOLS_OUTPUT_DIR}/hbc-deltaprep
    dependency_extractor=${HERMES_TOOLS_OUTPUT_DIR}/dependency-extractor
    FileCheck=${HERMES_TOOLS_OUTPUT_DIR}/FileCheck
    hermes=${HERMES_TOOLS_OUTPUT_DIR}/hermes
    hermesc=${HERMES_TOOLS_OUTPUT_DIR}/hermesc
    hdb=${HERMES_TOOLS_OUTPUT_DIR}/hdb
    hbcdump=${HERMES_TOOLS_OUTPUT_DIR}/hbcdump
    hbc-deltaprep=${HERMES_TOOLS_OUTPUT_DIR}/hbc-deltaprep
    hbc_diff=${HERMES_TOOLS_OUTPUT_DIR}/hbc-diff
    build_mode=${HERMES_ASSUMED_BUILD_MODE_IN_LIT_TEST}
    exception_on_oom_enabled=${HERMESVM_EXCEPTION_ON_OOM}
    node_hermes_enabled_flag=${HERMES_BUILD_NODE_HERMES}
    node-hermes=${HERMES_TOOLS_OUTPUT_DIR}/node-hermes
    profiler=${HERMES_PROFILER_MODE_IN_LIT_TEST}
    gc=${HERMESVM_GCKIND}
    ubsan=${HERMES_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER}
    coverage=${coverage_directory}
    )

  set(HERMES_LIT_TEST_PARAMS
    ${HERMES_LIT_TEST_PARAMS_BASE}
    FileCheckOrRegen=${HERMES_TOOLS_OUTPUT_DIR}/FileCheck
    )

  set(HERMES_LIT_UPDATE_PARAMS
    ${HERMES_LIT_TEST_PARAMS_BASE}
    "FileCheckOrRegen=${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utils/lit-expect-gen/generate.py"
    )

  set(LLVH_LIT_ARGS "-sv")

  add_lit_testsuite(check-hermes "Running the Hermes regression tests"
    ${CMAKE_CURRENT_SOURCE_DIR}/test
    ${CMAKE_CURRENT_SOURCE_DIR}/unittests
    PARAMS ${HERMES_LIT_TEST_PARAMS}
    DEPENDS ${HERMES_TEST_DEPS}
    ARGS ${HERMES_TEST_EXTRA_ARGS}
    )
  set_target_properties(check-hermes PROPERTIES FOLDER "Hermes regression tests")

  # update-lit will regenerate the expectations for all tests that are verified with FileCheckOrRegen.
  # All other tests are run normally.
  add_lit_testsuite(update-lit "Running the Hermes regression tests, and updating auto-generated expectations."
    ${CMAKE_CURRENT_SOURCE_DIR}/test
    ${CMAKE_CURRENT_SOURCE_DIR}/unittests
    PARAMS ${HERMES_LIT_UPDATE_PARAMS}
    DEPENDS ${HERMES_TEST_DEPS}
    ARGS ${HERMES_TEST_EXTRA_ARGS}
    )
  set_target_properties(update-lit PROPERTIES FOLDER "Hermes regression tests")
endif()
