# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

project(hermesfbjni CXX)

set(fbjni_SOURCES
  ${FBJNI_PATH}/cxx/fbjni/ByteBuffer.cpp
  ${FBJNI_PATH}/cxx/fbjni/OnLoad.cpp
  ${FBJNI_PATH}/cxx/fbjni/ReadableByteChannel.cpp
  ${FBJNI_PATH}/cxx/fbjni/fbjni.cpp
  ${FBJNI_PATH}/cxx/fbjni/detail/Environment.cpp
  ${FBJNI_PATH}/cxx/fbjni/detail/Exceptions.cpp
  ${FBJNI_PATH}/cxx/fbjni/detail/Hybrid.cpp
  ${FBJNI_PATH}/cxx/fbjni/detail/References.cpp
  ${FBJNI_PATH}/cxx/fbjni/detail/utf8.cpp
  ${FBJNI_PATH}/cxx/lyra/cxa_throw.cpp
  ${FBJNI_PATH}/cxx/lyra/lyra.cpp
  ${FBJNI_PATH}/cxx/lyra/lyra_breakpad.cpp
  ${FBJNI_PATH}/cxx/lyra/lyra_exceptions.cpp
)

add_library(hermesfbjni STATIC
  ${fbjni_SOURCES}
)

target_compile_options(hermesfbjni PRIVATE
  -fno-omit-frame-pointer
  -fexceptions
  -O3
  -Wall
  -std=c++11
  -frtti
  -ffunction-sections
  -DDISABLE_CPUCAP
  -DDISABLE_XPLAT
  -DNDEBUG
  -Wno-error
)

target_include_directories(hermesfbjni PUBLIC
  ${FBJNI_PATH}/cxx
)

target_link_libraries(hermesfbjni
  android
  log
)
