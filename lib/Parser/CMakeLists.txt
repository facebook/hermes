# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesParser
        JSLexer.cpp
        JSONParser.cpp
        JSParser.cpp
        JSParserImpl.cpp
        JSParserImpl-flow.cpp
        JSParserImpl-jsx.cpp
        JSParserImpl-ts.cpp
        JSParserImpl.h
        FlowHelpers.cpp
        rust-api.cpp
        LINK_OBJLIBS hermesSupport hermesAST dtoa LLVHSupport
)
