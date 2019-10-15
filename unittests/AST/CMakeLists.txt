# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

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
