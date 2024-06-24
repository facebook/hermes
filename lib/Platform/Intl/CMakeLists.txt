# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesBCP47Parser BCP47Parser.cpp)

if(HERMES_ENABLE_INTL)
  if(HERMES_IS_ANDROID)
    add_hermes_library(hermesPlatformIntl PlatformIntlAndroid.cpp
        LINK_LIBS fbjni::fbjni
        LINK_OBJLIBS hermesPublic
    )
    target_compile_options(hermesPlatformIntl_obj PRIVATE -frtti -fexceptions)
  elseif(APPLE)
    add_hermes_library(hermesPlatformIntl PlatformIntlApple.mm
        LINK_LIBS ${FOUNDATION}
        LINK_OBJLIBS hermesPublic
    )
    # Work around a bug in unity builds where it tries to build Obj-C as C++.
    set_target_properties(hermesPlatformIntl_obj PROPERTIES UNITY_BUILD false)
    target_compile_options(hermesPlatformIntl_obj PRIVATE -fobjc-arc)
  else()
    add_hermes_library(hermesPlatformIntl PlatformIntlICU.cpp PlatformIntlShared.cpp
        LINK_OBJLIBS
        hermesBCP47Parser
        hermesPublic
    )
    hermes_link_icu(hermesPlatformIntl)
  endif()
endif()
