# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(PlatformUnicodeSources
  PlatformUnicodeTest.cpp
  CharacterPropertyTest.cpp
  )

add_hermes_unittest(PlatformUnicodeTests
  ${PlatformUnicodeSources}
  )

target_link_libraries(PlatformUnicodeTests
  hermesPlatformUnicode
  )

if(HERMES_ENABLE_INTL)
  target_link_libraries(PlatformUnicodeTests
    hermesPlatformIntl
    hermesBCP47Parser
    hermesPublic
    )
endif()
