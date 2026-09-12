# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(PlatformUnicodeSources
  PlatformUnicodeTest.cpp
  CharacterPropertyTest.cpp
  NormalizationConformanceTest.cpp
  CaseConversionTest.cpp
  CollationTest.cpp
  CollationConformanceTest.cpp
  )

add_hermes_unittest(PlatformUnicodeTests
  ${PlatformUnicodeSources}
  )

# CollationTestData.inc holds one string literal per conformance row, and MSVC
# emits each literal into its own COMDAT section, which overflows the default
# COFF section limit (fatal error C1128). /bigobj lifts that limit.
if(MSVC)
  set_source_files_properties(CollationConformanceTest.cpp PROPERTIES
    COMPILE_OPTIONS "/bigobj")
endif()

target_link_libraries(PlatformUnicodeTests
  hermesPlatformUnicode
  )
