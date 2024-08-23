cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

cmake_policy(PUSH)

if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Honor visibility properties.
endif()

if (POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

# Don't create a project if it was already created by another CMakeLists.txt.
# This allows one library to embed another library without making a collision.
if (NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit CXX)
endif()

include(CheckCXXCompilerFlag)
INCLUDE(CheckCXXSourceCompiles)
include(GNUInstallDirs)

# AsmJit - Deprecated
# ===================

if (DEFINED ASMJIT_BUILD_EMBED)
  message(DEPRECATION "ASMJIT_BUILD_EMBED is deprecated, use ASMJIT_EMBED")
  set(ASMJIT_EMBED "${ASMJIT_BUILD_EMBED}")
endif()

if (DEFINED ASMJIT_BUILD_STATIC)
  message(DEPRECATION "ASMJIT_BUILD_STATIC is deprecated, use ASMJIT_STATIC")
  set(ASMJIT_STATIC "${ASMJIT_BUILD_STATIC}")
endif()

# AsmJit - Configuration - Build
# ==============================

if (NOT DEFINED ASMJIT_TEST)
  set(ASMJIT_TEST FALSE)
endif()

if (NOT DEFINED ASMJIT_EMBED)
  set(ASMJIT_EMBED FALSE)
endif()

if (NOT DEFINED ASMJIT_STATIC)
  set(ASMJIT_STATIC ${ASMJIT_EMBED})
endif()

if (NOT DEFINED ASMJIT_SANITIZE)
  set(ASMJIT_SANITIZE FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_CUSTOM_FLAGS)
  set(ASMJIT_NO_CUSTOM_FLAGS FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_NATVIS)
  set(ASMJIT_NO_NATVIS FALSE)
endif()

# EMBED implies STATIC.
if (ASMJIT_EMBED AND NOT ASMJIT_STATIC)
  set(ASMJIT_STATIC TRUE)
endif()

# AsmJit - Configuration - Backend
# ================================

if (NOT DEFINED ASMJIT_NO_X86)
  set(ASMJIT_NO_X86 FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_AARCH64)
  set(ASMJIT_NO_AARCH64 FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_FOREIGN)
  set(ASMJIT_NO_FOREIGN FALSE)
endif()

# AsmJit - Configuration - Features
# =================================

if (NOT DEFINED ASMJIT_NO_DEPRECATED)
  set(ASMJIT_NO_DEPRECATED FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_SHM_OPEN)
  set(ASMJIT_NO_SHM_OPEN FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_JIT)
  set(ASMJIT_NO_JIT FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_TEXT)
  set(ASMJIT_NO_TEXT FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_LOGGING)
  set(ASMJIT_NO_LOGGING ${ASMJIT_NO_TEXT})
endif()

if (NOT DEFINED ASMJIT_NO_VALIDATION)
  set(ASMJIT_NO_VALIDATION FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_INTROSPECTION)
  set(ASMJIT_NO_INTROSPECTION FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_BUILDER)
  set(ASMJIT_NO_BUILDER FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_COMPILER)
  if (ASMJIT_NO_BUILDER OR ASMJIT_NO_INTROSPECTION)
    set(ASMJIT_NO_COMPILER TRUE)
  else()
    set(ASMJIT_NO_COMPILER FALSE)
  endif()
endif()

# AsmJit - Configuration - CMake Introspection
# ============================================

set(ASMJIT_DIR              "${CMAKE_CURRENT_LIST_DIR}"  CACHE PATH "Location of 'asmjit'")
set(ASMJIT_TEST             "${ASMJIT_TEST}"             CACHE BOOL "Build 'asmjit' test applications")
set(ASMJIT_EMBED            "${ASMJIT_EMBED}"            CACHE BOOL "Embed 'asmjit' library (no targets)")
set(ASMJIT_STATIC           "${ASMJIT_STATIC}"           CACHE BOOL "Build 'asmjit' library as static")
set(ASMJIT_SANITIZE         "${ASMJIT_SANITIZE}"         CACHE STRING "Build with sanitizers: 'address', 'undefined', etc...")
set(ASMJIT_NO_NATVIS        "${ASMJIT_NO_NATVIS}"        CACHE BOOL "Disable natvis support (embedding asmjit.natvis in PDB)")
set(ASMJIT_NO_CUSTOM_FLAGS  "${ASMJIT_NO_CUSTOM_FLAGS}"  CACHE BOOL "Disable extra compilation flags added by AsmJit to its targets")

set(ASMJIT_NO_X86           "${ASMJIT_NO_X86}"           CACHE BOOL "Disable X86/X64 backend")
set(ASMJIT_NO_AARCH64       "${ASMJIT_NO_AARCH64}"       CACHE BOOL "Disable AArch64 backend")
set(ASMJIT_NO_FOREIGN       "${ASMJIT_NO_FOREIGN}"       CACHE BOOL "Disable all foreign architectures (enables only a target architecture)")

set(ASMJIT_NO_DEPRECATED    "${ASMJIT_NO_DEPRECATED}"    CACHE BOOL "Disable deprecated API at build time")
set(ASMJIT_NO_SHM_OPEN      "${ASMJIT_NO_SHM_OPEN}"      CACHE BOOL "Disable the use of shm_open() even on platforms where it's supported")
set(ASMJIT_NO_JIT           "${ASMJIT_NO_JIT}"           CACHE BOOL "Disable VirtMem, JitAllocator, and JitRuntime at build time")
set(ASMJIT_NO_TEXT          "${ASMJIT_NO_TEXT}"          CACHE BOOL "Disable textual representation of instructions, enums, cpu features, ...")
set(ASMJIT_NO_LOGGING       "${ASMJIT_NO_LOGGING}"       CACHE BOOL "Disable logging features at build time")
set(ASMJIT_NO_VALIDATION    "${ASMJIT_NO_VALIDATION}"    CACHE BOOL "Disable instruction validation API at build time")
set(ASMJIT_NO_INTROSPECTION "${ASMJIT_NO_INTROSPECTION}" CACHE BOOL "Disable instruction introspection API at build time")
set(ASMJIT_NO_BUILDER       "${ASMJIT_NO_BUILDER}"       CACHE BOOL "Disable Builder emitter at build time")
set(ASMJIT_NO_COMPILER      "${ASMJIT_NO_COMPILER}"      CACHE BOOL "Disable Compiler emitter at build time")

# AsmJit - Project
# ================

set(ASMJIT_INCLUDE_DIRS "${ASMJIT_DIR}/src")     # Include directory is the same as source dir.
set(ASMJIT_DEPS "")                              # AsmJit dependencies (libraries) for the linker.
set(ASMJIT_LIBS "")                              # Dependencies of libs/apps that want to use AsmJit.
set(ASMJIT_CFLAGS "")                            # Public compiler flags.
set(ASMJIT_PRIVATE_CFLAGS "")                    # Private compiler flags independent of build type.
set(ASMJIT_PRIVATE_CFLAGS_DBG "")                # Private compiler flags used by debug builds.
set(ASMJIT_PRIVATE_CFLAGS_REL "")                # Private compiler flags used by release builds.
set(ASMJIT_SANITIZE_CFLAGS "")                   # Compiler flags required by currently enabled sanitizers.
set(ASMJIT_SANITIZE_LFLAGS "")                   # Linker flags required by currently enabled sanitizers.

# AsmJit - Utilities
# ==================

function(asmjit_detect_cflags out)
  set(out_array ${${out}})
  foreach(flag ${ARGN})
    string(REGEX REPLACE "[+]" "x" flag_signature "${flag}")
    string(REGEX REPLACE "[-=:;/.\]" "_" flag_signature "${flag_signature}")
    check_cxx_compiler_flag(${flag} "__CxxFlag_${flag_signature}")
    if (${__CxxFlag_${flag_signature}})
      list(APPEND out_array "${flag}")
    endif()
  endforeach()
  set(${out} "${out_array}" PARENT_SCOPE)
endfunction()

# Support for various sanitizers provided by C/C++ compilers.
function(asmjit_detect_sanitizers out)
  set(_out_array ${${out}})
  set(_flags "")

  foreach(_arg ${ARGN})
    string(REPLACE "," ";" _arg "${_arg}")
    list(APPEND _flags ${_arg})
  endforeach()

  foreach(_flag ${_flags})
    if (NOT "${_flag}" MATCHES "^-fsanitize=")
      SET(_flag "-fsanitize=${_flag}")
    endif()

    # Sanitizers also require link flags, see CMAKE_REQUIRED_FLAGS.
    set(CMAKE_REQUIRED_FLAGS "${_flag}")
    asmjit_detect_cflags(_out_array ${_flag})
    unset(CMAKE_REQUIRED_FLAGS)
  endforeach()

  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

function(asmjit_add_target target target_type)
  set(single_val "")
  set(multi_val SOURCES LIBRARIES CFLAGS CFLAGS_DBG CFLAGS_REL)
  cmake_parse_arguments("X" "" "${single_val}" "${multi_val}" ${ARGN})

  if ("${target_type}" MATCHES "^(EXECUTABLE|TEST)$")
    add_executable(${target} ${X_SOURCES})
  else()
    add_library(${target} ${target_type} ${X_SOURCES})
  endif()

  set_target_properties(${target} PROPERTIES DEFINE_SYMBOL "")
  target_link_libraries(${target} PRIVATE ${X_LIBRARIES})

  # target_link_options was added in cmake v3.13, don't use it for now...
  foreach(link_flag ${ASMJIT_PRIVATE_LFLAGS})
    set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${link_flag}")
  endforeach()

  target_compile_features(${target} PUBLIC cxx_std_11)
  set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS NO)
  set_property(TARGET ${target} PROPERTY CXX_VISIBILITY_PRESET hidden)
  target_compile_options(${target} PRIVATE ${X_CFLAGS} ${ASMJIT_SANITIZE_CFLAGS} $<$<CONFIG:Debug>:${X_CFLAGS_DBG}> $<$<NOT:$<CONFIG:Debug>>:${X_CFLAGS_REL}>)

  if ("${target_type}" STREQUAL "TEST")
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

