# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
   PlatformUnicodeJava.cpp
   PlatformUnicodeCF.cpp
   PlatformUnicodeICU.cpp
   PlatformUnicodeLite.cpp
   PlatformUnicodeEmscripten.cpp
   CharacterProperties.cpp
)

if (HERMES_UNICODE_LITE)
  add_definitions(-DHERMES_PLATFORM_UNICODE=HERMES_PLATFORM_UNICODE_LITE)
  add_hermes_library(hermesPlatformUnicode STATIC ${source_files})
elseif(HERMES_IS_ANDROID)
  add_hermes_library(hermesPlatformUnicode STATIC ${source_files}
      LINK_LIBS
      fbjni::fbjni
  )
  set_source_files_properties(PlatformUnicodeJava.cpp PROPERTIES
      COMPILE_FLAGS "-frtti -fexceptions")
else()
  add_hermes_library(hermesPlatformUnicode STATIC ${source_files}
      LINK_LIBS
      ${CORE_FOUNDATION}
  )
endif()

hermes_link_icu(hermesPlatformUnicode)
