# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesBackend
  RegAlloc.cpp
  BackendContext.cpp
  Lowering.cpp
  LowerBuiltinCalls.cpp
  Exceptions.cpp
  LIRPeephole.cpp
  SerializedLiteralGenerator.cpp
  SerializedLiteralParserBase.cpp
  LINK_OBJLIBS hermesFrontend hermesOptimizer
)

add_hermes_library(hermesBackendLean
  SerializedLiteralParserBase.cpp
  LINK_OBJLIBS
  hermesSupport
  hermesPublic
)

target_compile_definitions(hermesBackendLean_obj PUBLIC HERMESVM_LEAN)

add_subdirectory(HBC)
add_subdirectory(SH)
