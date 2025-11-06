/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H
#define HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H

#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule.h"

namespace hermes {
namespace vm {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

struct RuntimeOffsets {
  static constexpr uint32_t stackPointer = offsetof(Runtime, stackPointer);
  static constexpr uint32_t registerStackEnd =
      offsetof(SHRuntime, registerStackEnd);
  static constexpr uint32_t currentFrame = offsetof(Runtime, currentFrame_);
  static constexpr uint32_t currentIP = offsetof(Runtime, currentIP_);
  static constexpr uint32_t globalObject = offsetof(Runtime, global_);
  static constexpr uint32_t thrownValue = offsetof(Runtime, thrownValue_);
  static constexpr uint32_t identifierTable =
      offsetof(Runtime, identifierTable_);
  static constexpr uint32_t shLocals = offsetof(Runtime, shLocals);

  using BuiltinsType = decltype(Runtime::builtins_);
  static constexpr uint32_t builtins = offsetof(Runtime, builtins_);

  static constexpr uint32_t nativeStackHigh =
      offsetof(Runtime, overflowGuard_) +
      offsetof(StackOverflowGuard, nativeStackHigh);
  static constexpr uint32_t nativeStackSize =
      offsetof(Runtime, overflowGuard_) +
      offsetof(StackOverflowGuard, nativeStackSize);

  static constexpr uint32_t codeBlockJitPtr = offsetof(CodeBlock, JITCompiled_);
  static constexpr uint32_t jsFunctionCodeBlock =
      offsetof(JSFunction, codeBlock_);

  static constexpr uint32_t runtimeModuleModuleCache =
      offsetof(RuntimeModule, moduleExports_);
  using RuntimeModuleObjectLiteralHiddenClassesType =
      decltype(RuntimeModule::objectLiteralHiddenClasses_);
  static constexpr uint32_t runtimeModuleObjectLiteralHiddenClasses =
      offsetof(RuntimeModule, objectLiteralHiddenClasses_);

  /// Can't use offsetof here because KindAndSize uses bitfields.
  static constexpr uint32_t kindAndSizeKind = KindAndSize::kNumSizeBits / 8;

  static constexpr uint32_t boxedDoubleValue = offsetof(BoxedDouble, value_);

  static constexpr uint32_t stringPrimitiveLengthAndFlags =
      offsetof(StringPrimitive, lengthAndFlags_);
  static constexpr uint32_t stringPrimitiveLengthMask =
      StringPrimitive::LENGTH_MASK;

  static constexpr uint32_t hiddenClassLazyJITId =
      offsetof(HiddenClass, lazyJITId_);
#ifdef HERMESVM_GC_HADES
  static constexpr uint32_t runtimeHadesYGLevel =
      offsetof(Runtime, heap_.youngGen_.level_);
  static constexpr uint32_t runtimeHadesYGEnd =
      offsetof(Runtime, heap_.youngGen_.effectiveEnd_);
  static constexpr uint32_t runtimeHadesOGMarkingBarriers =
      offsetof(Runtime, heap_.ogMarkingBarriers_);
#endif

#ifndef NDEBUG
  static constexpr uint32_t runtimeDebugAllocCounter =
      offsetof(Runtime, heap_.debugAllocationCounter_);
  static constexpr uint32_t gcCellDebugAllocId =
      offsetof(GCCell, _debugAllocationId_);
  static constexpr uint32_t gcCellMagicValue = GCCell::kMagic;
#endif

  static constexpr uint32_t runtimeRootClazzes =
      offsetof(Runtime, rootClazzes_);

  using IdentifierTableLookupEntryType = IdentifierTable::LookupEntry;
  using IdentifierTableLookupVectorType =
      decltype(IdentifierTable::lookupVector_);
  static constexpr uint32_t identifierTableLookupVector =
      offsetof(IdentifierTable, lookupVector_);
  static constexpr uint32_t identifierTableLookupEntrySize =
      sizeof(IdentifierTable::LookupEntry);
  static constexpr uint32_t identifierTableLookupEntryStrPrim =
      offsetof(IdentifierTable::LookupEntry, strPrim_);

  static constexpr uint32_t runtimeJitCounters =
      offsetof(Runtime, jitContext_.counters_);
};

#pragma GCC diagnostic pop

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H
