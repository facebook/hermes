# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesHBCBackend
  HBC.cpp
  ISel.cpp
  Bytecode.cpp
  BytecodeStream.cpp
  BytecodeGenerator.cpp
  BytecodeDataProvider.cpp
  BytecodeProviderFromSrc.cpp
  BytecodeDisassembler.cpp
  BytecodeFormConverter.cpp
  ConsecutiveStringStorage.cpp
  DebugInfo.cpp
  Passes.cpp
  SerializedLiteralGenerator.cpp
  SerializedLiteralParserBase.cpp
  SimpleBytecodeBuilder.cpp
  StringKind.cpp
  TraverseLiteralStrings.cpp
  UniquingFilenameTable.cpp
  UniquingStringLiteralTable.cpp
  Passes/FuncCallNOpts.cpp
  Passes/InsertProfilePoint.cpp
  Passes/OptEnvironmentInit.cpp
  LINK_OBJLIBS
  hermesBackend
  hermesInst
  hermesSourceMap
  hermesAST
  hermesPublic
)

add_hermes_library(hermesHBCBackendLean
  BytecodeDataProvider.cpp
  ConsecutiveStringStorage.cpp
  DebugInfo.cpp
  SerializedLiteralParserBase.cpp
  SimpleBytecodeBuilder.cpp
  UniquingFilenameTable.cpp
  LINK_OBJLIBS
  hermesSupport
  hermesPublic
)

target_compile_definitions(hermesHBCBackendLean_obj PUBLIC HERMESVM_LEAN)
