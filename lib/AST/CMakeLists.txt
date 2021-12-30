# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesAST
    ASTBuilder.cpp
    ESTree.cpp
    ESTreeJSONDumper.cpp
    SemValidate.cpp
    SemanticValidator.cpp SemanticValidator.h
    CommonJS.cpp
    LINK_LIBS
    hermesSupport
)
