# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

if(NOT HERMES_IS_ANDROID)
  # We need FindICU from 3.7
  cmake_minimum_required(VERSION 3.7.0)
else()
  # We'll be using ICU through Java, so we don't need FindICU.
  # 3.6.0 is the minimum version shipped with the Android SDK.
  cmake_minimum_required(VERSION 3.6.0)
endif()

# Set the VERSION variables based on the project command
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

# Include file check macros honor CMAKE_REQUIRED_LIBRARIES.
if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

# This must be consistent with the release_version in
# android/build.gradle and npm/package.json
project(Hermes
        VERSION 0.1.1
        LANGUAGES C CXX)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

set(HERMES_RELEASE_VERSION ${PROJECT_VERSION})

# Project options.
set(HERMESVM_GCKIND NONCONTIG_GENERATIONAL
  CACHE STRING
  "HermesVM GC type: either NONCONTIG_GENERATIONAL or MALLOC")
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

set(HERMESVM_TIMELIMIT OFF CACHE BOOL
  "Enable time limit in hermes VM")

set(HERMESVM_INDIRECT_THREADING ${DEFAULT_INTERPRETER_THREADING} CACHE BOOL
  "Enable the indirect threaded interpreter")

set(HERMESVM_ALLOW_COMPRESSED_POINTERS ON CACHE BOOL
  "Enable compressed pointers. If this is on and the target is a 64-bit build, compressed pointers will be used.")

# Hermes VM Handle sanitization (moving the heap after every alloc)
set(HERMESVM_SANITIZE_HANDLES OFF CACHE BOOL
  "Enable Handle sanitization")

# Build with -DHERMES_SLOW_DEBUG for debug builds
# This does not affect release builds
set(HERMES_SLOW_DEBUG ON CACHE BOOL
  "Enable slow checks in Debug builds")

# On CentOS:
#   sudo yum install zlib-static glibc-static ncurses-static readline-static
set(HERMES_STATIC_LINK OFF CACHE BOOL
  "Link Hermes statically. May only work on GNU/Linux.")

set(HERMES_ENABLE_DEBUGGER OFF CACHE BOOL
  "Build with debugger support")

set(HERMES_FACEBOOK_BUILD OFF CACHE BOOL
    "Build Facebook (rather than open-source) version of Hermes")

set(HERMESVM_EXCEPTION_ON_OOM OFF CACHE BOOL
    "GC Out-of-memory raises an exception, rather than causing a crash")

set(HERMESVM_JIT OFF CACHE BOOL
  "Enable the JIT")

set(HERMESVM_JIT_DISASSEMBLER OFF CACHE BOOL
  "Enable the JIT disassembler")

set(HERMESVM_SYNTH_REPLAY OFF CACHE BOOL
  "Enable native JSI function recording")

set(HERMES_USE_FLOWPARSER OFF CACHE BOOL
  "Use libflowparser for parsing es6")

set(HERMES_IS_ANDROID OFF CACHE BOOL
  "Building for Android")

set(HERMES_ENABLE_WERROR OFF CACHE BOOL
  "Whether the build should have -Werror enabled")

set(HERMES_ENABLE_WIN10_ICU_FALLBACK ON CACHE BOOL
  "Whether to allow falling back on Win10 ICU")

set(HERMES_GITHUB_RESOURCE_DIR "" CACHE STRING
  "A directory with additional files to bundle in the GitHub release")

set(ANDROID_LINUX_PERF_PATH ""
  CACHE STRING
  "If buildling for Android, full path to <linux/perf_events.h>")

# On Android we need an ABI specific version of LLVM.
# Unfortunately, the build system doesn't allow setting the
# LLVM_BUILD_DIR per ABI, so we do it here instead.
if (HERMES_IS_ANDROID)
  set(LLVM_BUILD_DIR "${LLVM_BUILD_BASE}-${ANDROID_ABI}")
  add_definitions(-DHERMES_PLATFORM_UNICODE=HERMES_PLATFORM_UNICODE_JAVA)

  # The toolchain passes -Wa,--noexecstack which is valid for compiling
  # but not for linking. Just ignore it.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")

  # JS developers aren't VM developers. Give them a faster build.
  set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")

  # The release build can focus on size.
  set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

  if (ANDROID_ABI STREQUAL "arm64-v8a")
    # Using -flto on arm64 fails due to it using a different linker by default
    # https://github.com/android-ndk/ndk/issues/242
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fuse-ld=gold")
  endif()
