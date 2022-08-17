# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_library(hermesOptimizer
  STATIC
  Optimizer/PassManager/Pipeline.cpp
  Optimizer/Scalar/SimplifyCFG.cpp
  Optimizer/Scalar/CSE.cpp
  Optimizer/Scalar/CodeMotion.cpp
  Optimizer/Scalar/DCE.cpp
  Optimizer/Scalar/Mem2Reg.cpp
  Optimizer/Scalar/TypeInference.cpp
  Optimizer/Scalar/StackPromotion.cpp
  Optimizer/Scalar/InstSimplify.cpp
  Optimizer/Scalar/Auditor.cpp
  Optimizer/Wasm/WasmSimplify.cpp
  Optimizer/Scalar/SimpleCallGraphProvider.cpp
  Optimizer/Scalar/ResolveStaticRequire.cpp
  Optimizer/Scalar/SimpleCallGraphProvider.cpp
  Optimizer/Scalar/FuncSigOpts.cpp
  Optimizer/Scalar/Utils.cpp
  Optimizer/Scalar/Inlining.cpp
  Optimizer/Scalar/HoistStartGenerator.cpp
  Optimizer/Scalar/InstructionEscapeAnalysis.cpp
  Optimizer/Scalar/TDZDedup.cpp
  IR/Analysis.cpp
  IR/IREval.cpp
)

add_hermes_library(hermesFrontend
  STATIC
  IRGen/IRGen.cpp
  IRGen/ESTreeIRGen.cpp IRGen/ESTreeIRGen.h
  IRGen/ESTreeIRGen-expr.cpp
  IRGen/ESTreeIRGen-stmt.cpp
  IRGen/ESTreeIRGen-func.cpp
  IRGen/ESTreeIRGen-except.cpp
  IRGen/IRInstrument.cpp
  IR/IR.cpp
  IR/CFG.cpp
  IR/IRBuilder.cpp
  IR/IRVerifier.cpp
  IR/Instrs.cpp
  Utils/Dumper.cpp
  Optimizer/Wasm/WasmIntrinsics.cpp
  Optimizer/Wasm/EmitWasmIntrinsics.cpp
  LINK_LIBS hermesSupport hermesFrontEndDefs hermesAST hermesParser
)

add_subdirectory(VM)
add_subdirectory(Inst)
add_subdirectory(FrontEndDefs)
add_subdirectory(CompilerDriver)
add_subdirectory(DependencyExtractor)
add_subdirectory(ADT)
add_subdirectory(AST)
add_subdirectory(AST2JS)
add_subdirectory(FlowParser)
add_subdirectory(Parser)
add_subdirectory(SourceMap)
add_subdirectory(Support)
add_subdirectory(BCGen)
add_subdirectory(ConsoleHost)
add_subdirectory(Regex)
add_subdirectory(Platform)
add_subdirectory(InternalBytecode)
