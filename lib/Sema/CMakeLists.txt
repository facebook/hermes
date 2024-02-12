# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesSema
    ASTEval.cpp ASTEval.h
    ASTLowering.cpp ASTLowering.h
    Keywords.cpp
    SemContext.cpp
    SemResolve.cpp
    SemanticResolver.cpp SemanticResolver.h
    DeclCollector.cpp DeclCollector.h
    ScopedFunctionPromoter.cpp ScopedFunctionPromoter.h
    ESTreeClone.cpp ESTreeClone.h
    CheckImplicitReturn.cpp
    FlowContext.cpp
    FlowTypesDumper.cpp FlowTypesDumper.h
    FlowChecker.cpp FlowChecker.h
    FlowChecker-scopetypes.cpp
    LINK_OBJLIBS
    hermesSupport
    hermesAST
)
