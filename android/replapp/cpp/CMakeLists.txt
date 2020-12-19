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