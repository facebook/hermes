/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HadesGC.h"

namespace hermes {
namespace vm {

HadesGC::HadesGC(
    MetadataTable metaTable,
    GCCallbacks *gcCallbacks,
    PointerBase *pointerBase,
    const GCConfig &gcConfig,
    std::shared_ptr<CrashManager> crashMgr,
    std::shared_ptr<StorageProvider>)
    : GCBase(
          metaTable,
          gcCallbacks,
          pointerBase,
          gcConfig,
          std::move(crashMgr)) {}

HadesGC::~HadesGC() {}

// TODO: Fill these out
void HadesGC::getHeapInfoWithMallocSize(HeapInfo &info) {}
void HadesGC::getCrashManagerHeapInfo(CrashManager::HeapInformation &info) {}
void HadesGC::createSnapshot(llvm::raw_ostream &os) {}

void HadesGC::collect() {}

void HadesGC::finalizeAll() {}

// TODO: Inline perf check for all write barrier code.
void HadesGC::writeBarrier(void *loc, HermesValue value) {}
void HadesGC::writeBarrier(void *loc, void *value) {}
void HadesGC::writeBarrierRange(GCHermesValue *start, uint32_t numHVs) {}

bool HadesGC::canAllocExternalMemory(uint32_t size) {
  return false;
}

void HadesGC::markSymbol(SymbolID symbolID) {}

WeakRefSlot *HadesGC::allocWeakSlot(HermesValue init) {
  return nullptr;
}

void HadesGC::forAllObjs(const std::function<void(GCCell *)> &callback) {}

#ifndef NDEBUG

bool HadesGC::validPointer(const void *ptr) const {
  return false;
}

bool HadesGC::dbgContains(const void *ptr) const {
  return false;
}

void HadesGC::trackReachable(CellKind kind, unsigned sz) {}

size_t HadesGC::countUsedWeakRefs() const {
  return 0;
}

bool HadesGC::isMostRecentFinalizableObj(const GCCell *cell) const {
  return false;
}

#endif

} // namespace vm
} // namespace hermes
