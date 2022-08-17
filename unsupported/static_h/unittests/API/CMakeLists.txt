# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(APITestsSources
  APITest.cpp
  APITestFactory.cpp
  DebuggerTest.cpp
  SegmentTest.cpp
  HeapSnapshotAPITest.cpp
  SynthTraceTest.cpp
  SynthTraceParserTest.cpp
  SynthTraceSerializationTest.cpp
  TimerStatsTest.cpp
  )
set(APISegmentTestCompileSources
  SegmentTestCompile.cpp
  )

set(HERMES_LINK_COMPONENTS LLVHSupport)

# Build SegmentTestCompile without EH and RTTI
add_hermes_library(SegmentTestCompile STATIC ${APISegmentTestCompileSources} LINK_LIBS hermesHBCBackend)

# Turn on EH and RTTI for APITests
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
# For some reason (bug?) add_unittest() is clearing LLVM_REQUIRES_RTTI, so
# we need to set this one.
set(LLVM_ENABLE_RTTI ON)

add_hermes_unittest(APITests ${APITestsSources})

target_link_libraries(APITests hermesapi compileJS SegmentTestCompile traceInterpreter timerStats)

add_hermes_unittest(APILeanTests APILeanTest.cpp)
target_link_libraries(APILeanTests hermesapiLean)
