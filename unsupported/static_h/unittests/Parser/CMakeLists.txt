# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

set(ParserSources
  JSLexerTest.cpp
  JSONParserTest.cpp
  JSParserTest.cpp
  )

add_hermes_unittest(HermesParserTests
  ${ParserSources}
  )

target_link_libraries(HermesParserTests
 hermesAST
 hermesParser
 hermesSupport
 )
