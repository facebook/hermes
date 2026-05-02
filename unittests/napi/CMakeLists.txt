# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Turn on EH and RTTI for test code (gtest requires them).
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
set(LLVM_ENABLE_RTTI ON)

# Build a test NAPI addon as a shared library for module loading tests.
# The addon is compiled against the NAPI headers only — it does NOT
# link against the Hermes VM. NAPI symbols are resolved at load time
# from the host executable (the test binary).
add_library(napi_test_addon SHARED test_addon.c)
target_include_directories(napi_test_addon PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)
set_target_properties(napi_test_addon PROPERTIES
  PREFIX ""  # No "lib" prefix — matches Node.js addon convention.
  SUFFIX ".node"
)
# On macOS, use -undefined dynamic_lookup so the addon can reference
# NAPI symbols that will be provided by the host process at load time.
if(APPLE)
  target_link_options(napi_test_addon PRIVATE
    -undefined dynamic_lookup
  )
endif()

# Reference test addon with add(a,b) and hello() functions.
# Source lives in test/napi/test_addon/ for use by lit tests.
add_library(napi_ref_addon SHARED
  ${PROJECT_SOURCE_DIR}/test/napi/test_addon/binding.c
)
target_include_directories(napi_ref_addon PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)
set_target_properties(napi_ref_addon PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
if(APPLE)
  target_link_options(napi_ref_addon PRIVATE
    -undefined dynamic_lookup
  )
endif()

# TextEncoder NAPI addon for proving-ground tests.
add_library(napi_text_encoder_addon SHARED
  ${PROJECT_SOURCE_DIR}/test/napi/text_encoder_addon/text_encoder.cpp
)
target_include_directories(napi_text_encoder_addon PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)
set_target_properties(napi_text_encoder_addon PROPERTIES
  PREFIX ""
  SUFFIX ".node"
)
if(APPLE)
  target_link_options(napi_text_encoder_addon PRIVATE
    -undefined dynamic_lookup
  )
endif()

add_hermes_unittest(NapiTests
  NapiAddFinalizerTest.cpp
  NapiArrayBufferTest.cpp
  NapiAsyncContextTest.cpp
  NapiAsyncWorkTest.cpp
  NapiBigIntTest.cpp
  NapiCallbackScopeTest.cpp
  NapiCleanupHookTest.cpp
  NapiExternalArrayBufferTest.cpp
  NapiCallFunctionTest.cpp
  NapiCoercionTest.cpp
  NapiComparisonTest.cpp
  NapiCreateFunctionTest.cpp
  NapiDataViewTest.cpp
  NapiDefineClassTest.cpp
  NapiEnvTest.cpp
  NapiExceptionTest.cpp
  NapiExternalTest.cpp
  NapiFunctionTest.cpp
  NapiHandleScopeTest.cpp
  NapiInstanceDataTest.cpp
  NapiModuleLoadTest.cpp
  NapiModuleTest.cpp
  NapiNewInstanceTest.cpp
  NapiObjectTest.cpp
  NapiPromiseTest.cpp
  NapiReferenceTest.cpp
  NapiRunScriptTest.cpp
  NapiStringTest.cpp
  NapiTypedArrayTest.cpp
  NapiTypeTagTest.cpp
  NapiValueTest.cpp
  NapiWrapTest.cpp
  LINK_LIBS hermesNapi hermesvm_a
)

# Allow tests to include hermes_napi_impl.h (the internal header).
target_include_directories(NapiTests PRIVATE
  ${PROJECT_SOURCE_DIR}/API/napi
)

# Export NAPI symbols from the test binary so dlopen'd addons can
# resolve them. On macOS, -export_dynamic exports all symbols from the
# main executable to dlopen'd libraries. On Linux, -rdynamic does the
# equivalent.
if(APPLE)
  target_link_options(NapiTests PRIVATE
    -Wl,-export_dynamic
  )
elseif(UNIX)
  target_link_options(NapiTests PRIVATE
    -rdynamic
  )
endif()

# Pass the test addon path so NapiModuleLoadTest can find it.
add_dependencies(NapiTests napi_test_addon)
target_compile_definitions(NapiTests PRIVATE
  NAPI_TEST_ADDON_PATH="$<TARGET_FILE:napi_test_addon>"
)
