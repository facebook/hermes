# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)

add_hermes_library(hermesNodeApiJsiRuntime
    NodeApiJsiRuntime.cpp
    ApiLoaders/HermesApi.cpp
    ApiLoaders/HermesApi.h
    ApiLoaders/JSRuntimeApi.cpp
    ApiLoaders/JSRuntimeApi.h
    ApiLoaders/NodeApi_win.cpp
    ApiLoaders/NodeApi.cpp
    ApiLoaders/NodeApi.h
  LINK_LIBS
    hermesRuntimeApi
    jsi
    nodeApi 
  )
target_include_directories(hermesNodeApiJsiRuntime PUBLIC ..)
