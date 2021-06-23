/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Domain.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

const VTable Domain::vt{
    CellKind::DomainKind,
    cellSize<Domain>(),
    _finalizeImpl,
    _markWeakImpl,
    _mallocSizeImpl,
    nullptr,
    nullptr,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Code,
        nullptr,
        Domain::_snapshotAddEdgesImpl,
        Domain::_snapshotAddNodesImpl,
        nullptr}};

void DomainBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Domain *>(cell);
  mb.setVTable(&Domain::vt);
  mb.addField("cjsModules", &self->cjsModules_);
  mb.addField("throwingRequire", &self->throwingRequire_);
}

#ifdef HERMESVM_SERIALIZE
Domain::Domain(Deserializer &d) : GCCell(&d.getRuntime()->getHeap(), &vt) {
  if (d.readInt<uint8_t>()) {
    cjsModules_.set(
        d.getRuntime(),
        Domain::deserializeArrayStorage(d),
        &d.getRuntime()->getHeap());
  }
  // Field llvh::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
  size_t size = d.readInt<size_t>();
  for (size_t i = 0; i < size; i++) {
    auto res = cjsModuleTable_
                   .try_emplace(
                       SymbolID::unsafeCreate(d.readInt<uint32_t>()),
                       d.readInt<uint32_t>())
                   .second;
    if (!res) {
      hermes_fatal("Shouldn't fail to insert during deserialization");
    }
  }

  // Field CopyableVector<RuntimeModule *> runtimeModules_{};
  size = d.readInt<size_t>();
  for (size_t i = 0; i < size; i++) {
    runtimeModules_.push_back(
        RuntimeModule::deserialize(d), &d.getRuntime()->getHeap());
  }

  d.readRelocation(&throwingRequire_, RelocationKind::GCPointer);
}

void DomainSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const Domain>(cell);
  // If we have an ArrayStorage serialize it here.
  bool hasArray = (bool)self->cjsModules_;
  s.writeInt<uint8_t>(hasArray);
  if (hasArray) {
    Domain::serializeArrayStorage(s, self->cjsModules_.get(s.getRuntime()));
  }
  // Field llvh::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
  size_t size = self->cjsModuleTable_.size();
  s.writeInt<size_t>(size);
  for (auto it = self->cjsModuleTable_.begin();
       it != self->cjsModuleTable_.end();
       it++) {
    s.writeInt<uint32_t>(it->first.unsafeGetRaw());
    s.writeInt<uint32_t>(it->second);
  }

  // Field CopyableVector<RuntimeModule *> runtimeModules_{};
  // Domain owns RuntimeModules. Call serialize funtion for them here.
  size = self->runtimeModules_.size();
  s.writeInt<size_t>(size);
  for (size_t i = 0; i < size; i++) {
    self->runtimeModules_[i]->serialize(s);
  }

  s.writeRelocation(self->throwingRequire_.get(s.getRuntime()));
  s.endObject(cell);
}

void DomainDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::DomainKind && "Expected Domain");
  auto *cell = d.getRuntime()->makeAFixed<Domain, HasFinalizer::Yes>(d);
  d.endObject(cell);
}

void Domain::serializeArrayStorage(Serializer &s, const ArrayStorage *cell) {
  assert(cell->size() % CJSModuleSize == 0 && "Invalid ArrayStorage size");
  s.writeInt<ArrayStorage::size_type>(cell->capacity());
  s.writeInt<ArrayStorage::size_type>(cell->size());
  for (ArrayStorage::size_type i = 0; i < cell->size(); i += CJSModuleSize) {
    s.writeHermesValue(cell->data()[i + CachedExportsOffset]);
    s.writeHermesValue(cell->data()[i + ModuleOffset]);
    s.writeHermesValue(cell->data()[i + FunctionIndexOffset]);
    HermesValue rm = cell->data()[i + runtimeModuleOffset];
    assert(
        (rm.isEmpty() || rm.isNativeValue()) &&
        "RuntimeModule must be empty or a native pointer.");
    s.writeHermesValue(
        rm.isEmpty() ? HermesValue::encodeNativePointer(nullptr) : rm,
        /* nativePointer */ true);
  }
  s.endObject(cell);
}

