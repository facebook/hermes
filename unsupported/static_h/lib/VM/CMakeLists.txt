# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  ArrayStorage.cpp
  BasicBlockExecutionInfo.cpp
  BigIntPrimitive.cpp
  BoxedDouble.cpp
  BuildMetadata.cpp
  Callable.cpp
  CellKind.cpp
  CheckHeapWellFormedAcceptor.cpp
  CodeBlock.cpp
  DictPropertyMap.cpp
  Domain.cpp
  DummyObject.cpp
  GCBase.cpp
  OrderedHashMap.cpp
  HandleRootOwner.cpp
  HeapSnapshot.cpp
  HermesValue.cpp
  HiddenClass.cpp
  IdentifierTable.cpp
  Interpreter.cpp InstLayout.inc Interpreter-slowpaths.cpp
  JSArray.cpp
  JSArrayBuffer.cpp
  JSCallSite.cpp
  JSCallableProxy.cpp
  JSDataView.cpp
  JSDate.cpp
  JSError.cpp
  JSGenerator.cpp
  JSObject.cpp
  JSProxy.cpp
  JSRegExp.cpp
  JSRegExpStringIterator.cpp
  JSMapImpl.cpp
  JSNativeFunctions.cpp
  JSTypedArray.cpp
  JSWeakMapImpl.cpp
  JSWeakRef.cpp
  LimitedStorageProvider.cpp
  DecoratedObject.cpp
  HostModel.cpp
  NativeState.cpp
  Operations.cpp
  PredefinedStringIDs.cpp
  PrimitiveBox.cpp
  PropertyAccessor.cpp
  Runtime.cpp Runtime-profilers.cpp
  RuntimeModule.cpp
  Profiler/ChromeTraceSerializerPosix.cpp
  Profiler/CodeCoverageProfiler.cpp
  Profiler/InlineCacheProfiler.cpp
  Profiler/SamplingProfilerPosix.cpp
  SegmentedArray.cpp
  SerializedLiteralParser.cpp
  SingleObject.cpp
  StackFrame.cpp
  StackTracesTree.cpp
  StorageProvider.cpp
  StringPrimitive.cpp
  StringView.cpp
  SymbolRegistry.cpp
  TimeLimitMonitor.cpp
  TwineChar16.cpp
  StringRefUtils.cpp
  VTable.cpp
  Metadata.cpp

  detail/IdentifierHashTable.cpp

  JSLib/Array.cpp
  JSLib/ArrayBuffer.cpp
  JSLib/ArrayIterator.cpp
  JSLib/AsyncFunction.cpp
  JSLib/BigInt.cpp
  JSLib/CallSite.cpp
  JSLib/DataView.cpp
  JSLib/TypedArray.cpp
  JSLib/Error.cpp
  JSLib/GeneratorFunction.cpp
  JSLib/GeneratorPrototype.cpp
  JSLib/GlobalObject.cpp
  JSLib/IteratorPrototype.cpp
  JSLib/HermesInternal.cpp
  JSLib/HermesBuiltin.cpp
  JSLib/Instrument.cpp
  JSLib/Intl.cpp
  JSLib/JSLibInternal.cpp JSLib/JSLibInternal.h
  JSLib/RuntimeCommonStorage.cpp
  JSLib/Map.cpp
  JSLib/Math.cpp
  JSLib/JSON.cpp
  JSLib/RuntimeJSONUtils.cpp
  JSLib/JSONLexer.cpp
  JSLib/Object.cpp
  JSLib/Proxy.cpp
  JSLib/Reflect.cpp
  JSLib/Set.cpp
  JSLib/String.cpp
  JSLib/StringIterator.cpp
  JSLib/Function.cpp
  JSLib/Number.cpp
  JSLib/Boolean.cpp
  JSLib/RegExp.cpp
  JSLib/RegExpStringIterator.cpp
  JSLib/DateUtil.cpp
  JSLib/Sorting.cpp
  JSLib/Symbol.cpp
  JSLib/Date.cpp JSLib/DateUtil.cpp
  JSLib/WeakMap.cpp
  JSLib/WeakRef.cpp
  JSLib/WeakSet.cpp
  JSLib/print.cpp
  JSLib/eval.cpp
  JSLib/escape.cpp
  JSLib/require.cpp
)

if (${HERMESVM_GCKIND} STREQUAL "MALLOC")
  list(APPEND source_files gcs/MallocGC.cpp gcs/FillerCell.cpp)
elseif (${HERMESVM_GCKIND} STREQUAL "HADES")
  list(APPEND source_files
            gcs/AlignedHeapSegment.cpp
            gcs/AlignedStorage.cpp
            gcs/CardTableNC.cpp
            gcs/FillerCell.cpp
            gcs/HadesGC.cpp
  )
elseif (${HERMESVM_GCKIND} STREQUAL "RUNTIME")
  list(APPEND source_files
          gcs/AlignedHeapSegment.cpp gcs/AlignedStorage.cpp
          gcs/CardTableNC.cpp gcs/FillerCell.cpp
          gcs/HadesGC.cpp
  )
else()
  message(WARNING "Not linking garbage collector")
endif()

list(APPEND source_files
  Debugger/Debugger.cpp
  JSLib/DebuggerInternal.cpp
)

# HostModel.cpp defines an abstract base class HostObjectProxy.
# This can be (and is) implemented by code which uses rtti, and
# therefore expects the base class to have typeinfo, so
# HostModel.cpp must be compiled -frtti, unlike most of the VM.
# DecoratedObject::Decoration requires it as well, as it is the
# base class for HostObjectProxy.
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set_source_files_properties(DecoratedObject.cpp HostModel.cpp PROPERTIES COMPILE_FLAGS -frtti)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  set_source_files_properties(DecoratedObject.cpp HostModel.cpp PROPERTIES COMPILE_FLAGS /GR)
endif()

set(link_libs
  hermesADT
  hermesInst
  hermesSupport
  hermesPlatform
  hermesInternalBytecode
  hermesPublic
  dtoa
)

if(HERMES_ENABLE_INTL)
  list(APPEND link_libs hermesPlatformIntl)
endif()

add_hermes_library(hermesVMRuntime STATIC ${source_files}
    LINK_LIBS
    ${link_libs}
    hermesHBCBackend
    hermesParser
)

add_hermes_library(hermesVMRuntimeLean STATIC ${source_files}
    LINK_LIBS
    ${link_libs}
    hermesHBCBackendLean
)
target_compile_definitions(hermesVMRuntimeLean PUBLIC HERMESVM_LEAN)

add_subdirectory(Instrumentation)
