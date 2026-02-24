# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# This file centralizes all Windows-specific build configuration logic.

# Detect if lld-link is being used as the linker
set(HERMES_USING_LLD_LINK OFF)
if(DEFINED CMAKE_LINKER AND CMAKE_LINKER MATCHES "lld-link")
  set(HERMES_USING_LLD_LINK ON)
endif()

# The version that we put inside of DLL files
if(NOT DEFINED HERMES_FILE_VERSION)
  set(HERMES_FILE_VERSION "${PROJECT_VERSION}.0")
endif()

# Make the HERMES_FILE_VERSION accessible for version printing in C++.
if(HERMES_FILE_VERSION)
  add_definitions(-DHERMES_FILE_VERSION="${HERMES_FILE_VERSION}")
endif()

# The file version convertible to number representation.
# We must replace dots with commas.
string(REPLACE "." "," HERMES_FILE_VERSION_BIN ${HERMES_FILE_VERSION})

# Make the HERMES_FILE_VERSION_BIN accessible for converting to a number.
if(HERMES_FILE_VERSION_BIN)
  add_definitions(-DHERMES_FILE_VERSION_BIN=${HERMES_FILE_VERSION_BIN})
endif()

# Set the default target platform.
set(HERMES_WINDOWS_TARGET_PLATFORM "x64" CACHE STRING "Target compilation platform")

# Handle HERMES_WINDOWS_FORCE_NATIVE_BUILD option to override cross-compilation detection
option(HERMES_WINDOWS_FORCE_NATIVE_BUILD "Force CMake to treat this as a native build (not cross-compilation)" ON)
if(HERMES_WINDOWS_FORCE_NATIVE_BUILD)
  set(CMAKE_CROSSCOMPILING FALSE)
endif()

# Use Windows NLS (WinGlob) for Unicode on all Windows targets including UWP.
# ICU headers are not available at compile time (ICU is loaded at runtime).
set(DEFAULT_HERMES_MSVC_USE_PLATFORM_UNICODE_WINGLOB ON)

option(HERMES_MSVC_USE_PLATFORM_UNICODE_WINGLOB "Use platform Unicode WINGLOB" ${DEFAULT_HERMES_MSVC_USE_PLATFORM_UNICODE_WINGLOB})
if (HERMES_MSVC_USE_PLATFORM_UNICODE_WINGLOB)
  add_definitions(-DUSE_PLATFORM_UNICODE_WINGLOB)
endif()

# Configure Hybrid CRT for Windows builds (must be before any targets)
# Uses static C++ runtime with dynamic Universal CRT to avoid MSVCP140.dll dependency
# See for details: https://github.com/microsoft/WindowsAppSDK/blob/main/docs/Coding-Guidelines/HybridCRT.md
# UWP (WindowsStore) targets do not support the hybrid runtime configuration, so let
# them fall back to the toolchain default (dynamic CRT) to keep the ABI contract.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Enable source linking
if ("${CMAKE_C_COMPILER_ID}" MATCHES "MSVC")
  # NOTE: Dependencies are not properly setup here.
  #       Currently, CMake does not know to re-link if SOURCE_LINK_JSON changes
  #       Currently, CMake does not re-generate SOURCE_LINK_JSON if git's HEAD changes
  if ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER_EQUAL "19.20")
    include(SourceLink)
    file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/source_link.json" SOURCE_LINK_JSON)
    source_link(${PROJECT_SOURCE_DIR} ${SOURCE_LINK_JSON})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SOURCELINK:${SOURCE_LINK_JSON}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SOURCELINK:${SOURCE_LINK_JSON}")
  else()
    message(WARNING "Disabling SourceLink due to old version of MSVC. Please update to VS2019!")
  endif()
endif()

