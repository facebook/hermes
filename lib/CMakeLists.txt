# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesOptimizer
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
  LINK_OBJLIBS hermesSupport hermesFrontEndDefs hermesAST hermesParser
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

# Define combined libraries from all components. Components are defined as object
# libraries. It is tricky.
#
# The problem is that most of the componenents have dependencies on other components.
# We can't simply add them as LINK_LIBRARIES because that transfers their tranisitive
# dependency metadata to the combined library. The combined library ends up
# containing the dependency and simultaneously depending on it.
# So, we are forced to include just dependencies using $<TARGET_OBJECTS:component>.
# But then theere is anothet problem: the TARGET_OBJECTS approach doesn't transfer
# any metadata at all (include paths, external dependencies, etc).
# Fortunately most components don't have external dependencies or include paths,
# but some do.
#
# Our solution. If a component has external dependencies or other metadata we
# need, we make sure it has not internal dependencies and add it to LINK_LIBRARIES
# (this could make it harder to use in tests, but such is life).
# The rest of the components we add with the TARGET_OBJECTS approach.

add_library(hermescompiler STATIC)
target_link_libraries(hermescompiler PUBLIC
  LLVHSupport_obj
  dtoa_obj
  hermesADT_obj
  hermesAST2JS_obj
  hermesAST_obj
  hermesBackend_obj
  hermesFrontEndDefs_obj
  hermesFrontend_obj
  hermesHBCBackend_obj
  hermesInst_obj
  hermesOptimizer_obj
  hermesParser_obj
  hermesPlatformUnicode_obj
  hermesPlatform_obj
  hermesPublic_obj
  hermesRegex_obj
  hermesSHBackend_obj
  hermesSourceMap_obj
  hermesSupport_obj
  zip_obj
  )

add_library(hermesvm_a STATIC)
target_link_libraries(hermesvm_a PUBLIC
  LLVHSupport_obj
  dtoa_obj
  hermesADT_obj
  hermesAST2JS_obj
  hermesAST_obj
  hermesBackend_obj
  hermesFrontEndDefs_obj
  hermesFrontend_obj
  hermesHBCBackend_obj
  hermesInst_obj
  hermesInstrumentation_obj
  hermesInternalBytecode_obj
  hermesOptimizer_obj
  hermesParser_obj
  hermesPlatformUnicode_obj
  hermesPlatform_obj
  hermesPublic_obj
  hermesRegex_obj
  hermesSHBackend_obj
  hermesSourceMap_obj
  hermesSupport_obj
  hermesVMRuntime_obj
  zip_obj
  )

add_library(hermesvmlean_a STATIC)
target_link_libraries(hermesvmlean_a PUBLIC
  LLVHSupport_obj
  dtoa_obj
  hermesADT_obj
  hermesHBCBackendLean_obj
  hermesInst_obj
  hermesInstrumentation_obj
  hermesInternalBytecode_obj
  hermesPlatformUnicode_obj
  hermesPlatform_obj
  hermesPublic_obj
  hermesRegex_obj
  hermesSupport_obj
  hermesVMRuntimeLean_obj
  zip_obj
  )

if(HERMES_ENABLE_INTL)
  set(INTL_OBJ_LIBS hermesPlatformIntl_obj hermesBCP47Parser_obj)
  target_link_libraries(hermesvm_a PUBLIC ${INTL_OBJ_LIBS})
  target_link_libraries(hermesvmlean_a PUBLIC ${INTL_OBJ_LIBS})
endif()

configure_file(config/libhermesvm-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config/libhermesvm-config.h)
