/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/ModuleExportsCache.h"

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StaticHUtils.h"

namespace hermes::vm {

namespace {

/// Attempts to ensure that \p exports covers the index \p modIndex.
/// If successful:
///   Grows the array by at least a factor of 2 when it grows
///   If grows, writes the new array pointer back to \p exports.
///   Fills new elements with the empty HermesValue.
///   Returns true.
/// If unsuccessful:
///   Leaves the array unchanged.
///   Returns false.
bool ensureModuleExportCovered(
    Runtime &runtime,
    ArrayStorage *&exports,
    uint32_t modIndex) {
  // If \p modIndex is bigger than the largest possible cache size, don't
  // try to expand.
  if (modIndex >= ArrayStorage::maxElements()) {
    return false;
  }

  // If we need to alloc or resize, a size that makes modIndex a legal index.
  ArrayStorage::size_type newSize =
      static_cast<ArrayStorage::size_type>(modIndex) + 1;

  // First ensure that the moduleExports_ array is allocated.
  if (!exports) {
    // ArrayStorage::create only throws if the requested size is greater than
    // ArrayStorage::maxElements.  (Note that OOM is fatal error, not an
    // exception.)  The test above on modIndex proves that we avoid this case,
    // so no allocation failure can happen here.
    exports = vmcast<ArrayStorage>(
        runtime.ignoreAllocationFailure(ArrayStorage::create(runtime, newSize))
            .getObject(runtime));
  }

  if (modIndex < exports->size()) {
    return true;
  }

  struct : public Locals {
    PinnedValue<ArrayStorage> modExports;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  lv.modExports = exports;
  MutableHandle<ArrayStorage> modExportsHandle(lv.modExports);
  // If the new size is greater than the capacity, resize will attempt to
  // double the capacity, preventing frequent re-allocations.
  ExecutionStatus stat =
      ArrayStorage::resize(modExportsHandle, runtime, newSize);
  if (LLVM_UNLIKELY(stat == ExecutionStatus::EXCEPTION)) {
    // Discard the exception since this is purely for caching.
    runtime.clearThrownValue();
    return false;
  }
  // Write the possibly modified pointer value back.
  exports = lv.modExports.get();
  return true;
}

} // namespace

void module_export_cache::set(
    Runtime &runtime,
    ArrayStorage *&exports,
    uint32_t modIndex,
    Handle<> modExport) {
  if (LLVM_LIKELY(ensureModuleExportCovered(runtime, exports, modIndex))) {
    exports->set(modIndex, *modExport, runtime.getHeap());
  }
}

} // namespace hermes::vm
