set(SupportSources
  Algorithms.cpp
  AllocatorTest.cpp
  ConsumableRangeTest.cpp
  ConversionsTest.cpp
  CtorConfigTest.cpp
  HashStringTest.cpp
  JSONEmitterTest.cpp
  LEB128Test.cpp
  PlatformLoggingTest.cpp
  RegexTest.cpp
  SNPrintfBufTest.cpp
  SourceErrorManagerTest.cpp
  UnicodeTest.cpp
  )

add_hermes_unittest(HermesSupportTests
  ${SupportSources}
  )

target_link_libraries(HermesSupportTests
 hermesPlatform
 hermesSupport
 dtoa
 )
