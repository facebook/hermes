# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

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
        LINK_LIBS hermesSupport hermesAST dtoa
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
