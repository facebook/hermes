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
  mb.addField("@cjsModules", &self->cjsModules_);
  mb.addField("@throwingRequire", &self->throwingRequire_);
}

Domain::Domain(Deserializer &d) : GCCell(&d.getRuntime()->getHeap(), &vt) {
  d.readRelocation(&cjsModules_, RelocationKind::GCPointer);
  // Field llvm::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
  // Deserializing the CommonJS module table is unnecessary to deserialize
  // global object initialization.
  // TODO: Once we deserialize arbitrary JS heap state, begin deserializing this
  // field.

  // Field CopyableVector<RuntimeModule *> runtimeModules_{};
  // Deserializing the runtime Modules list is unnecessary to deserialize
  // global object initialization.
  // TODO: Once we deserialize arbitrary JS heap state, begin deserializing this
  // field.

  d.readRelocation(&throwingRequire_, RelocationKind::GCPointer);
}

void DomainSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const Domain>(cell);
  s.writeRelocation(self->cjsModules_.get(s.getRuntime()));
  // Field llvm::DenseMap<SymbolID, uint32_t> cjsModuleTable_{};
  // Serializing the CommonJS module table is unnecessary to serialize global
  // object initialization.
  // TODO: Once we serialize arbitrary JS heap state, begin serializing this
  // field.
  assert(
      self->cjsModuleTable_.size() == 0 &&
      "Shouldn't have cjsModules at this point.");
  // Field CopyableVector<RuntimeModule *> runtimeModules_{};
  // Serializing runtime Modules list is unnecessary to serialize global
  // object initialization.
  // TODO: Once we serialize arbitrary JS heap state, begin serializing this
  // field.

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

    auto requireFn = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        (void *)TypeErrorKind::InvalidDynamicRequire,
        throwTypeError,
        Predefined::getSymbolID(Predefined::emptyString),
        0,
        runtime->makeNullHandle<JSObject>());

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

void RequireContextSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs()
      << "Serialize function not implemented for RequireContext\n");
}

void RequireContextDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs()
      << "Deserialize function not implemented for RequireContext\n");
}

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