endif()

# The hermes project is built using CMake and the LLVM's build system.
# It expects LLVM_SRC_DIR and LLVM_BUILD_DIR to be set. For example:
#
# cmake -G Ninja -DLLVM_BUILD_DIR=$XXX -DLLVM_SRC_DIR=$YYY
#
file(TO_CMAKE_PATH "${LLVM_BUILD_DIR}" LLVM_BUILD_DIR)
file(TO_CMAKE_PATH "${LLVM_SRC_DIR}" LLVM_SRC_DIR)
list(APPEND CMAKE_MODULE_PATH "${LLVM_BUILD_DIR}/lib/cmake/llvm/")

# Enable debug mode by default
if ((NOT GENERATOR_IS_MULTI_CONFIG) AND CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (HERMES_STATIC_LINK)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

CHECK_CXX_SOURCE_COMPILES(
  "int main() { void *p = &&label; goto *p; label: return 0; }"
  HAVE_COMPUTED_GOTO)

if(HAVE_COMPUTED_GOTO)
  set(DEFAULT_INTERPRETER_THREADING ON)
else()
  set(DEFAULT_INTERPRETER_THREADING OFF)
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
if(HERMESVM_SANITIZE_HANDLES)
    add_definitions(-DHERMESVM_SANITIZE_HANDLES)
endif()
if(HERMES_FACEBOOK_BUILD)
    add_definitions(-DHERMES_FACEBOOK_BUILD)
endif()
if(HERMESVM_EXCEPTION_ON_OOM)
    add_definitions(-DHERMESVM_EXCEPTION_ON_OOM)
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

# Only interpret if() arguments as variables or keywords when unquoted.
# CMake emits a warning if this is not set.
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

if (HERMES_ENABLE_WERROR)
  # Turn all warnings into errors on some compilers:
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(LLVM_ENABLE_WERROR ON)
  elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(LLVM_ENABLE_WERROR ON)
  elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    # Warn.
  endif()
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
    option(LLVM_ENABLE_LTO "Build Hermes with LTO" ON)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  # Superss warnings about unknown warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option")
  # Suppress an uninteresting warning about C++17 name mangling changes.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")
  # Don't export symbols unless we explicitly say so
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  # C4068 unknown pragma
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4068")
  # C4201 nonstandard extension used: nameless struct/union
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4201")
endif()

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
if (APPLE)
  set(ICU_FOUND 1)
  set(ICU_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/icu_decls)
  set(ICU_LIBRARIES
    icucore
  )
  include_directories(${ICU_INCLUDE_DIRS})
endif()

if (NOT ICU_FOUND)
  # Workaround: FindICU does not correctly recognize ICU include dir until
  # CMake 3.8.0.  https://github.com/Kitware/CMake/commit/cdf7e5d8
  list(APPEND icu_include_suffixes "include")

  # FindICU uses ICU_ROOT variable as a hint
  # Include 'uc' twice for static libraries that depend on each other.
  find_package(ICU 52 COMPONENTS uc i18n data uc)

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
  if (HERMES_STATIC_LINK AND target_type MATCHES "EXECUTABLE|STATIC_LIBRARY")
    target_link_libraries(${target_name} PRIVATE dl pthread)
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

if (HERMES_ENABLE_DEBUGGER)
  add_definitions(-DHERMES_ENABLE_DEBUGGER)
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

set(HERMES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HERMES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Tell LLVM where to place the compiled binaries.
#
# NOTE: LLVM also uses this variable to find its own binaries, which can create
# problems. For now the only LLVM tool we need is llvm-lit, so our short term
# solution is to just hack that special case (the long term solution would be
# to stop using add_llvm_tool() for our own tools).
set(LLVM_RUNTIME_OUTPUT_INTDIR ${HERMES_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)

if(EXISTS ${HERMES_SOURCE_DIR}/API/jsi)
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/API/jsi)
elseif(EXISTS ${FBSOURCE_DIR}/xplat/jsi)
  set(HERMES_JSI_DIR ${FBSOURCE_DIR}/xplat/jsi)
elseif(EXISTS ${HERMES_SOURCE_DIR}/../jsi)
  set(HERMES_JSI_DIR ${HERMES_SOURCE_DIR}/../jsi)
else()
  message(FATAL_ERROR "Unable to find jsi.")
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/public
  ${CMAKE_CURRENT_SOURCE_DIR}/external/flowparser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external
  )

if(HERMES_IS_ANDROID)
  if(EXISTS ${HERMES_SOURCE_DIR}/first-party/fbjni/cxx)
    set(FBJNI_PATH ${HERMES_SOURCE_DIR}/first-party/fbjni)
  elseif(EXISTS ${FBSOURCE_DIR}/fbandroid/libraries/fbjni/cxx)
    set(FBJNI_PATH ${FBSOURCE_DIR}/fbandroid/libraries/fbjni)
  elseif(EXISTS ${HERMES_SOURCE_DIR}/../../fbandroid/libraries/fbjni/cxx)
    set(FBJNI_PATH ${HERMES_SOURCE_DIR}/../../fbandroid/libraries/fbjni/cxx)
  else()
    message(FATAL_ERROR "Unable to find fbjni.")
  endif()
  include_directories("${FBJNI_PATH}/cxx/")
  add_subdirectory(first-party/fbjni)

  # JNI requires that JNI_OnLoad is (re-)exported for initialization.
  set(OPTIONAL_JNI_ONLOAD "-Wl,--undefined=JNI_OnLoad")
endif()

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
  interp-dispatch-bench
  hdb
  hbcdump
  hermes-repl
  hbc-attribute
  hbc-deltaprep
  hbc-diff
  llvm-config
  )

