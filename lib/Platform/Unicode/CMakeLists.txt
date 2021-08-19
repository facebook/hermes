# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
   PlatformUnicodeJava.cpp
   PlatformUnicodeCF.cpp
   PlatformUnicodeICU.cpp
   PlatformUnicodeEmscripten.cpp
   CharacterProperties.cpp
)

if(HERMES_IS_ANDROID)
  add_hermes_library(hermesPlatformUnicode STATIC ${source_files}
      LINK_LIBS
      fbjni::fbjni
      ${CORE_FOUNDATION}
  )
  set_source_files_properties(PlatformUnicodeJava.cpp PROPERTIES
      COMPILE_FLAGS "-frtti -fexceptions -std=c++14")
else()
  add_hermes_library(hermesPlatformUnicode STATIC ${source_files}
      LINK_LIBS
      ${CORE_FOUNDATION}
  )
endif()

hermes_link_icu(hermesPlatformUnicode)
