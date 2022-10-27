# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

# Some sources here have conflicting names for free functions, so disable unity
# builds if they have been globally enabled.
set(CMAKE_UNITY_BUILD OFF)

set(RTSources
  AlignedHeapSegmentTest.cpp
  AlignedStorageTest.cpp
  ArrayTest.cpp
  ArrayStorageTest.cpp
  BigIntPrimitiveTest.cpp
  BytecodeProviderTest.cpp
  CallResultTest.cpp
  CardObjectBoundaryNCTest.cpp
  CardTableNCTest.cpp
  CastingTest.cpp
  CodeCoverageProfilerTest.cpp
  CopyableVectorTest.cpp
  CrashManagerTest.cpp
  DateUtilTest.cpp
  DecoratedObjectTest.cpp
  DictPropertyMapTest.cpp
  GCBasicsTest.cpp
  GCFinalizerTest.cpp
  GCFragmentationTest.cpp
  GCGuardPageNCTest.cpp
  GCInitTest.cpp
  GCLazySegmentNCTest.cpp
  GCMarkWeakTest.cpp
  GCObjectIterationTest.cpp
  GCOOMTest.cpp
  GCReturnUnusedMemoryTest.cpp
  GCSanitizeHandlesTest.cpp
  HeapSnapshotTest.cpp
  HermesValueTest.cpp
  HiddenClassTest.cpp
  IdentifierTableTest.cpp
  InstrumentationAPITest.cpp
  InternalPropertiesTest.cpp
  InterpreterTest.cpp
  IRInstrumentationTest.cpp
  JSLibTest.cpp
  JSONParseTest.cpp
  JSTypedArrayTest.cpp
  NativeFrameTest.cpp
  NativeFunctionTest.cpp
  NativeStateTest.cpp
  MarkBitArrayNCTest.cpp
  ObjectBufferTest.cpp
  ObjectModelTest.cpp
  OperationsTest.cpp
  PredefinedStringsTest.cpp
  HandleTest.cpp
  RuntimeConfigTest.cpp
  SamplingHeapProfilerTest.cpp
  SamplingProfilerPosixTest.cpp
  SegmentedArrayTest.cpp
  SmallHermesValueTest.cpp
  SmallXStringTest.cpp
  StaticBuiltinsTest.cpp
  StackTracesTreeTest.cpp
  StorageProviderTest.cpp
  StringBuilderTest.cpp
  StringPrimitiveTest.cpp
  StringViewTest.cpp
  SymbolIDTest.cpp
  TestHelpers.cpp TestHelpers.h
  TestHelpers1.cpp TestHelpers1.h
  TwineChar16Test.cpp
  WeakValueMapTest.cpp
  MetadataTest.cpp
  )

add_hermes_unittest(HermesVMRuntimeTests
  ${RTSources}
  )

target_link_libraries(HermesVMRuntimeTests
  hermesVMRuntime
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
  dtoa
 )

add_subdirectory(Instrumentation)