# Configure Clang compiler flags
function(hermes_windows_configure_clang_flags)
    # Basic Windows definitions
    set(CLANG_CXX_FLAGS "-DWIN32 -D_WINDOWS -D_CRT_RAND_S")
    
    # Debug information
    set(CLANG_CXX_FLAGS "${CLANG_CXX_FLAGS} -g")
    
    # Optimization flags for function/data sections (enables /OPT:REF)
    set(CLANG_CXX_FLAGS "${CLANG_CXX_FLAGS} -ffunction-sections -fdata-sections")
    
    # Security flags
    set(CLANG_CXX_FLAGS "${CLANG_CXX_FLAGS} -fstack-protector-all")
    set(CLANG_CXX_FLAGS "${CLANG_CXX_FLAGS} -Xclang -gsrc-hash=sha256")

    # On 32-bit x86, Clang assumes 16-byte stack alignment for SSE but the
    # Windows x86 ABI only guarantees 4-byte alignment.  Without explicit
    # realignment, SSE codegen in deep floating-point call chains (e.g. ICU's
    # uprv_floor → ClockMath::floorDivide → Grego::timeToFields) can corrupt
    # the /GS stack cookie, causing false stack-buffer-overrun aborts.
    if(HERMES_WINDOWS_TARGET_PLATFORM STREQUAL "x86")
      set(CLANG_CXX_FLAGS "${CLANG_CXX_FLAGS} -mstackrealign")
    endif()
   
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

# Configure MSVC compiler flags
function(hermes_windows_configure_msvc_flags)
    set(MSVC_CXX_FLAGS "")

    # Enable function-level linking (enables /OPT:REF linker optimization)
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /Gy")
    
    # Debug information
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /Zi")

    # Security flags
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /GS /DYNAMICBASE /guard:cf /Qspectre /sdl /ZH:SHA_256")
    
    # SDL (Security Development Lifecycle) requires these warnings enabled:
    # Note: Upstream Hermes.cmake disables these with -wd flags, but we override them here
    # for Windows security compliance. Last flag wins in MSVC.
    
    # C4146: Re-enable 'unary minus operator applied to unsigned type, result still unsigned'
    # Downgrade from error to warning level 3 (promoted to error by /sdl)
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /w34146")

    # Windows-specific warning suppressions for DLL builds

    # C4251: class X needs to have dll-interface to be used by clients of class Y
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /wd4251")
    # C4275: non dll-interface class X used as base for dll-interface class Y
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /wd4275")
    # C4646: function declared with 'noreturn' has non-void return type
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /wd4646")
    # C4312: 'reinterpret_cast': conversion from 'X' to 'hermes::vm::GCCell *' of greater size
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /wd4312")
    # C4703: potentially uninitialized local pointer variable used
    # /sdl re-promotes this after upstream Hermes.cmake's -wd4703; re-suppress it.
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /wd4703")

    # Silence C++20 deprecation of std::is_pod used in upstream llvh headers.
    # /sdl promotes C4996 (deprecation) to an error; this targeted macro avoids
    # modifying the vendored llvh code.
    set(MSVC_CXX_FLAGS "${MSVC_CXX_FLAGS} /D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING")

    # Apply flags to both C and C++
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSVC_CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_CXX_FLAGS}" PARENT_SCOPE)
endfunction()

# Configure lld-link (Clang) linker flags
function(hermes_windows_configure_lld_flags)
  # Debug information
  list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/DEBUG:FULL")

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Security flags
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/DYNAMICBASE")
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/guard:cf")
    if(NOT HERMES_MSVC_ARM64)
      list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/CETCOMPAT")
    endif()

    # Optimization flags
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/OPT:REF")
    # Note: ICF is disabled due to Hermes code using pointers to functions that are optimized away
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/OPT:NOICF")

    # Deterministic builds
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/BREPRO")

    # Hybrid CRT: Remove static UCRT, add dynamic UCRT
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/NODEFAULTLIB:libucrt.lib")
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/DEFAULTLIB:ucrt.lib")
  else()
    # Hybrid CRT Debug: Remove static debug UCRT, add dynamic debug UCRT
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/NODEFAULTLIB:libucrtd.lib")
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/DEFAULTLIB:ucrtd.lib")
  endif()

  # Enable Large Address Aware for x86 builds, allowing up to 4GB of virtual
  # address space on 64-bit Windows (default is 2GB for 32-bit executables).
  if(HERMES_WINDOWS_TARGET_PLATFORM STREQUAL "x86")
    list(APPEND HERMES_EXTRA_LINKER_FLAGS "LINKER:/LARGEADDRESSAWARE")
  endif()

  set(HERMES_EXTRA_LINKER_FLAGS "${HERMES_EXTRA_LINKER_FLAGS}" PARENT_SCOPE)
