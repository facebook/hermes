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
