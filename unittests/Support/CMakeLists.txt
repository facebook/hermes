# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(SupportSources
  Algorithms.cpp
  AllocatorTest.cpp
  BigIntSupportTest.cpp
  CheckedMalloc.cpp
  ConversionsTest.cpp
  CtorConfigTest.cpp
  Base64Test.cpp
  HashStringTest.cpp
  HermesSafeMathTest.cpp
  JSONEmitterTest.cpp
  LEB128Test.cpp
  OptValueTest.cpp
  OSCompatTest.cpp
  PageAccessTrackerTest.cpp
  PlatformLoggingTest.cpp
  RegexTest.cpp
  SerialExecutorTest.cpp
  SNPrintfBufTest.cpp
  SourceErrorManagerTest.cpp
  StackBoundsTest.cpp
  StatsAccumulatorTest.cpp
  StringSetVectorTest.cpp
  UnicodeTest.cpp
  test_sh_fp_trunc.cpp
  )

add_hermes_unittest(HermesSupportTests
  ${SupportSources}
  )

target_link_libraries(HermesSupportTests
 hermesPlatform
 hermesRegex
 hermesSupport
 hermesPublic
 dtoa
 )
