/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Domain.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

VTable Domain::vt{CellKind::DomainKind,
                  sizeof(Domain),
                  _finalizeImpl,
                  _markWeakImpl,
                  _mallocSizeImpl};

void DomainBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Domain *>(cell);
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
  // Field llvm::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
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
  // Field llvm::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
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
  void *mem = d.getRuntime()->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      sizeof(Domain));
  auto *cell = new (mem) Domain(d);
  auto &samplingProfiler = SamplingProfiler::getInstance();
  samplingProfiler->increaseDomainCount();
  d.endObject(cell);
}

void Domain::serializeArrayStorage(Serializer &s, const ArrayStorage *cell) {
  assert(
      cell->size() % runtimeModuleOffset == 0 && "Invalid ArrayStorage size");
  s.writeInt<ArrayStorage::size_type>(cell->capacity());
  s.writeInt<ArrayStorage::size_type>(cell->size());
  for (ArrayStorage::size_type i = 0; i < cell->size(); i += CJSModuleSize) {
    s.writeHermesValue(cell->data()[i + CachedExportsOffset]);
    s.writeHermesValue(cell->data()[i + ModuleOffset]);
    s.writeHermesValue(cell->data()[i + FunctionIndexOffset]);
    s.writeHermesValue(
        cell->data()[i + runtimeModuleOffset], /* nativePointer */ true);
  }
  s.endObject(cell);
}

ArrayStorage *Domain::deserializeArrayStorage(Deserializer &d) {
  ArrayStorage::size_type capacity = d.readInt<ArrayStorage::size_type>();
  ArrayStorage::size_type size = d.readInt<ArrayStorage::size_type>();
  assert(size % runtimeModuleOffset == 0 && "Invalid ArrayStorage size");
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
  void *mem =
      runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(sizeof(Domain));
  auto self = createPseudoHandle(new (mem) Domain(runtime));
  auto &samplingProfiler = SamplingProfiler::getInstance();
  samplingProfiler->increaseDomainCount();
  return self;
}

void Domain::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<Domain>(cell);
  self->~Domain();
  auto &samplingProfiler = SamplingProfiler::getInstance();
  samplingProfiler->decreaseDomainCount();
}

Domain::~Domain() {
  for (RuntimeModule *rm : runtimeModules_) {
    delete rm;
  }
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
    // Resize the array to allow for the new modules, if necessary.
    if (requiredSize > self->cjsModules_.get(runtime)->size()) {
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

    // symbolId must not be inlined because getSymbolIDFromStringID allocates,
    // which can make self->cjsModuleTable_ stale.
    SymbolID symbolId =
        runtimeModule->getSymbolIDFromStringIDMayAllocate(pair.first);
    auto result = self->cjsModuleTable_.try_emplace(symbolId, startIndex);
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

  self->cjsModules_.set(runtime, cjsModules.get(), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

ObjectVTable RequireContext::vt{
    VTable(CellKind::RequireContextKind, sizeof(RequireContext)),
    RequireContext::_getOwnIndexedRangeImpl,
    RequireContext::_haveOwnIndexedImpl,
    RequireContext::_getOwnIndexedPropertyFlagsImpl,
    RequireContext::_getOwnIndexedImpl,
    RequireContext::_setOwnIndexedImpl,
    RequireContext::_deleteOwnIndexedImpl,
    RequireContext::_checkAllOwnIndexedImpl,
};

void RequireContextBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
RequireContext::RequireContext(Deserializer &d) : JSObject(d, &vt.base) {}

void RequireContextSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void RequireContextDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::RequireContextKind && "Expected RequireContext");
  void *mem = d.getRuntime()->alloc</*fixedSize*/ true, HasFinalizer::No>(
      sizeof(RequireContext));
  auto *cell = new (mem) RequireContext(d);
  d.endObject(cell);
}
#endif

Handle<RequireContext> RequireContext::create(
    Runtime *runtime,
    Handle<Domain> domain,
    Handle<StringPrimitive> dirname) {
  void *mem = runtime->alloc</*fixedSize*/ true, HasFinalizer::No>(
      sizeof(RequireContext));
  auto self = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) RequireContext(
              runtime,
              vmcast<JSObject>(runtime->objectPrototype),
              runtime->getHiddenClassForPrototypeRaw(
                  vmcast<JSObject>(runtime->objectPrototype)))));

  JSObject::addInternalProperties(self, runtime, 2, domain);
  JSObject::setInternalProperty(*self, runtime, 1, dirname.getHermesValue());

  return self;
}

} // namespace vm
} // namespace hermes
