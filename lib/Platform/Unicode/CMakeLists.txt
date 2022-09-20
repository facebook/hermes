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

add_hermes_library(hermesPlatformUnicode ${source_files})

if(HERMES_UNICODE_LITE)
  # nothing
elseif(HERMES_IS_ANDROID)
  target_link_libraries(hermesPlatformUnicode_obj fbjni::fbjni)
  set_source_files_properties(PlatformUnicodeJava.cpp PROPERTIES
    COMPILE_FLAGS "-frtti -fexceptions")
elseif(APPLE)
  target_link_libraries(hermesPlatformUnicode_obj ${CORE_FOUNDATION})
endif()

hermes_link_icu(hermesPlatformUnicode_obj)
