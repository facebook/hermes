# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_hermes_library(hermesreplapp SHARED
        ReplProxy.cpp
        LINK_LIBS
        libhermes
        hermesapi
        compileJS
        hermesfbjni
        jsi
        ${CORE_FOUNDATION}
)
target_compile_options(hermesreplapp PRIVATE -frtti -fexceptions)
hermes_link_icu(hermesreplapp)

add_library(hermesevalapp SHARED
        EvalProxy.cpp
)

target_link_libraries(hermesevalapp
        hermesfbjni
)

include_directories(${HERMES_SOURCE_DIR}/../build_android/outputs/include/)

# This is no doubt ugly !
# target_link_libraries(hermesevalapp "${HERMES_SOURCE_DIR}/../build_android/outputs/${INTLTESTAPP_MODE}/unstripped-debug/lib/${CMAKE_ANDROID_ARCH_ABI}/libhermes.so")
target_link_libraries(hermesevalapp "${HERMES_SOURCE_DIR}/../build_android//distributions/hermes-runtime-android-${INTLTESTAPP_MODE}/unstripped-release/lib/${CMAKE_ANDROID_ARCH_ABI}/libhermes.so")
target_compile_options(hermesevalapp PRIVATE -frtti -fexceptions)