endfunction()

# Configure MSVC linker flags
function(hermes_windows_configure_msvc_linker_flags)
  # Debug information (common to debug and release)
  list(APPEND MSVC_COMMON_LINKER_FLAGS "LINKER:/DEBUG:FULL")

  # Enable Large Address Aware for x86 builds, allowing up to 4GB of virtual
  # address space on 64-bit Windows (default is 2GB for 32-bit executables).
  if(HERMES_WINDOWS_TARGET_PLATFORM STREQUAL "x86")
    list(APPEND MSVC_COMMON_LINKER_FLAGS "LINKER:/LARGEADDRESSAWARE")
  endif()

  # Debug-specific flags
  set(MSVC_DEBUG_LINKER_FLAGS "${MSVC_COMMON_LINKER_FLAGS}")

  # Hybrid CRT: Remove static UCRT, add dynamic UCRT
  list(APPEND MSVC_DEBUG_LINKER_FLAGS "LINKER:/NODEFAULTLIB:libucrtd.lib")
  list(APPEND MSVC_DEBUG_LINKER_FLAGS "LINKER:/DEFAULTLIB:ucrtd.lib")

  # Release-specific flags
  set(MSVC_RELEASE_LINKER_FLAGS "${MSVC_COMMON_LINKER_FLAGS}")

  # Hybrid CRT: Remove static UCRT, add dynamic UCRT
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/NODEFAULTLIB:libucrt.lib")
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/DEFAULTLIB:ucrt.lib")
  
  # Security flags
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/ZH:SHA_256")
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/guard:cf")
  if(NOT HERMES_MSVC_ARM64)
    list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/CETCOMPAT")
  endif()

  # Optimization flags
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/OPT:REF")
  # Note: ICF is disabled due to Hermes code using pointers to functions that are optimized away
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/OPT:NOICF")

  # Deterministic builds
  list(APPEND MSVC_RELEASE_LINKER_FLAGS "LINKER:/BREPRO")

  set(HERMES_EXTRA_LINKER_FLAGS
    "$<$<CONFIG:Debug>:${MSVC_DEBUG_LINKER_FLAGS}>"
    "$<$<CONFIG:Release>:${MSVC_RELEASE_LINKER_FLAGS}>"
    PARENT_SCOPE
  )
endfunction()

# Main configuration function
function(hermes_windows_configure_build)
  message(STATUS "Configuring Hermes Windows build...")
  message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "  Target platform: ${HERMES_WINDOWS_TARGET_PLATFORM}")

  if(${HERMES_WINDOWS_TARGET_PLATFORM} STREQUAL "arm64" OR ${HERMES_WINDOWS_TARGET_PLATFORM} STREQUAL "arm64ec")
    set(HERMES_MSVC_ARM64 ON)
  endif()

  # Configure compiler flags
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    hermes_windows_configure_clang_flags()
  else()
    hermes_windows_configure_msvc_flags()
  endif()
  
  # Configure linker flags
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    hermes_windows_configure_lld_flags()
  else()
    hermes_windows_configure_msvc_linker_flags()
  endif()
  
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
  set(HERMES_EXTRA_LINKER_FLAGS "${HERMES_EXTRA_LINKER_FLAGS}" PARENT_SCOPE)

  message(STATUS "Hermes Windows build configuration complete")
endfunction()

# Utility function to display current configuration
function(hermes_windows_show_configuration)
  message(STATUS "=== Hermes Windows Configuration ===")
  message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
  message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
  message(STATUS "Extra Linker Flags: ${HERMES_EXTRA_LINKER_FLAGS}")
  message(STATUS "=====================================")
endfunction()
