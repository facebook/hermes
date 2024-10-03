# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(NO_EH_RTTI_SOURCES
  CDPJSONHelpers.cpp
  )
set(APITestsSources
  APITest.cpp
  APITestFactory.cpp
  # ${HERMES_JSI_DIR}/jsi/test/testlib.cpp # Should we remove jsi dir?
  AsyncDebuggerAPITest.cpp
  CDPAgentTest.cpp
  ConsoleMessageTest.cpp
  DomainStateTest.cpp
  SynchronizedCallbackTest.cpp
  DebuggerTest.cpp
  # SegmentTest.cpp
  HeapSnapshotAPITest.cpp
  SynthTraceTest.cpp
  SynthTraceParserTest.cpp
  SynthTraceSerializationTest.cpp
  TimerStatsTest.cpp
  ${NO_EH_RTTI_SOURCES}
  )
set(APISegmentTestCompileSources
  SegmentTestCompile.cpp
  )

if (NOT HERMES_ENABLE_EH AND NOT HERMES_ENABLE_RTTI)
  if (GCC_COMPATIBLE)
    set_property(SOURCE ${NO_EH_RTTI_SOURCES} APPEND_STRING
      PROPERTY COMPILE_FLAGS "-fno-exceptions -fno-rtti")
  elseif (MSVC)
    set_property(SOURCE ${NO_EH_RTTI_SOURCES} APPEND_STRING
      PROPERTY COMPILE_FLAGS "/EHs-c- /GR-")
  endif ()
endif ()

# Build SegmentTestCompile without EH and RTTI
# TODO: Fix the compilation error in SH
# add_hermes_library(SegmentTestCompile ${APISegmentTestCompileSources} LINK_OBJLIBS hermesHBCBackend)

# Turn on EH and RTTI for APITests
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
# For some reason (bug?) add_unittest() is clearing LLVM_REQUIRES_RTTI, so
# we need to set this one.
set(LLVM_ENABLE_RTTI ON)

add_hermes_unittest(APITests ${APITestsSources})

target_link_libraries(APITests
  compileJS
  traceInterpreter
  timerStats
  hermesvm_a
  hermesSandboxRuntime
  cdpInternal
  # SegmentTestCompile # Commenting out for now
)

add_hermes_unittest(APILeanTests APILeanTest.cpp)
target_link_libraries(APILeanTests hermesvmlean_a)
