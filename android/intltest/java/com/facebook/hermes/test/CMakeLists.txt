# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

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
  )
  target_compile_options(jsijni PRIVATE -frtti -fexceptions)

  add_hermes_library(jsijniepi SHARED
      ${epi_source_files}
      LINK_LIBS
      libhermes
      hermesapi
      compileJS
      fbjni::fbjni
      jsi
  )
  target_compile_options(jsijniepi PRIVATE -frtti -fexceptions)
endif()
