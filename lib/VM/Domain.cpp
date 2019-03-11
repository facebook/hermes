/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Domain.h"

#include "hermes/VM/GCPointer-inline.h"

namespace hermes {
namespace vm {

VTable Domain::vt{CellKind::DomainKind,
                  sizeof(Domain),
                  _finalizeImpl,
                  _markWeakImpl,
                  _mallocSizeImpl};

void DomainBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Domain *>(cell);
  mb.addField("@cjsModules", &self->cjsModules_);
}

PseudoHandle<Domain> Domain::create(Runtime *runtime) {
  void *mem =
      runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(sizeof(Domain));
  auto self = createPseudoHandle(new (mem) Domain(runtime));
  return self;
}

void Domain::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<Domain>(cell);
  self->~Domain();
}

Domain::~Domain() {
  for (RuntimeModule *rm : runtimeModules_) {
    delete rm;
  }
}

void Domain::_markWeakImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast_during_gc<Domain>(cell, gc);
  self->markWeakRefs(gc);
}

void Domain::markWeakRefs(GC *gc) {
  for (RuntimeModule *rm : runtimeModules_) {
    rm->markDomainRef(gc);
  }
}

size_t Domain::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<Domain>(cell);
  return self->cjsModuleTable_.getMemorySize() +
      self->runtimeModules_.capacity_in_bytes();
}

ExecutionStatus Domain::importCJSModuleTable(
    Handle<Domain> self,
    Runtime *runtime,
    RuntimeModule *runtimeModule,
    uint32_t cjsModuleOffset) {
  if (runtimeModule->getBytecode()->getCJSModuleTable().empty() &&
      runtimeModule->getBytecode()->getCJSModuleTableStatic().empty()) {
    // Nothing to do, avoid allocating and simply return.
    return ExecutionStatus::RETURNED;
  }

  const uint64_t newModules =
      runtimeModule->getBytecode()->getCJSModuleTable().size() +
      runtimeModule->getBytecode()->getCJSModuleTableStatic().size();

  assert(
      newModules <= std::numeric_limits<uint32_t>::max() &&
      "number of modules is 32 bits due to the bytecode format");

  static_assert(
      CJSModuleSize < 10, "CJSModuleSize must be small to avoid overflow");

  // Use uint64_t to allow us to check for overflow.
  const uint64_t requiredSize = (cjsModuleOffset + newModules) * CJSModuleSize;
  if (requiredSize > std::numeric_limits<uint32_t>::max()) {
    return runtime->raiseRangeError("Loaded module count exceeded limit");
  }

  MutableHandle<ArrayStorage> cjsModules{runtime};

  if (!self->cjsModules_) {
    // Create the module table on first import.
    // Ensure the size and capacity are set correctly to allow for directly
    // setting the values necessary.
    auto cjsModulesRes =
        ArrayStorage::create(runtime, requiredSize, requiredSize);
    if (LLVM_UNLIKELY(cjsModulesRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    cjsModules = vmcast<ArrayStorage>(*cjsModulesRes);
  } else {
    cjsModules = self->cjsModules_;
    // Resize the array to allow for the new modules, if necessary.
    if (requiredSize > self->cjsModules_->size()) {
      if (LLVM_UNLIKELY(
              ArrayStorage::resize(cjsModules, runtime, requiredSize) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  assert(cjsModules && "cjsModules not set");

  uint32_t index = cjsModuleOffset * CJSModuleSize;

  /// Push element \param val onto cjsModules and increment index.
  const auto pushNoError = [runtime, &cjsModules, &index](HermesValue val) {
    cjsModules->at(index).set(val, &runtime->getHeap());
    ++index;
  };

  // Import full table that allows dynamic requires.
  for (const auto &pair : runtimeModule->getBytecode()->getCJSModuleTable()) {
    const auto startIndex = index;

    pushNoError(HermesValue::encodeEmptyValue());
    pushNoError(HermesValue::encodeObjectValue(nullptr));
    pushNoError(HermesValue::encodeNativeUInt32(pair.second));
    pushNoError(HermesValue::encodeNativePointer(runtimeModule));

    auto result = self->cjsModuleTable_.try_emplace(
        runtimeModule->getSymbolIDFromStringID(pair.first), startIndex);
    (void)result;
    assert(result.second && "Duplicate CJS modules");
  }

  // Import table to be used for requireFast.
  for (const auto &functionID :
       runtimeModule->getBytecode()->getCJSModuleTableStatic()) {
    pushNoError(HermesValue::encodeEmptyValue());
    pushNoError(HermesValue::encodeObjectValue(nullptr));
    pushNoError(HermesValue::encodeNativeUInt32(functionID));
    pushNoError(HermesValue::encodeNativePointer(runtimeModule));
  }

  self->cjsModules_.set(cjsModules.get(), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

} // namespace vm
} // namespace hermes