ArrayStorage *Domain::deserializeArrayStorage(Deserializer &d) {
  ArrayStorage::size_type capacity = d.readInt<ArrayStorage::size_type>();
  ArrayStorage::size_type size = d.readInt<ArrayStorage::size_type>();
  assert(size % CJSModuleSize == 0 && "Invalid ArrayStorage size");
  auto cjsModulesRes = ArrayStorage::create(d.getRuntime(), capacity, size);
  if (LLVM_UNLIKELY(cjsModulesRes == ExecutionStatus::EXCEPTION)) {
    hermes_fatal("fail to allocate memory for CJSModules");
  }
  auto *cell = vmcast<ArrayStorage>(*cjsModulesRes);
  for (ArrayStorage::size_type i = 0; i < cell->size(); i += CJSModuleSize) {
    d.readHermesValue(&cell->data()[i + CachedExportsOffset]);
    d.readHermesValue(&cell->data()[i + ModuleOffset]);
    d.readHermesValue(&cell->data()[i + FunctionIndexOffset]);
    d.readHermesValue(
        &cell->data()[i + runtimeModuleOffset], /* nativePointer */ true);
  }
  d.endObject(cell);
  return cell;
}
#endif

PseudoHandle<Domain> Domain::create(Runtime *runtime) {
  auto *cell = runtime->makeAFixed<Domain, HasFinalizer::Yes>(runtime);
  auto self = createPseudoHandle(cell);
  return self;
}

void Domain::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<Domain>(cell);
  for (RuntimeModule *rm : self->runtimeModules_) {
    gc->getIDTracker().untrackNative(rm);
  }
  self->~Domain();
}

Domain::~Domain() {
  for (RuntimeModule *rm : runtimeModules_) {
    delete rm;
  }
}

PseudoHandle<NativeFunction> Domain::getThrowingRequire(
    Runtime *runtime) const {
  return createPseudoHandle(throwingRequire_.get(runtime));
}

void Domain::_markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor) {
  auto *self = reinterpret_cast<Domain *>(cell);
  self->markWeakRefs(acceptor);
}

void Domain::markWeakRefs(WeakRefAcceptor &acceptor) {
  for (RuntimeModule *rm : runtimeModules_) {
    rm->markDomainRef(acceptor);
  }
}

size_t Domain::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<Domain>(cell);
  size_t rmSize = 0;
  for (auto *rm : self->runtimeModules_)
    rmSize += sizeof(RuntimeModule) + rm->additionalMemorySize();

  return self->cjsModuleTable_.getMemorySize() +
      self->runtimeModules_.capacity_in_bytes() + rmSize;
}

void Domain::_snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap) {
  auto *const self = vmcast<Domain>(cell);
  for (RuntimeModule *rm : self->runtimeModules_)
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Internal, "RuntimeModule", gc->getNativeID(rm));
}

void Domain::_snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap) {
  auto *const self = vmcast<Domain>(cell);
  for (RuntimeModule *rm : self->runtimeModules_) {
    // Create a native node for each RuntimeModule owned by this domain.
    rm->snapshotAddNodes(gc, snap);
    snap.beginNode();
    rm->snapshotAddEdges(gc, snap);
    snap.endNode(
        HeapSnapshot::NodeType::Native,
        "RuntimeModule",
        gc->getNativeID(rm),
        sizeof(RuntimeModule) + rm->additionalMemorySize(),
        0);
  }
}

