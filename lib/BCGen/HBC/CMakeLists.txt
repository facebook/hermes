# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesHBCBackend
  HBC.cpp
  ISel.cpp ISel.h
  Bytecode.cpp
  BytecodeStream.cpp
  BytecodeGenerator.cpp BytecodeGenerator.h
  BCProvider.cpp
  BCProviderFromSrc.cpp
  BytecodeDisassembler.cpp
  BytecodeFormConverter.cpp
  ConsecutiveStringStorage.cpp
  DebugInfo.cpp
  Passes.cpp
  SimpleBytecodeBuilder.cpp
  StringKind.cpp
  UniquingFilenameTable.cpp
  UniquingStringLiteralTable.cpp
  LoweringPipelines.cpp LoweringPipelines.h
  Passes/InsertProfilePoint.cpp
  Passes/OptParentEnvironment.cpp
  Passes/PeepholeLowering.cpp
  Passes/ReorderRegisters.cpp
  LINK_OBJLIBS
  hermesBackend
  hermesInst
  hermesRegex
  hermesSourceMap
  hermesAST
)

add_hermes_library(hermesHBCBackendLean
  HBCStub.cpp
  BCProvider.cpp
  ConsecutiveStringStorage.cpp
  DebugInfo.cpp
  SimpleBytecodeBuilder.cpp
  UniquingFilenameTable.cpp
  LINK_OBJLIBS
  hermesSupport
)
