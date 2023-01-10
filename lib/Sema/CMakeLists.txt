# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesSema
    SemContext.cpp
    SemResolve.cpp
    SemanticResolver.cpp SemanticResolver.h
    DeclCollector.cpp DeclCollector.h
    ScopedFunctionPromoter.cpp ScopedFunctionPromoter.h
    LINK_OBJLIBS
    hermesSupport
)
