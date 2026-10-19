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
    # The ICU-backed implementation was deleted along with the ICU dependency.
    # Apple and Android have native Intl backends; nothing else does.
    message(FATAL_ERROR
      "HERMES_ENABLE_INTL is only supported on Apple and Android platforms.")
  endif()
endif()
