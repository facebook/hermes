# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(api_sources
  hermes.cpp
  DebuggerAPI.cpp
  )

file(GLOB api_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB api_public_headers ${PROJECT_SOURCE_DIR}/public/hermes/Public/*.h)

add_hermes_library(hermesapi
  ${api_sources}
  LINK_OBJLIBS hermesPublic LINK_LIBS jsi
  )
target_include_directories(hermesapi_obj PUBLIC ..)

add_hermes_library(hermesapiLean
        ${api_sources}
        LINK_OBJLIBS hermesPublic LINK_LIBS jsi)
target_include_directories(hermesapiLean_obj PUBLIC ..)
target_compile_definitions(hermesapiLean_obj PUBLIC HERMESVM_LEAN)

add_hermes_library(synthTrace hermes_tracing.cpp SynthTrace.cpp TracingRuntime.cpp
  LINK_OBJLIBS hermesapi)

add_hermes_library(timerStats TimerStats.cpp LINK_LIBS jsi LINK_OBJLIBS hermesSupport)

add_hermes_library(traceInterpreter TraceInterpreter.cpp
  LINK_OBJLIBS hermesapi hermesInstrumentation synthTrace synthTraceParser)

set(HERMES_ENABLE_RTTI OFF)

# synthTraceParser uses exceptions but not rtti
add_hermes_library(synthTraceParser SynthTraceParser.cpp LINK_OBJLIBS hermesSupport hermesParser synthTrace)

set(HERMES_ENABLE_EH OFF)

# compileJS uses neither exceptions nor RTTI
add_hermes_library(compileJS CompileJS.cpp LINK_OBJLIBS hermesPublic)
