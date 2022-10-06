# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesAST
    ASTBuilder.cpp
    ESTree.cpp
    ESTreeJSONDumper.cpp
    Keywords.cpp Keywords.h
    SemValidate.cpp
    SemanticValidator.cpp SemanticValidator.h
    SemContext.cpp
    SemResolve.cpp
    SemanticResolver.cpp SemanticResolver.h
    DeclCollector.cpp DeclCollector.h
    ScopedFunctionPromoter.cpp ScopedFunctionPromoter.h
    CommonJS.cpp
    LINK_OBJLIBS
    hermesSupport
)
