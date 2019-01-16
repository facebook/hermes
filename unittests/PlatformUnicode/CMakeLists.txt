set(PlatformUnicodeSources
  PlatformUnicodeTest.cpp
  CharacterPropertyTest.cpp
  )

add_hermes_unittest(PlatformUnicodeTests
  ${PlatformUnicodeSources}
  )

target_link_libraries(PlatformUnicodeTests
  hermesPlatformUnicode
  ${CORE_FOUNDATION}
  )

hermes_link_icu(PlatformUnicodeTests)
