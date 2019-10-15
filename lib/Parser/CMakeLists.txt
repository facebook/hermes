# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(LLVM_LINK_COMPONENTS support)

add_llvm_library(hermesParser
        JSLexer.cpp
        JSONParser.cpp
        JSParser.cpp
        JSParserImpl.cpp JSParserImpl.h
        LINK_LIBS hermesSupport hermesAST dtoa
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

