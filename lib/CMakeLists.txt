# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesOptimizer
  Optimizer/PassManager/PassManager.cpp
  Optimizer/PassManager/Pipeline.cpp
  Optimizer/Scalar/SimplifyCFG.cpp
  Optimizer/Scalar/CSE.cpp
  Optimizer/Scalar/CodeMotion.cpp
  Optimizer/Scalar/DCE.cpp
  Optimizer/Scalar/LowerGeneratorFunction.cpp
  Optimizer/Scalar/Mem2Reg.cpp
  Optimizer/Scalar/SimpleMem2Reg.cpp
  Optimizer/Scalar/FrameLoadStoreOpts.cpp
  Optimizer/Scalar/ScopeElimination.cpp
  Optimizer/Scalar/TypeInference.cpp
  Optimizer/Scalar/SimpleStackPromotion.cpp
  Optimizer/Scalar/ObjectMergeNewStores.cpp
  Optimizer/Scalar/ObjectStackPromotion.cpp
  Optimizer/Scalar/InstSimplify.cpp
  Optimizer/Scalar/Auditor.cpp
  Optimizer/Scalar/ResolveStaticRequire.cpp
  Optimizer/Scalar/FuncSigOpts.cpp
  Optimizer/Scalar/Utils.cpp
  Optimizer/Scalar/FunctionAnalysis.cpp
  Optimizer/Scalar/Inlining.cpp
  Optimizer/Scalar/HoistStartGenerator.cpp
  Optimizer/Scalar/InstructionEscapeAnalysis.cpp
  Optimizer/Scalar/TDZDedup.cpp
  Optimizer/Scalar/StripDebugInsts.cpp
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
  IRGen/ESTreeIRGen-typed-class.cpp
  IR/IR.cpp
  IR/CFG.cpp
  IR/IRBuilder.cpp
  IR/IRVerifier.cpp
  IR/Instrs.cpp
  IR/IRUtils.cpp
  Utils/Dumper.cpp
  LINK_OBJLIBS hermesSupport hermesFrontEndDefs hermesAST hermesSema hermesParser
)

add_subdirectory(VM)
add_subdirectory(Inst)
add_subdirectory(FrontEndDefs)
add_subdirectory(CompilerDriver)
add_subdirectory(DependencyExtractor)
add_subdirectory(ADT)
add_subdirectory(AST)
add_subdirectory(AST2JS)
add_subdirectory(Parser)
add_subdirectory(SourceMap)
add_subdirectory(Support)
add_subdirectory(BCGen)
add_subdirectory(ConsoleHost)
add_subdirectory(Regex)
add_subdirectory(Platform)
add_subdirectory(InternalJavaScript)
add_subdirectory(Sema)

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
  hermesSema_obj
  hermesSHBackend_obj
  hermesSourceMap_obj
  hermesSupport_obj
  zip_obj
  )

add_library(hermesvm_a STATIC)
target_link_libraries(hermesvm_a PUBLIC
  LLVHSupport_obj
  dtoa_obj
  hermesapi_obj
  hermesADT_obj
  hermesAST2JS_obj
  hermesAST_obj
  hermesBackend_obj
  hermesFrontEndDefs_obj
  hermesFrontend_obj
  hermesHBCBackend_obj
  hermesInst_obj
  hermesInstrumentation_obj
  hermesInternalJavaScript_obj
  hermesOptimizer_obj
  hermesParser_obj
  hermesPlatformUnicode_obj
  hermesPlatform_obj
  hermesPublic_obj
  hermesRegex_obj
  hermesSema_obj
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
  hermesapiLean_obj
  hermesADT_obj
  hermesFrontEndDefs_obj # needed by JIT
  hermesHBCBackendLean_obj
  hermesInst_obj
  hermesInstrumentation_obj
  hermesInternalUnit_obj
  hermesInternalJavaScript_obj
  hermesPlatformUnicode_obj
  hermesPlatform_obj
  hermesPublic_obj
  hermesRegex_obj
  hermesSupport_obj
  hermesVMRuntimeLean_obj
  zip_obj
  )

# CMake requires at least one file for the target, so create an empty file.
add_library(hermesvm SHARED dummy.cpp)
add_library(hermesvmlean SHARED dummy.cpp)

