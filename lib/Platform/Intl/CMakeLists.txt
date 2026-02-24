# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesBCP47Parser BCP47Parser.cpp)

# Windows: Intl is ALWAYS built (HERMES_ENABLE_INTL is ignored).
# The WinGlob fallback ensures Intl always works at some level.
# ICU provider selection happens at runtime, not compile time.
if(WIN32)
  add_hermes_library(hermesPlatformIntl
      impl_win/PlatformIntlWin.cpp
      impl_win/PlatformIntlWinGlob.cpp
      impl_win/IcuProviderBundled.cpp
      impl_win/IcuProviderDynamic.cpp
      impl_win/IcuProviderWin.cpp
      impl_win/IcuProviderRegistry.cpp
      impl_win/WinIntlUtils.cpp
      LINK_LIBS
      hermesBCP47Parser
      hermesPublic
  )
  # NOTE: No hermes_link_icu() - ICU is loaded at runtime via GetProcAddress!
elseif(HERMES_ENABLE_INTL)
  if(HERMES_IS_ANDROID)
    add_hermes_library(hermesPlatformIntl PlatformIntlAndroid.cpp
        LINK_LIBS fbjni::fbjni hermesPublic
    )
    target_compile_options(hermesPlatformIntl_obj PRIVATE -frtti -fexceptions)
  elseif(APPLE)
    add_hermes_library(hermesPlatformIntl PlatformIntlApple.mm
        LINK_LIBS
        ${FOUNDATION}
        hermesBCP47Parser
        hermesPublic
    )
    # Work around a bug in unity builds where it tries to build Obj-C as C++.
    set_target_properties(hermesPlatformIntl PROPERTIES UNITY_BUILD false)
    target_compile_options(hermesPlatformIntl_obj PRIVATE -fobjc-arc)
  else()
    add_hermes_library(hermesPlatformIntl PlatformIntlICU.cpp
        PlatformIntlShared.cpp
        impl_icu/Collator.cpp
        impl_icu/IntlUtils.cpp
        impl_icu/LocaleConverter.cpp
        impl_icu/LocaleBCP47Object.cpp
        impl_icu/LocaleResolver.cpp
        impl_icu/OptionHelpers.cpp
        LINK_OBJLIBS
        hermesBCP47Parser
        hermesPublic
    )
    hermes_link_icu(hermesPlatformIntl)
  endif()
endif()
