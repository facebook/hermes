set(LLVM_LINK_COMPONENTS
  Core
  )

set(ASTSources
  JSONTest.cpp
  ESTreeTest.cpp
  ValidatorTest.cpp
  )

add_hermes_unittest(HermesASTTests
    ${ASTSources}
  )

target_link_libraries(HermesASTTests
 hermesParser
 hermesAST
 hermesSupport
 )
