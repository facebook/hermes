# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
#
# Builds hermes-icu.dll from vendored ICU 78 sources (Node.js icu-small).
# Single export: hermes_icu_get_vtable() returning const hermes_icu_vtable*.

cmake_minimum_required(VERSION 3.20)

# =============================================================================
# Step 1: Decompress ICU data and generate C source
# =============================================================================

set(ICU_DAT_BZ2 "${CMAKE_CURRENT_SOURCE_DIR}/source/data/in/icudt78l.dat.bz2")
set(ICU_DAT "${CMAKE_CURRENT_BINARY_DIR}/icudt78l.dat")
set(ICU_DAT_C "${CMAKE_CURRENT_BINARY_DIR}/icudt78l_dat.c")

# Decompress .bz2 at configure time using Python
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(NOT EXISTS "${ICU_DAT}")
  message(STATUS "Decompressing ICU data: ${ICU_DAT_BZ2}")
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c
      "import bz2,sys; open(sys.argv[2],'wb').write(bz2.open(sys.argv[1]).read())"
      "${ICU_DAT_BZ2}" "${ICU_DAT}"
    RESULT_VARIABLE _bz2_result
  )
  if(NOT _bz2_result EQUAL 0)
    message(FATAL_ERROR "Failed to decompress ICU data file")
  endif()
endif()

# Convert .dat to C byte array at configure time
if(NOT EXISTS "${ICU_DAT_C}" OR "${ICU_DAT}" IS_NEWER_THAN "${ICU_DAT_C}")
  message(STATUS "Generating ICU data C source: ${ICU_DAT_C}")
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "
import sys
with open(sys.argv[1], 'rb') as f:
    data = f.read()
with open(sys.argv[2], 'w') as out:
    out.write('/* Auto-generated from icudt78l.dat - do not edit. */\\n')
    out.write('#ifdef _MSC_VER\\n')
    out.write('__declspec(align(16))\\n')
    out.write('#else\\n')
    out.write('__attribute__((aligned(16)))\\n')
    out.write('#endif\\n')
    out.write('const unsigned char icudt78_dat[] = {\\n')
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        out.write('  ' + ','.join(f'0x{b:02x}' for b in chunk) + ',\\n')
    out.write('};\\n')
"
      "${ICU_DAT}" "${ICU_DAT_C}"
    RESULT_VARIABLE _gen_result
  )
  if(NOT _gen_result EQUAL 0)
    message(FATAL_ERROR "Failed to generate ICU data C source")
  endif()
endif()

# =============================================================================
# Step 2: Collect ICU source files
# =============================================================================

file(GLOB ICU_COMMON_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/source/common/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/common/*.c"
)
file(GLOB ICU_I18N_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/source/i18n/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/i18n/*.c"
)

# =============================================================================
# Step 3: Define the hermes-icu shared library target
# =============================================================================

add_library(hermes-icu SHARED
  ${ICU_COMMON_SOURCES}
  ${ICU_I18N_SOURCES}
  ${ICU_DAT_C}
  hermes_icu_glue.cpp
)

set_target_properties(hermes-icu PROPERTIES
  OUTPUT_NAME "hermes-icu"
)

# =============================================================================
# Step 4: Include directories
# =============================================================================

# BEFORE ensures our ICU 78 headers take priority over the old
# external/icu_decls/ headers that are added globally by the root CMakeLists.txt.
target_include_directories(hermes-icu BEFORE PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/source/common
  ${CMAKE_CURRENT_SOURCE_DIR}/source/i18n
  ${CMAKE_SOURCE_DIR}/include  # for hermes/Platform/Intl/hermes_icu.h
)

# =============================================================================
# Step 5: Compile definitions
# =============================================================================

# Definitions for ALL ICU sources
target_compile_definitions(hermes-icu PRIVATE
  U_STATIC_IMPLEMENTATION=1
  U_DISABLE_RENAMING=1
  U_ENABLE_DYLOAD=0
  UCONFIG_NO_SERVICE=1
  U_HAVE_STD_STRING=1
  U_ATTRIBUTE_DEPRECATED=
)

# U_COMMON_IMPLEMENTATION only for common sources
set_property(SOURCE ${ICU_COMMON_SOURCES} APPEND PROPERTY
  COMPILE_DEFINITIONS U_COMMON_IMPLEMENTATION=1
)

# U_I18N_IMPLEMENTATION only for i18n sources
set_property(SOURCE ${ICU_I18N_SOURCES} APPEND PROPERTY
  COMPILE_DEFINITIONS U_I18N_IMPLEMENTATION=1
)

# =============================================================================
# Step 6: Compiler flags
# =============================================================================

# ICU requires exceptions and RTTI
if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # /utf-8: ICU sources contain UTF-8 encoded Unicode character literals (e.g.
  # u'£', u'₹'). Without this flag MSVC uses the system codepage and fails
  # with C2015 "too many characters in constant".
  target_compile_options(hermes-icu PRIVATE /EHsc /GR /utf-8)
else()
  target_compile_options(hermes-icu PRIVATE -fexceptions -frtti)
endif()

# Suppress warnings for vendored ICU code (we don't own these sources)
if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_options(hermes-icu PRIVATE /W0)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  target_compile_options(hermes-icu PRIVATE
    -Wno-deprecated-declarations
    -Wno-sign-compare
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-implicit-fallthrough
    -Wno-missing-field-initializers
    -Wno-shorten-64-to-32
    -Wno-covered-switch-default
    -Wno-unreachable-code-break
    -Wno-unreachable-code-return
    -Wno-extra-semi
    -Wno-comma
    -Wno-format-nonliteral
    -Wno-undef
    -Wno-tautological-undefined-compare
    -Wno-header-hygiene
    -Wno-ambiguous-reversed-operator
    -Wno-suggest-destructor-override
    -Wno-suggest-override
  )
endif()

# =============================================================================
# Step 7: Linker configuration
# =============================================================================

# Single export via .def file
target_sources(hermes-icu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/hermes-icu.def"
)

# Security linker flags (BinSkim compliance)
target_link_options(hermes-icu PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

# Windows system libraries needed by ICU
target_link_libraries(hermes-icu PRIVATE advapi32)

# =============================================================================
# Step 8: Version resource
# =============================================================================

if(WIN32)
  configure_file(version.rc.in version.rc @ONLY)
  target_sources(hermes-icu PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
endif()

# =============================================================================
# Step 9: Copy hermes-icu.dll next to hermes.exe for testing
# =============================================================================

add_custom_command(TARGET hermes-icu POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:hermes-icu>
    ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:hermes-icu>
  COMMENT "Copying hermes-icu.dll to bin/ for testing"
)
