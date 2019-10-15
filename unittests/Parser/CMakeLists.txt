# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

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