# AsmJit - Compiler Support
# =========================

# We will have to keep this most likely forever as some users may still be using it.
set(ASMJIT_INCLUDE_DIR "${ASMJIT_INCLUDE_DIRS}")

if (NOT ASMJIT_NO_CUSTOM_FLAGS)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
    list(APPEND ASMJIT_PRIVATE_CFLAGS
      -MP                      # [+] Multi-Process Compilation.
      -GF                      # [+] Eliminate duplicate strings.
      -Zc:__cplusplus          # [+] Conforming __cplusplus definition.
      -Zc:inline               # [+] Remove unreferenced COMDAT.
      -Zc:strictStrings        # [+] Strict const qualification of string literals.
      -Zc:threadSafeInit-      # [-] Thread-safe statics.
      -W4)                     # [+] Warning level 4.

    list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG
      -GS)                     # [+] Buffer security-check.

    list(APPEND ASMJIT_PRIVATE_CFLAGS_REL
      -GS-                     # [-] Buffer security-check.
      -O2                      # [+] Favor speed over size.
      -Oi)                     # [+] Generate intrinsic functions.
  elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
    list(APPEND ASMJIT_PRIVATE_CFLAGS -Wall -Wextra -Wconversion)
    list(APPEND ASMJIT_PRIVATE_CFLAGS -fno-math-errno)
    list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -O2)

    # -fno-semantic-interposition is not available on apple - the compiler issues a warning, which is not detected.
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -fno-threadsafe-statics)
    else()
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -fno-threadsafe-statics -fno-semantic-interposition)
    endif()

    # The following flags can save few bytes in the resulting binary.
    asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL
      -fmerge-all-constants    # Merge all constants even if it violates ISO C++.
      -fno-enforce-eh-specs)   # Don't enforce termination if noexcept function throws.
  endif()