set(HERMES_LIT_TEST_PARAMS
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
  hbcdump=${HERMES_BINARY_DIR}/bin/hbcdump
  repl=${HERMES_BINARY_DIR}/bin/hermes-repl
  hbc-deltaprep=${HERMES_BINARY_DIR}/bin/hbc-deltaprep
  hbc_diff=${HERMES_BINARY_DIR}/bin/hbc-diff
  build_mode=${HERMES_ASSUMED_BUILD_MODE_IN_LIT_TEST}
  exception_on_oom_enabled=${HERMESVM_EXCEPTION_ON_OOM}
  timelimit_enabled=${HERMESVM_TIMELIMIT}
  )

set(LLVM_LIT_ARGS "-sv")

# This is a hack to help LLVM find its own llvm-lit tool, since we had to set
# LLVM_RUNTIME_OUTPUT_INTDIR to our own build dir.
set(LLVM_LIT_OUTPUT_DIR ${LLVM_BUILD_DIR}/${CMAKE_CFG_INTDIR}/bin)

add_lit_testsuite(check-hermes "Running the Hermes regression tests"
  ${HERMES_SOURCE_DIR}/test
  ${HERMES_SOURCE_DIR}/unittests
  PARAMS ${HERMES_LIT_TEST_PARAMS}
  DEPENDS ${HERMES_TEST_DEPS}
  ARGS ${HERMES_TEST_EXTRA_ARGS}
  )
set_target_properties(check-hermes PROPERTIES FOLDER "Hermes regression tests")

# This is how github release files are built.

set(HERMES_GITHUB_DIR ${HERMES_BINARY_DIR}/github)
string(TOLOWER ${CMAKE_SYSTEM_NAME} HERMES_GITHUB_SYSTEM_NAME)
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
  DEPENDS hermes hermes-repl make_bundle_dir
  VERBATIM
  COMMAND
    # We need bin/hermes or Release/bin/hermes.exe in a predictable location
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hermes> $<TARGET_FILE:hermes-repl> ${HERMES_GITHUB_EXTRAS} .
  COMMAND
    ${CMAKE_COMMAND} -E tar zcf ${HERMES_GITHUB_DIR}/${HERMES_CLI_GITHUB_FILE} .
)

add_custom_target(
  github-cli-release
  DEPENDS ${HERMES_GITHUB_DIR}/${HERMES_CLI_GITHUB_FILE})
