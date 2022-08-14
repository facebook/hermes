# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesBCP47Parser STATIC BCP47Parser.cpp)

if(HERMES_ENABLE_INTL)
  if(HERMES_IS_ANDROID)
    add_hermes_library(hermesPlatformIntl STATIC PlatformIntlAndroid.cpp
        LINK_LIBS fbjni::fbjni hermesPublic
    )
    target_compile_options(hermesPlatformIntl PRIVATE -frtti -fexceptions)
  elseif(APPLE)
    add_hermes_library(hermesPlatformIntl STATIC PlatformIntlApple.mm
        LINK_LIBS
        ${FOUNDATION}
        hermesBCP47Parser
        hermesPublic
    )
    # Work around a bug in unity builds where it tries to build Obj-C as C++.
    set_target_properties(hermesPlatformIntl PROPERTIES UNITY_BUILD false)
    target_compile_options(hermesPlatformIntl PRIVATE -fobjc-arc)
  else()
    add_hermes_library(hermesPlatformIntl STATIC PlatformIntlDummy.cpp LINK_LIBS hermesPublic)
  endif()
endif()