ExecutionStatus Domain::importCJSModuleTable(
    Handle<Domain> self,
    Runtime *runtime,
    RuntimeModule *runtimeModule) {
  if (runtimeModule->getBytecode()->getCJSModuleTable().empty() &&
      runtimeModule->getBytecode()->getCJSModuleTableStatic().empty()) {
    // Nothing to do, avoid allocating and simply return.
    return ExecutionStatus::RETURNED;
  }

  static_assert(
      CJSModuleSize < 10, "CJSModuleSize must be small to avoid overflow");

  MutableHandle<ArrayStorage> cjsModules{runtime};

  if (!self->cjsModules_) {
    // Create the module table on first import.
    // If module IDs are contiguous and start from 0, we won't need to resize
    // for this RuntimeModule.

    const uint64_t firstSegmentModules =
        runtimeModule->getBytecode()->getCJSModuleTable().size() +
        runtimeModule->getBytecode()->getCJSModuleTableStatic().size();

    assert(
        firstSegmentModules <= std::numeric_limits<uint32_t>::max() &&
        "number of modules is 32 bits due to the bytecode format");

    // Use uint64_t to allow us to check for overflow.
    const uint64_t requiredSize = firstSegmentModules * CJSModuleSize;
    if (requiredSize > std::numeric_limits<uint32_t>::max()) {
      return runtime->raiseRangeError("Loaded module count exceeded limit");
    }

    auto cjsModulesRes = ArrayStorage::create(runtime, requiredSize);
    if (LLVM_UNLIKELY(cjsModulesRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    cjsModules = vmcast<ArrayStorage>(*cjsModulesRes);

    auto requireFn = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        (void *)TypeErrorKind::InvalidDynamicRequire,
        throwTypeError,
        Predefined::getSymbolID(Predefined::emptyString),
        0,
        Runtime::makeNullHandle<JSObject>());

    auto context = RequireContext::create(
        runtime,
        self,
        runtime->getPredefinedStringHandle(Predefined::emptyString));

    // Set the require.context property.
    PropertyFlags pf = PropertyFlags::defaultNewNamedPropertyFlags();
    pf.writable = 0;
    pf.configurable = 0;
    if (LLVM_UNLIKELY(
            JSObject::defineNewOwnProperty(
                requireFn,
                runtime,
                Predefined::getSymbolID(Predefined::context),
                pf,
                context) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    self->throwingRequire_.set(runtime, *requireFn, &runtime->getHeap());
  } else {
    cjsModules = self->cjsModules_.get(runtime);
  }

  assert(cjsModules && "cjsModules not set");

  // Find the maximum module ID so we can resize cjsModules at most once per
  // RuntimeModule.
  uint64_t maxModuleID = cjsModules->size() / CJSModuleSize;

  // The non-static module table does not store module IDs. They are assigned
  // during registration by counting insertions to cjsModuleTable_. Count the
  // insertions up front.
  for (const auto &pair : runtimeModule->getBytecode()->getCJSModuleTable()) {
    SymbolID symbolId =
        runtimeModule->getSymbolIDFromStringIDMayAllocate(pair.first);
    if (self->cjsModuleTable_.find(symbolId) == self->cjsModuleTable_.end()) {
      ++maxModuleID;
    }
  }

  // The static module table stores module IDs in an arbitrary order. Scan for
  // the maximum ID.
  for (const auto &pair :
       runtimeModule->getBytecode()->getCJSModuleTableStatic()) {
    const auto &moduleID = pair.first;
    if (moduleID > maxModuleID) {
      maxModuleID = moduleID;
    }
  }

  assert(
      maxModuleID <= std::numeric_limits<uint32_t>::max() &&
      "number of modules is 32 bits due to the bytecode format");

  // Use uint64_t to allow us to check for overflow.
  const uint64_t requiredSize = (maxModuleID + 1) * CJSModuleSize;
  if (requiredSize > std::numeric_limits<uint32_t>::max()) {
    return runtime->raiseRangeError("Loaded module count exceeded limit");
  }

  // Resize the array to allow for the new modules, if necessary.
  if (requiredSize > cjsModules->size()) {
    if (LLVM_UNLIKELY(
            ArrayStorage::resize(cjsModules, runtime, requiredSize) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  /// \return Whether the module with ID \param moduleID has been registered in
  /// cjsModules. \pre Space has been allocated for this module's record in
  /// cjsModules.
  const auto isModuleRegistered = [&cjsModules,
                                   maxModuleID](uint32_t moduleID) -> bool {
    assert(
        moduleID <= maxModuleID &&
        "CJS module ID exceeds maximum known module ID");
    (void)maxModuleID;
    uint32_t index = moduleID * CJSModuleSize;
    uint32_t requiredSize = index + CJSModuleSize;
    assert(
        cjsModules->size() >= requiredSize &&
        "CJS module ID exceeds allocated storage");
    (void)requiredSize;
    return !cjsModules->at(index + FunctionIndexOffset).isEmpty();
  };

  /// Register CJS module \param moduleID in the runtime module table.
  /// \return The index into cjsModules where this module's record begins.
  /// \pre Space has been allocated for this module's record in cjsModules.
  /// \pre There is no module already registered under moduleID.
  auto &cjsEntryModuleID = self->cjsEntryModuleID_;
  const auto registerModule =
      [runtime,
       &cjsModules,
       runtimeModule,
       &isModuleRegistered,
       &cjsEntryModuleID](uint32_t moduleID, uint32_t functionID) -> uint32_t {
    assert(!isModuleRegistered(moduleID) && "CJS module ID collision occurred");
    (void)isModuleRegistered;
    if (LLVM_UNLIKELY(!cjsEntryModuleID.hasValue())) {
      cjsEntryModuleID = moduleID;
    }
    uint32_t index = moduleID * CJSModuleSize;
    cjsModules->set(
        index + CachedExportsOffset,
        HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
    cjsModules->set(
        index + ModuleOffset,
        HermesValue::encodeObjectValue(nullptr),
        &runtime->getHeap());
    cjsModules->set(
        index + FunctionIndexOffset,
        HermesValue::encodeNativeUInt32(functionID),
        &runtime->getHeap());
    cjsModules->set(
        index + runtimeModuleOffset,
        HermesValue::encodeNativePointer(runtimeModule),
        &runtime->getHeap());
    assert(isModuleRegistered(moduleID) && "CJS module was not registered");
    return index;
  };

  // Import full table that allows dynamic requires.
  for (const auto &pair : runtimeModule->getBytecode()->getCJSModuleTable()) {
    SymbolID symbolId =
        runtimeModule->getSymbolIDFromStringIDMayAllocate(pair.first);
    auto emplaceRes = self->cjsModuleTable_.try_emplace(symbolId, 0xffffffff);
    if (emplaceRes.second) {
      // This module has not been registered before.
      // Assign it an arbitrary unused module ID, because nothing will be
      // referencing that ID from outside Domain.
      // Counting insertions to cjsModuleTable_ is a valid source of unique IDs
      // since a given Domain uses either dynamic requires or statically
      // resolved requires.
      uint32_t moduleID = self->cjsModuleTable_.size() - 1;
      const auto functionID = pair.second;
      auto index = registerModule(moduleID, functionID);
      // Update the mapping from symbolId to an index into cjsModules.
      emplaceRes.first->second = index;
    }
  }

  // Import table to be used for requireFast.
  for (const auto &pair :
       runtimeModule->getBytecode()->getCJSModuleTableStatic()) {
    const auto &moduleID = pair.first;
    const auto &functionID = pair.second;
    if (!isModuleRegistered(moduleID)) {
      registerModule(moduleID, functionID);
    }
  }

  self->cjsModules_.set(runtime, cjsModules.get(), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

const ObjectVTable RequireContext::vt{
    VTable(CellKind::RequireContextKind, cellSize<RequireContext>()),
    RequireContext::_getOwnIndexedRangeImpl,
    RequireContext::_haveOwnIndexedImpl,
    RequireContext::_getOwnIndexedPropertyFlagsImpl,
    RequireContext::_getOwnIndexedImpl,
    RequireContext::_setOwnIndexedImpl,
    RequireContext::_deleteOwnIndexedImpl,
    RequireContext::_checkAllOwnIndexedImpl,
};

void RequireContextBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<RequireContext>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const RequireContext *>(cell);
  mb.setVTable(&RequireContext::vt.base);
  mb.addField(&self->domain_);
  mb.addField(&self->dirname_);
}

#ifdef HERMESVM_SERIALIZE
RequireContext::RequireContext(Deserializer &d) : JSObject(d, &vt.base) {
  d.readRelocation(&domain_, RelocationKind::GCPointer);
  d.readRelocation(&dirname_, RelocationKind::GCPointer);
}

void RequireContextSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<RequireContext>());
  const auto *self = static_cast<const RequireContext *>(cell);
  s.writeRelocation(self->domain_.get(s.getRuntime()));
  s.writeRelocation(self->dirname_.get(s.getRuntime()));
  s.endObject(cell);
}

void RequireContextDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::RequireContextKind && "Expected RequireContext");
  auto *cell = d.getRuntime()->makeAFixed<RequireContext>(d);
  d.endObject(cell);
}
#endif

Handle<RequireContext> RequireContext::create(
    Runtime *runtime,
    Handle<Domain> domain,
    Handle<StringPrimitive> dirname) {
  auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
  auto *cell = runtime->makeAFixed<RequireContext>(
      runtime, objProto, runtime->getHiddenClassForPrototype(*objProto, 0));
  auto self = JSObjectInit::initToHandle(runtime, cell);
  self->domain_.set(runtime, *domain, &runtime->getHeap());
  self->dirname_.set(runtime, *dirname, &runtime->getHeap());
  return self;
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
