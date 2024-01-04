# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesAST
    ASTBuilder.cpp
    ES6Class.cpp
    ESTree.cpp
    ESTreeJSONDumper.cpp
    CommonJS.cpp
    TS2Flow.cpp
    Context.cpp
    NativeContext.cpp
    LINK_OBJLIBS
    hermesRegex
    hermesSupport
)