# Do not build a framework for the test suite.
if(!HERMES_ENABLE_TEST_SUITE)
  if(APPLE)
    set_target_properties(hermesvm PROPERTIES OUTPUT_NAME hermes)

    set_target_properties(hermesvm PROPERTIES
      FRAMEWORK TRUE
      VERSION ${PROJECT_VERSION}
      SOVERSION ${PROJECT_VERSION}
      FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}
      MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
      MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
      MACOSX_FRAMEWORK_IDENTIFIER dev.hermesengine.${HERMES_APPLE_TARGET_PLATFORM}
    )

    add_custom_command(TARGET hermesvm POST_BUILD
      COMMAND /usr/libexec/PlistBuddy -c "Add :MinimumOSVersion string ${CMAKE_OSX_DEPLOYMENT_TARGET}" $<TARGET_FILE_DIR:hermesvm>/Info.plist
    )

    install(TARGETS hermesvm
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      FRAMEWORK DESTINATION Library/Frameworks/${HERMES_APPLE_TARGET_PLATFORM}
    )

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/API/hermes" DESTINATION include
      FILES_MATCHING PATTERN "*.h"
      PATTERN "synthtest" EXCLUDE)
  endif()

  # Create debug symbols (dSYM) bundle for Apple platform dylibs/frameworks
  # Largely inspired by https://github.com/llvm/llvm-project/blob/6701993027f8af172d7ba697884459261b00e3c6/llvm/cmake/modules/AddLLVM.cmake#L1934-L1986
  if(HERMES_BUILD_APPLE_DSYM)
    if(CMAKE_CXX_FLAGS MATCHES "-flto")
      set(lto_object ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/hermesvm-lto.o)
      set_property(TARGET hermesvm APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-object_path_lto,${lto_object}")
    endif()

    get_target_property(DSYM_PATH hermesvm LOCATION)
    if(HERMES_BUILD_APPLE_FRAMEWORK)
      get_filename_component(DSYM_PATH ${DSYM_PATH} DIRECTORY)
    endif()
    set(DSYM_PATH "${DSYM_PATH}.dSYM")

    if(NOT CMAKE_DSYMUTIL)
      set(CMAKE_DSYMUTIL xcrun dsymutil)
    endif()
    add_custom_command(TARGET hermesvm POST_BUILD
      COMMAND ${CMAKE_DSYMUTIL} $<TARGET_FILE:hermesvm> --out ${DSYM_PATH}
      BYPRODUCTS ${DSYM_PATH}
    )

    if(HERMES_BUILD_APPLE_FRAMEWORK)
      install(DIRECTORY ${DSYM_PATH} DESTINATION Library/Frameworks/${HERMES_APPLE_TARGET_PLATFORM})
    else()
      install(DIRECTORY ${DSYM_PATH} DESTINATION lib)
    endif()
  endif()
endif()

# Force the entire hermesvm_a library to be used by hermesvm. There is
# no cross platform way to do this (force_load is for macOS). Once we upgrade
# CMake 3.24, we can use the LINK_LIBRARY generator expression to do this.
# We use private dependencies for the shared library target, in order to avoid
# having its dependencies added to the link line of things that depend on it.
# That is, we do not want libraries that depend on hermesvm to link in an extra
# copy of any static libraries linked into it.
if(APPLE)
  target_link_libraries(hermesvm PRIVATE -force_load hermesvm_a)
  target_link_libraries(hermesvmlean PRIVATE -force_load hermesvmlean_a)
else()
  target_link_libraries(hermesvm PRIVATE
    -Wl,--whole-archive
    hermesvm_a
    -Wl,--no-whole-archive
  )
  target_link_libraries(hermesvmlean PRIVATE
    -Wl,--whole-archive
    hermesvmlean_a
    -Wl,--no-whole-archive
  )
endif()

# Since we use private dependencies above, we need to enumerate all the headers
# that should be available to consumers of hermesvm. Note that we only expose
# the headers which produce visible symbols.
target_include_directories(hermesvm PUBLIC ${CMAKE_SOURCE_DIR}/public ${CMAKE_SOURCE_DIR}/API ${HERMES_JSI_DIR})

# Private dependencies also mean that JSI will not get propagated to libraries
# that depend on hermesvm. If JSI is built as a shared library, ensure it is
# added to the link line for anything that depends on hermesvm. If it's a static
# library, ensure it isn't since that would link in JSI twice.
if (HERMES_BUILD_SHARED_JSI)
  target_link_libraries(hermesvm PUBLIC jsi)
endif()

# Apply additional linker flags.
target_link_options(hermesvm PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})
target_link_options(hermesvmlean PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

if(HERMES_ENABLE_INTL)
  set(INTL_OBJ_LIBS hermesPlatformIntl_obj hermesBCP47Parser_obj)
  target_link_libraries(hermesvm_a PUBLIC ${INTL_OBJ_LIBS})
  target_link_libraries(hermesvmlean_a PUBLIC ${INTL_OBJ_LIBS})
endif()

# Construct the HermesVM model string
set(HERMESVM_MODEL "")

if (HERMESVM_COMPRESSED_POINTERS)
  string(APPEND HERMESVM_MODEL "_cp")
endif()
if (HERMESVM_BOXED_DOUBLES)
  string(APPEND HERMESVM_MODEL "_bd")
endif()
if (HERMESVM_CONTIGUOUS_HEAP)
  string(APPEND HERMESVM_MODEL "_ch")
endif()
string(APPEND HERMESVM_MODEL "_s" ${HERMESVM_LOG_HEAP_SEGMENT_SIZE})
# The pointer size is not strictly necessary, but include it for completeness.
string(APPEND HERMESVM_MODEL "_p" ${CMAKE_SIZEOF_VOID_P})

configure_file(config/libhermesvm-config.h.in config/libhermesvm-config.h)
