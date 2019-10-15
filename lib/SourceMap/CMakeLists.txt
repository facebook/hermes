# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_llvm_library(hermesSourceMap
    SourceMap.cpp
    SourceMapGenerator.cpp
    SourceMapParser.cpp
    SourceMapTranslator.cpp
    LINK_LIBS hermesParser
)