endif()

# Support for sanitizers.
if (ASMJIT_SANITIZE)
  asmjit_detect_sanitizers(ASMJIT_SANITIZE_CFLAGS ${ASMJIT_SANITIZE})
  if (ASMJIT_SANITIZE_CFLAGS)
    message("-- Enabling sanitizers: '${ASMJIT_SANITIZE_CFLAGS}'")

    # Linker must receive the same flags as the compiler when it comes to sanitizers.
    set(ASMJIT_SANITIZE_LFLAGS ${ASMJIT_SANITIZE_CFLAGS})

    # Don't omit frame pointer if sanitizers are enabled.
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
      list(APPEND ASMJIT_SANITIZE_CFLAGS -Oy-)
    else()
      list(APPEND ASMJIT_SANITIZE_CFLAGS -fno-omit-frame-pointer -g)
    endif()

    list(APPEND ASMJIT_PRIVATE_CFLAGS ${ASMJIT_SANITIZE_CFLAGS})
    list(APPEND ASMJIT_PRIVATE_LFLAGS ${ASMJIT_SANITIZE_LFLAGS})
  endif()
endif()

if (WIN32)
  # Dependency: nothing extra at the moment.
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  # Dependency: libc is the only required library on Android as it also provides libthread.
  message("-- Dependency: adding libc (Android target detected)")
  list(APPEND ASMJIT_DEPS c)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
  # Dependency: libroot is used by Haiku instead of libc, so link to libroot and libpthread.
  message("-- Dependency: adding libroot and libpthread (Haiku target detected)")
  list(APPEND ASMJIT_DEPS root pthread)
