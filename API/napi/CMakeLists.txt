# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesNapi
  hermes_napi.cpp
  hermes_napi_arraybuffer.cpp
  hermes_napi_async_context.cpp
  hermes_napi_async_work.cpp
  hermes_napi_bigint.cpp
  hermes_napi_buffer.cpp
  hermes_napi_dataview.cpp
  hermes_napi_error.cpp
  hermes_napi_external.cpp
  hermes_napi_function.cpp
  hermes_napi_object.cpp
  hermes_napi_promise.cpp
  hermes_napi_reference.cpp
  hermes_napi_scope.cpp
  hermes_napi_string.cpp
  hermes_napi_tsfn.cpp
  hermes_napi_typedarray.cpp
  hermes_napi_typetag.cpp
  hermes_napi_value.cpp
  hermes_napi_wrap.cpp
  LINK_OBJLIBS hermesVMRuntime hermesPublic
)
target_include_directories(hermesNapi_obj PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(hermesNapi_obj PRIVATE
  HERMES_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  HERMES_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  HERMES_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  NAPI_VERSION=10
)

add_hermes_library(hermesNapiCompile
  hermes_napi_compile.cpp
  LINK_OBJLIBS hermesVMRuntime hermesPublic)
target_include_directories(hermesNapiCompile_obj PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
# Needs access to NAPI internal headers.
target_include_directories(hermesNapiCompile_obj PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(hermesNapiCompile_obj PRIVATE
  NAPI_VERSION=10
)
