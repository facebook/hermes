# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  CompilerDriver.cpp
)

add_hermes_library(hermesCompilerDriver STATIC ${source_files}
  LINK_LIBS
  hermesADT
  hermesAST
  hermesAST2JS
  hermesInst
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesFlowParser
  hermesSupport
  zip
)