else()
  # Dependency: libc is always required.
  message("-- Dependency: adding libc (Linux, BSD, or other UNIX/POSIX environment)")
  list(APPEND ASMJIT_DEPS c)

  # Dependency: pthread (required so AsmJit can use pthread_lock).
  check_cxx_source_compiles("
    #include <pthread.h>
    int main() {
      pthread_mutex_t m;
      pthread_mutex_init(&m, nullptr);
      return pthread_mutex_destroy(&m);
    }
    " ASMJIT_LIBC_HAS_LIBPTHREAD)
  if (ASMJIT_LIBC_HAS_LIBPTHREAD)
    message("-- Dependency: libpthread provided by libc (not linking to libpthread)")
  else()
    message("-- Dependency: libpthread not provided by libc, linking to libpthread")
    list(APPEND ASMJIT_DEPS pthread)
  endif()

  # Dependency: shm_open (required so AsmJit can use shm_open on supported platforms).
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "^(Linux|NetBSD)$" AND NOT ASMJIT_NO_SHM_OPEN)
    check_cxx_source_compiles("
      #include <sys/mman.h>
      int main() {
        const char file_name[1] {};
        return shm_open(file_name, 0, 0);
      }
      " ASMJIT_LIBC_HAS_LIBRT)
    if (ASMJIT_LIBC_HAS_LIBRT)
      message("-- Dependency: shm_open provided by libc (not linking to librt)")
    else()
      message("-- Dependency: shm_open not provided by libc, linking to librt")
      list(APPEND ASMJIT_DEPS rt)
    endif()
  endif()
endif()

set(ASMJIT_LIBS ${ASMJIT_DEPS})
if (NOT ASMJIT_EMBED)
  list(INSERT ASMJIT_LIBS 0 asmjit)
endif()

if (ASMJIT_EMBED)
  set(ASMJIT_TARGET_TYPE "EMBED")
elseif (ASMJIT_STATIC)
  set(ASMJIT_TARGET_TYPE "STATIC")
else()
  set(ASMJIT_TARGET_TYPE "SHARED")
endif()

foreach(build_option # AsmJit build options.
                     ASMJIT_STATIC
                     ASMJIT_NO_DEPRECATED
                     # AsmJit backends selection.
                     ASMJIT_NO_X86
                     ASMJIT_NO_AARCH64
                     ASMJIT_NO_FOREIGN
                     # AsmJit features selection.
                     ASMJIT_NO_JIT
                     ASMJIT_NO_TEXT
                     ASMJIT_NO_LOGGING
                     ASMJIT_NO_INTROSPECTION
                     ASMJIT_NO_VALIDATION
                     ASMJIT_NO_BUILDER
                     ASMJIT_NO_COMPILER)
  if (${build_option})
    List(APPEND ASMJIT_CFLAGS         "-D${build_option}")
    List(APPEND ASMJIT_PRIVATE_CFLAGS "-D${build_option}")
  endif()
endforeach()

# AsmJit - Linker Support
# =======================

if (WIN32)
  if (CMAKE_LINKER MATCHES "link\\.exe" OR CMAKE_LINKER MATCHES "lld-link\\.exe")
    set(ASMJIT_LINKER_SUPPORTS_NATVIS TRUE)
  endif()
endif()

# AsmJit - Source
# ===============

set(ASMJIT_SRC_LIST
  asmjit/asmjit.h
  asmjit/asmjit-scope-begin.h
  asmjit/asmjit-scope-end.h

  asmjit/core.h
  asmjit/core/api-build_p.h
  asmjit/core/api-config.h
  asmjit/core/archtraits.cpp
  asmjit/core/archtraits.h
  asmjit/core/archcommons.h
  asmjit/core/assembler.cpp
  asmjit/core/assembler.h
  asmjit/core/builder.cpp
  asmjit/core/builder.h
  asmjit/core/codebuffer.h
  asmjit/core/codeholder.cpp
  asmjit/core/codeholder.h
  asmjit/core/codewriter.cpp
  asmjit/core/codewriter_p.h
  asmjit/core/compiler.cpp
  asmjit/core/compiler.h
  asmjit/core/compilerdefs.h
  asmjit/core/constpool.cpp
  asmjit/core/constpool.h
  asmjit/core/cpuinfo.cpp
  asmjit/core/cpuinfo.h
  asmjit/core/emithelper.cpp
  asmjit/core/emithelper_p.h
  asmjit/core/emitter.cpp
  asmjit/core/emitter.h
  asmjit/core/emitterutils.cpp
  asmjit/core/emitterutils_p.h
  asmjit/core/environment.cpp
  asmjit/core/environment.h
  asmjit/core/errorhandler.cpp
  asmjit/core/errorhandler.h
  asmjit/core/formatter.cpp
  asmjit/core/formatter.h
  asmjit/core/func.cpp
  asmjit/core/func.h
  asmjit/core/funcargscontext.cpp
  asmjit/core/funcargscontext_p.h
  asmjit/core/globals.cpp
  asmjit/core/globals.h
  asmjit/core/inst.cpp
  asmjit/core/inst.h
  asmjit/core/instdb.cpp
  asmjit/core/instdb_p.h
  asmjit/core/jitallocator.cpp
  asmjit/core/jitallocator.h
  asmjit/core/jitruntime.cpp
  asmjit/core/jitruntime.h
  asmjit/core/logger.cpp
  asmjit/core/logger.h
  asmjit/core/misc_p.h
  asmjit/core/operand.cpp
  asmjit/core/operand.h
  asmjit/core/osutils.cpp
  asmjit/core/osutils.h
  asmjit/core/osutils_p.h
  asmjit/core/raassignment_p.h
  asmjit/core/rabuilders_p.h
  asmjit/core/radefs_p.h
  asmjit/core/ralocal.cpp
  asmjit/core/ralocal_p.h
  asmjit/core/rapass.cpp
  asmjit/core/rapass_p.h
  asmjit/core/rastack.cpp
  asmjit/core/rastack_p.h
  asmjit/core/string.cpp
  asmjit/core/string.h
  asmjit/core/support.cpp
  asmjit/core/support.h
  asmjit/core/target.cpp
  asmjit/core/target.h
  asmjit/core/type.cpp
  asmjit/core/type.h
  asmjit/core/virtmem.cpp
  asmjit/core/virtmem.h
  asmjit/core/zone.cpp
  asmjit/core/zone.h
  asmjit/core/zonehash.cpp
  asmjit/core/zonehash.h
  asmjit/core/zonelist.cpp
  asmjit/core/zonelist.h
  asmjit/core/zonestack.cpp
  asmjit/core/zonestack.h
  asmjit/core/zonestring.h
  asmjit/core/zonetree.cpp
  asmjit/core/zonetree.h
  asmjit/core/zonevector.cpp
  asmjit/core/zonevector.h

  asmjit/a64.h
  asmjit/arm.h
  asmjit/arm/armformatter.cpp
  asmjit/arm/armformatter_p.h
  asmjit/arm/armglobals.h
  asmjit/arm/armoperand.h
  asmjit/arm/armutils.h
  asmjit/arm/a64archtraits_p.h
  asmjit/arm/a64assembler.cpp
  asmjit/arm/a64assembler.h
  asmjit/arm/a64builder.cpp
  asmjit/arm/a64builder.h
  asmjit/arm/a64compiler.cpp
  asmjit/arm/a64compiler.h
  asmjit/arm/a64emithelper.cpp
  asmjit/arm/a64emithelper_p.h
  asmjit/arm/a64emitter.h
  asmjit/arm/a64formatter.cpp
  asmjit/arm/a64formatter_p.h
  asmjit/arm/a64func.cpp
  asmjit/arm/a64func_p.h
  asmjit/arm/a64globals.h
  asmjit/arm/a64instapi.cpp
  asmjit/arm/a64instapi_p.h
  asmjit/arm/a64instdb.cpp
  asmjit/arm/a64instdb.h
  asmjit/arm/a64operand.cpp
  asmjit/arm/a64operand.h
  asmjit/arm/a64rapass.cpp
  asmjit/arm/a64rapass_p.h

  asmjit/x86.h
  asmjit/x86/x86archtraits_p.h
  asmjit/x86/x86assembler.cpp
  asmjit/x86/x86assembler.h
  asmjit/x86/x86builder.cpp
  asmjit/x86/x86builder.h
  asmjit/x86/x86compiler.cpp
  asmjit/x86/x86compiler.h
  asmjit/x86/x86emithelper.cpp
  asmjit/x86/x86emithelper_p.h
  asmjit/x86/x86emitter.h
  asmjit/x86/x86formatter.cpp
  asmjit/x86/x86formatter_p.h
  asmjit/x86/x86func.cpp
  asmjit/x86/x86func_p.h
  asmjit/x86/x86globals.h
  asmjit/x86/x86instdb.cpp
  asmjit/x86/x86instdb.h
  asmjit/x86/x86instdb_p.h
  asmjit/x86/x86instapi.cpp
  asmjit/x86/x86instapi_p.h
  asmjit/x86/x86operand.cpp
  asmjit/x86/x86operand.h
  asmjit/x86/x86rapass.cpp
  asmjit/x86/x86rapass_p.h
)

if (MSVC AND NOT ASMJIT_NO_NATVIS)
  list(APPEND ASMJIT_SRC_LIST asmjit.natvis)
endif()

set(ASMJIT_SRC "")
foreach(src_file ${ASMJIT_SRC_LIST})
  set(src_file "${ASMJIT_DIR}/src/${src_file}")
  list(APPEND ASMJIT_SRC ${src_file})

  if ("${src_file}" MATCHES "\\.natvis")
    if (ASMJIT_LINKER_SUPPORTS_NATVIS)
      list(APPEND ASMJIT_PRIVATE_LFLAGS "-natvis:${src_file}")
    endif()
  endif()
endforeach()

source_group(TREE "${ASMJIT_DIR}" FILES ${ASMJIT_SRC})

# AsmJit - Summary
# ================

message("** AsmJit Summary **")
message("   ASMJIT_DIR=${ASMJIT_DIR}")
message("   ASMJIT_TEST=${ASMJIT_TEST}")
message("   ASMJIT_TARGET_TYPE=${ASMJIT_TARGET_TYPE}")
message("   ASMJIT_DEPS=${ASMJIT_DEPS}")
message("   ASMJIT_LIBS=${ASMJIT_LIBS}")
message("   ASMJIT_CFLAGS=${ASMJIT_CFLAGS}")
message("   ASMJIT_PRIVATE_CFLAGS=${ASMJIT_PRIVATE_CFLAGS}")
message("   ASMJIT_PRIVATE_CFLAGS_DBG=${ASMJIT_PRIVATE_CFLAGS_DBG}")
message("   ASMJIT_PRIVATE_CFLAGS_REL=${ASMJIT_PRIVATE_CFLAGS_REL}")

# AsmJit - Targets
# ================

if (NOT ASMJIT_EMBED)
  # Add AsmJit target.
  asmjit_add_target(asmjit "${ASMJIT_TARGET_TYPE}"
                    SOURCES    ${ASMJIT_SRC}
                    LIBRARIES  ${ASMJIT_DEPS}
                    CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                    CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                    CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

  target_compile_options(asmjit INTERFACE ${ASMJIT_CFLAGS})
  target_include_directories(asmjit BEFORE INTERFACE
                             $<BUILD_INTERFACE:${ASMJIT_INCLUDE_DIRS}>
                             $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  # Add asmjit::asmjit alias.
  add_library(asmjit::asmjit ALIAS asmjit)
  # TODO: [CMAKE] Deprecated alias - we use projectname::libraryname convention now.
  add_library(AsmJit::AsmJit ALIAS asmjit)

  # Add AsmJit install instructions (library and public headers).
  if (NOT ASMJIT_NO_INSTALL)
    install(TARGETS asmjit
            EXPORT asmjit-config
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(EXPORT asmjit-config
            NAMESPACE asmjit::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asmjit")

    foreach(_src_file ${ASMJIT_SRC_LIST})
      if ("${_src_file}" MATCHES "\\.h$" AND NOT "${_src_file}" MATCHES "_p\\.h$")
        get_filename_component(_src_dir ${_src_file} PATH)
        install(FILES "${ASMJIT_DIR}/src/${_src_file}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_src_dir}")
      endif()
    endforeach()
  endif()

  # Add AsmJit tests.
  if (ASMJIT_TEST)
    enable_testing()

    # Special target that always uses embedded AsmJit.
    asmjit_add_target(asmjit_test_unit TEST
                      SOURCES    ${ASMJIT_SRC}
                                 test/asmjit_test_unit.cpp
                                 test/broken.cpp
                                 test/broken.h
                      LIBRARIES  ${ASMJIT_DEPS}
                      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                                 -DASMJIT_TEST
                                 -DASMJIT_STATIC
                      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    target_include_directories(asmjit_test_unit BEFORE PRIVATE ${ASMJIT_INCLUDE_DIRS})

    asmjit_add_target(asmjit_test_assembler TEST
                      SOURCES    test/asmjit_test_assembler.cpp
                                 test/asmjit_test_assembler.h
                                 test/asmjit_test_assembler_a64.cpp
                                 test/asmjit_test_assembler_x64.cpp
                                 test/asmjit_test_assembler_x86.cpp
                      LIBRARIES  asmjit::asmjit
                      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

    asmjit_add_target(asmjit_test_perf EXECUTABLE
                      SOURCES    test/asmjit_test_perf.cpp
                                 test/asmjit_test_perf_a64.cpp
                                 test/asmjit_test_perf_x86.cpp
                      SOURCES    test/asmjit_test_perf.h
                      LIBRARIES  asmjit::asmjit
                      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

    foreach(_target asmjit_test_emitters
                    asmjit_test_execute
                    asmjit_test_x86_sections)
      asmjit_add_target(${_target} TEST
                        SOURCES    test/${_target}.cpp
                        LIBRARIES  asmjit::asmjit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endforeach()

    if (NOT ASMJIT_NO_INTROSPECTION)
      asmjit_add_target(asmjit_test_instinfo TEST
                        SOURCES    test/asmjit_test_instinfo.cpp
                        LIBRARIES  asmjit::asmjit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endif()

    if (NOT (ASMJIT_NO_BUILDER OR ASMJIT_NO_COMPILER))
      # Vectorcall tests and XMM tests require at least SSE2 in 32-bit mode (in 64-bit mode it's implicit).
      # Some compilers don't like passing -msse2 for 64-bit targets, and some compilers targeting non-x86
      # would pass "-msse2" compile flag check, but with a warning not detected by CMake. Thus, verify that
      # our target is really 32-bit X86 and only use -msse2 or -arch:SSE2 flags when necessary.
      set(ASMJIT_SSE2_CFLAGS "")

      check_cxx_source_compiles("
        #if defined(_M_IX86) || defined(__X86__) || defined(__i386__)
          int target_is_32_bit_x86() { return 1; }
        #else
          // Compile error...
        #endif

        int main() {
          return target_is_32_bit_x86();
        }
        " ASMJIT_TARGET_IS_32_BIT_X86)

      if (ASMJIT_TARGET_IS_32_BIT_X86)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
          asmjit_detect_cflags(ASMJIT_SSE2_CFLAGS "-arch:SSE2")
        else()
          asmjit_detect_cflags(ASMJIT_SSE2_CFLAGS "-msse2")
        endif()
      endif()
      asmjit_add_target(asmjit_test_compiler TEST
                        SOURCES    test/asmjit_test_compiler.cpp
                                   test/asmjit_test_compiler.h
                                   test/asmjit_test_compiler_a64.cpp
                                   test/asmjit_test_compiler_x86.cpp
                        LIBRARIES  asmjit::asmjit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS} ${ASMJIT_SSE2_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endif()

  endif()
endif()

cmake_policy(POP)
