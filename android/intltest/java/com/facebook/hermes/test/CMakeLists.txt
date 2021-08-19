# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

set(jni_source_files
    HermesRuntimeFactory.cpp
    JSRuntime.cpp
    MappedFileBuffer.cpp
    OnLoad.cpp
)

set(epi_source_files
    HermesEpilogue.cpp
    MappedFileBuffer.cpp
    OnLoadEpilogue.cpp
)

# fbjni is currently only configured for Android, so make this rule
# conditional to avoid unnecessarily breaking host build targets.
if(HERMES_IS_ANDROID)
  add_hermes_library(jsijni SHARED
      ${jni_source_files}
      LINK_LIBS
      libhermes
      hermesapi
      compileJS
      fbjni::fbjni
      jsi
      ${CORE_FOUNDATION}
  )
  target_compile_options(jsijni PRIVATE -frtti -fexceptions)
  hermes_link_icu(jsijni)

  add_hermes_library(jsijniepi SHARED
      ${epi_source_files}
      LINK_LIBS
      libhermes
      hermesapi
      compileJS
      fbjni::fbjni
      jsi
      ${CORE_FOUNDATION}
  )
  target_compile_options(jsijniepi PRIVATE -frtti -fexceptions)
  hermes_link_icu(jsijniepi)
endif()
