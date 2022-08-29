/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "class"
#include "hermes/VM/HiddenClass.h"

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StringView.h"

#include "llvh/Support/Debug.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
using llvh::dbgs;

namespace hermes {
namespace vm {

namespace detail {

#ifdef HERMES_MEMORY_INSTRUMENTATION
void TransitionMap::snapshotAddNodes(GC &gc, HeapSnapshot &snap) {
  if (!isLarge()) {
    return;
  }
  // Make one node that is the sum of the sizes of the WeakValueMap and the
  // llvh::DenseMap to which it points.
  // This is based on the assumption that the WeakValueMap uniquely owns that
  // DenseMap.
  snap.beginNode();
  snap.endNode(
      HeapSnapshot::NodeType::Native,
      "WeakValueMap",
      gc.getNativeID(large()),
      getMemorySize(),
      0);
}

void TransitionMap::snapshotAddEdges(GC &gc, HeapSnapshot &snap) {
  if (!isLarge()) {
    return;
  }
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Internal,
      "transitionMap",
      gc.getNativeID(large()));
}

void TransitionMap::snapshotUntrackMemory(GC &gc) {
  // Untrack the memory ID in case one was created.
  if (isLarge()) {
    gc.getIDTracker().untrackNative(large());
  }
}
#endif

size_t TransitionMap::getMemorySize() const {
  // Inline slot is not counted here (it counts as part of the HiddenClass).
  return isLarge() ? sizeof(*large()) + large()->getMemorySize() : 0;
}

void TransitionMap::uncleanMakeLarge(Runtime &runtime) {
  assert(!isClean() && "must not still be clean");
  assert(!isLarge() && "must not yet be large");
  auto large = new WeakValueMap<Transition, HiddenClass>();
  // Move any valid entry into the allocated map.
  if (auto value = smallValue().get(runtime))
    large->insertNewLocked(runtime, smallKey_, runtime.makeHandle(value));
  u.large_ = large;
  smallKey_.symbolID = SymbolID::deleted();
  assert(isLarge());
}

} // namespace detail

const VTable HiddenClass::vt{
    CellKind::HiddenClassKind,
    cellSize<HiddenClass>(),
    _finalizeImpl,
    _markWeakImpl,
    _mallocSizeImpl,
    nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
    ,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Object,
        HiddenClass::_snapshotNameImpl,
        HiddenClass::_snapshotAddEdgesImpl,
        HiddenClass::_snapshotAddNodesImpl,
        nullptr}
#endif
};

void HiddenClassBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const HiddenClass *>(cell);
  mb.setVTable(&HiddenClass::vt);
  mb.addField("symbol", &self->symbolID_);
  mb.addField("parent", &self->parent_);
  mb.addField("propertyMap", &self->propertyMap_);
  mb.addField("forInCache", &self->forInCache_);
}

void HiddenClass::_markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor) {
  auto *self = reinterpret_cast<HiddenClass *>(cell);
  self->transitionMap_.markWeakRefs(acceptor);
}

void HiddenClass::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<HiddenClass>(cell);
#ifdef HERMES_MEMORY_INSTRUMENTATION
  self->transitionMap_.snapshotUntrackMemory(gc);
#endif
  self->~HiddenClass();
}

size_t HiddenClass::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<HiddenClass>(cell);
  return self->transitionMap_.getMemorySize();
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string HiddenClass::_snapshotNameImpl(GCCell *cell, GC &gc) {
  auto *const self = vmcast<HiddenClass>(cell);
  std::string name{cell->getVT()->snapshotMetaData.defaultNameForNode(self)};
  if (self->isDictionary()) {
    return name + "(Dictionary)";
  }
  return name;
}

void HiddenClass::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<HiddenClass>(cell);
  self->transitionMap_.snapshotAddEdges(gc, snap);
}

void HiddenClass::_snapshotAddNodesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<HiddenClass>(cell);
  self->transitionMap_.snapshotAddNodes(gc, snap);
}
#endif

CallResult<HermesValue> HiddenClass::createRoot(Runtime &runtime) {
  return create(
      runtime,
      ClassFlags{},
      Runtime::makeNullHandle<HiddenClass>(),
      SymbolID{},
      PropertyFlags{},
      0);
}

CallResult<HermesValue> HiddenClass::create(
    Runtime &runtime,
    ClassFlags flags,
    Handle<HiddenClass> parent,
    SymbolID symbolID,
    PropertyFlags propertyFlags,
    unsigned numProperties) {
  assert(
      (flags.dictionaryMode || numProperties == 0 || *parent) &&
      "non-empty non-dictionary orphan");
  auto *obj =
      runtime.makeAFixed<HiddenClass, HasFinalizer::Yes, LongLived::Yes>(
          runtime, flags, parent, symbolID, propertyFlags, numProperties);
  return HermesValue::encodeObjectValue(obj);
}

Handle<HiddenClass> HiddenClass::copyToNewDictionary(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    bool noCache) {
  assert(
      !selfHandle->isDictionaryNoCache() && "class already in no-cache mode");

  auto newFlags = selfHandle->flags_;
  newFlags.dictionaryMode = true;
  // If the requested, transition to no-cache mode.
  if (noCache) {
    newFlags.dictionaryNoCacheMode = true;
  }

  /// Allocate a new class without a parent.
  auto newClassHandle = runtime.makeHandle<HiddenClass>(
      runtime.ignoreAllocationFailure(HiddenClass::create(
          runtime,
          newFlags,
          Runtime::makeNullHandle<HiddenClass>(),
          SymbolID{},
          PropertyFlags{},
          selfHandle->numProperties_)));

  // Optionally allocate the property map and move it to the new class.
  if (LLVM_UNLIKELY(!selfHandle->propertyMap_))
    initializeMissingPropertyMap(selfHandle, runtime);

  newClassHandle->propertyMap_.set(
      runtime, selfHandle->propertyMap_, runtime.getHeap());
  selfHandle->propertyMap_.setNull(runtime.getHeap());

  LLVM_DEBUG(
      dbgs() << "Converted Class:" << selfHandle->getDebugAllocationId()
             << " to dictionary Class:"
             << newClassHandle->getDebugAllocationId() << "\n");

  return newClassHandle;
}

void HiddenClass::forEachPropertyNoAlloc(
    HiddenClass *self,
    PointerBase &base,
    std::function<void(SymbolID, NamedPropertyDescriptor)> callback) {
  std::vector<std::pair<SymbolID, NamedPropertyDescriptor>> properties;
  HiddenClass *curr = self;
  while (curr && !curr->propertyMap_) {
    // Skip invalid symbols stored in the hidden class chain, as well as
    // flag-only transitions.
    if (curr->symbolID_.isValid() && !curr->propertyFlags_.flagsTransition) {
      properties.emplace_back(
          curr->symbolID_,
          NamedPropertyDescriptor{
              curr->propertyFlags_, curr->numProperties_ - 1});
    }
    curr = curr->parent_.get(base);
  }

  // Either we reached the root hidden class and never found a property
  // map, or we found a property map somewhere in the HiddenClass chain.
  if (curr) {
    assert(
        curr->propertyMap_ &&
        "If it's not the root class, it must have a property map");
    // The DPM exists, and it can be iterated over to find some properties.
    DictPropertyMap::forEachPropertyNoAlloc(
        curr->propertyMap_.getNonNull(base), callback);
  }
  // Add any iterated properties at the end.
  // Since we moved backwards through HiddenClasses, the properties are in
  // reverse order. Iterate backwards through properties to get the original
  // order.
  for (auto it = properties.rbegin(); it != properties.rend(); ++it) {
    callback(it->first, it->second);
  }
}

OptValue<HiddenClass::PropertyPos> HiddenClass::findProperty(
    PseudoHandle<HiddenClass> self,
    Runtime &runtime,
    SymbolID name,
    PropertyFlags expectedFlags,
    NamedPropertyDescriptor &desc) {
  // Lazily create the property map.
  if (LLVM_UNLIKELY(!self->propertyMap_)) {
    // If expectedFlags is valid, we can check if there is an outgoing
    // transition with name and the flags. The presence of such a transition
    // indicates that this is a new property and we don't have to build the map
    // in order to look for it (since we wouldn't find it anyway).
    if (expectedFlags.isValid()) {
      Transition t{name, expectedFlags};
      if (self->transitionMap_.containsKey(t, runtime.getHeap())) {
        LLVM_DEBUG(
            dbgs()
            << "Property " << runtime.formatSymbolID(name)
            << " NOT FOUND in Class:" << self->getDebugAllocationId()
            << " due to existing transition to Class:"
            << self->transitionMap_.lookup(runtime, t)->getDebugAllocationId()
            << "\n");
        return llvh::None;
      }
    }

    auto selfHandle = runtime.makeHandle(std::move(self));
    initializeMissingPropertyMap(selfHandle, runtime);
    self = selfHandle;
  }

  auto *propMap = self->propertyMap_.getNonNull(runtime);
  {
    // propMap is a raw pointer.  We assume that find does no allocation.
    NoAllocScope noAlloc(runtime);
    auto found = DictPropertyMap::find(propMap, name);
    if (!found)
      return llvh::None;
    // Technically, the last use of propMap occurs before the call here, so
    // it would be legal for the call to allocate.  If that were ever the case,
    // we would move "found" out of scope, and terminate the NoAllocScope here.
    desc = DictPropertyMap::getDescriptorPair(propMap, *found)->second;
    return *found;
  }
}

llvh::Optional<NamedPropertyDescriptor> HiddenClass::findPropertyNoAlloc(
    HiddenClass *self,
    PointerBase &base,
    SymbolID name) {
  for (HiddenClass *curr = self; curr; curr = curr->parent_.get(base)) {
    if (curr->propertyMap_) {
      // If a property map exists, just search this hidden class
      auto found =
          DictPropertyMap::find(curr->propertyMap_.getNonNull(base), name);
      if (found) {
        return DictPropertyMap::getDescriptorPair(
                   curr->propertyMap_.getNonNull(base), *found)
            ->second;
      }
    }
    // Else, no property map exists. Check the current hidden class before
    // moving up.
    if (curr->symbolID_ == name) {
      return NamedPropertyDescriptor{
          curr->propertyFlags_, curr->numProperties_ - 1};
    }
  }
  // Reached the root hidden class without finding a property map or the
  // matching symbol, this property doesn't exist.
  return llvh::None;
}

bool HiddenClass::debugIsPropertyDefined(
    HiddenClass *self,
    PointerBase &base,
    SymbolID name) {
  do {
    // If we happen to have a property map, use it.
    if (self->propertyMap_)
      return DictPropertyMap::find(self->propertyMap_.getNonNull(base), name)
          .hasValue();
    // Is the property defined in this class?
    if (self->symbolID_ == name)
      return true;
    self = self->parent_.get(base);
  } while (self);
  return false;
}

Handle<HiddenClass> HiddenClass::deleteProperty(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    PropertyPos pos) {
  // We convert to dictionary if we're not yet a dictionary
  // (transition to a cacheable dictionary), or if we are, but not yet
  // in no-cache mode (transition to no-cache mode).
  auto newHandle = LLVM_UNLIKELY(!selfHandle->isDictionaryNoCache())
      ? copyToNewDictionary(selfHandle, runtime, selfHandle->isDictionary())
      : selfHandle;

  --newHandle->numProperties_;

  DictPropertyMap::erase(newHandle->propertyMap_.get(runtime), runtime, pos);

  LLVM_DEBUG(
      dbgs() << "Deleting from Class:" << selfHandle->getDebugAllocationId()
             << " produces Class:" << newHandle->getDebugAllocationId()
             << "\n");

  return newHandle;
}

CallResult<std::pair<Handle<HiddenClass>, SlotIndex>> HiddenClass::addProperty(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropertyFlags propertyFlags) {
  assert(propertyFlags.isValid() && "propertyFlags must be valid");

  if (LLVM_UNLIKELY(selfHandle->isDictionary())) {
    if (toArrayIndex(
            runtime.getIdentifierTable().getStringView(runtime, name))) {
      selfHandle->flags_.hasIndexLikeProperties = true;
    }

    // Allocate a new slot.
    // TODO: this changes the property map, so if we want to support OOM
    // handling in the future, and the following operation fails, we would have
    // to somehow be able to undo it, or use an approach where we peek the slot
    // but not consume until we are sure (which is less efficient, but more
    // robust). T31555339.
    SlotIndex newSlot = DictPropertyMap::allocatePropertySlot(
        selfHandle->propertyMap_.get(runtime), runtime);

    if (LLVM_UNLIKELY(
            addToPropertyMap(
                selfHandle,
                runtime,
                name,
                NamedPropertyDescriptor(propertyFlags, newSlot)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    ++selfHandle->numProperties_;
    return std::make_pair(selfHandle, newSlot);
  }

  // Do we already have a transition for that property+flags pair?
  auto existingChild =
      selfHandle->transitionMap_.lookup(runtime, {name, propertyFlags});
  if (LLVM_LIKELY(existingChild)) {
    auto childHandle = runtime.makeHandle(existingChild);
    // If the child doesn't have a property map, but we do, update our map and
    // move it to the child.
    if (!childHandle->propertyMap_ && selfHandle->propertyMap_) {
      LLVM_DEBUG(
          dbgs() << "Adding property " << runtime.formatSymbolID(name)
                 << " to Class:" << selfHandle->getDebugAllocationId()
                 << " transitions Map to existing Class:"
                 << childHandle->getDebugAllocationId() << "\n");

      if (LLVM_UNLIKELY(
              addToPropertyMap(
                  selfHandle,
                  runtime,
                  name,
                  NamedPropertyDescriptor(
                      propertyFlags, selfHandle->numProperties_)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      childHandle->propertyMap_.set(
          runtime, selfHandle->propertyMap_, runtime.getHeap());
    } else {
      LLVM_DEBUG(
          dbgs() << "Adding property " << runtime.formatSymbolID(name)
                 << " to Class:" << selfHandle->getDebugAllocationId()
                 << " transitions to existing Class:"
                 << childHandle->getDebugAllocationId() << "\n");
    }

    // In any case, clear our own map.
    selfHandle->propertyMap_.setNull(runtime.getHeap());

    return std::make_pair(childHandle, selfHandle->numProperties_);
  }

  // Do we need to convert to dictionary?
  if (LLVM_UNLIKELY(selfHandle->numProperties_ == kDictionaryThreshold)) {
    // Do it.
    auto childHandle = copyToNewDictionary(selfHandle, runtime);

    if (toArrayIndex(
            runtime.getIdentifierTable().getStringView(runtime, name))) {
      childHandle->flags_.hasIndexLikeProperties = true;
    }

    // Add the property to the child.
    if (LLVM_UNLIKELY(
            addToPropertyMap(
                childHandle,
                runtime,
                name,
                NamedPropertyDescriptor(
                    propertyFlags, childHandle->numProperties_)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return std::make_pair(childHandle, childHandle->numProperties_++);
  }

  // Allocate the child.
  auto childHandle = runtime.makeHandle<HiddenClass>(
      runtime.ignoreAllocationFailure(HiddenClass::create(
          runtime,
          selfHandle->flags_,
          selfHandle,
          name,
          propertyFlags,
          selfHandle->numProperties_ + 1)));

  // Add it to the transition table.
  auto inserted = selfHandle->transitionMap_.insertNew(
      runtime, Transition(name, propertyFlags), childHandle);
  (void)inserted;
  assert(
      inserted &&
      "transition already exists when adding a new property to hidden class");

  if (toArrayIndex(runtime.getIdentifierTable().getStringView(runtime, name))) {
    childHandle->flags_.hasIndexLikeProperties = true;
  }

  if (selfHandle->propertyMap_) {
    assert(
        !DictPropertyMap::find(selfHandle->propertyMap_.get(runtime), name) &&
        "Adding an existing property to hidden class");

    LLVM_DEBUG(
        dbgs() << "Adding property " << runtime.formatSymbolID(name)
               << " to Class:" << selfHandle->getDebugAllocationId()
               << " transitions Map to new Class:"
               << childHandle->getDebugAllocationId() << "\n");

    // Move the map to the child class.
    childHandle->propertyMap_.set(
        runtime, selfHandle->propertyMap_, runtime.getHeap());
    selfHandle->propertyMap_.setNull(runtime.getHeap());

    if (LLVM_UNLIKELY(
            addToPropertyMap(
                childHandle,
                runtime,
                name,
                NamedPropertyDescriptor(
                    propertyFlags, selfHandle->numProperties_)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    LLVM_DEBUG(
        dbgs() << "Adding property " << runtime.formatSymbolID(name)
               << " to Class:" << selfHandle->getDebugAllocationId()
               << " transitions to new Class:"
               << childHandle->getDebugAllocationId() << "\n");
  }

  return std::make_pair(childHandle, selfHandle->numProperties_);
}

Handle<HiddenClass> HiddenClass::updateProperty(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    PropertyPos pos,
    PropertyFlags newFlags) {
  assert(newFlags.isValid() && "newFlags must be valid");

  // In dictionary mode we simply update our map (which must exist).
  if (LLVM_UNLIKELY(selfHandle->isDictionary())) {
    assert(
        selfHandle->propertyMap_ &&
        "propertyMap must exist in dictionary mode");
    DictPropertyMap::getDescriptorPair(
        selfHandle->propertyMap_.getNonNull(runtime), pos)
        ->second.flags = newFlags;
    // If it's still cacheable, make it non-cacheable.
    if (!selfHandle->isDictionaryNoCache()) {
      selfHandle = copyToNewDictionary(selfHandle, runtime, /*noCache*/ true);
    }
    return selfHandle;
  }

  assert(
      selfHandle->propertyMap_ && "propertyMap must exist in updateProperty()");

  auto *descPair = DictPropertyMap::getDescriptorPair(
      selfHandle->propertyMap_.get(runtime), pos);
  // If the property flags didn't change, do nothing.
  if (descPair->second.flags == newFlags)
    return selfHandle;

  auto name = descPair->first;

  // The transition flags must indicate that it is a "flags transition".
  PropertyFlags transitionFlags = newFlags;
  transitionFlags.flagsTransition = 1;

  // Do we already have a transition for that property+flags pair?
  auto existingChild =
      selfHandle->transitionMap_.lookup(runtime, {name, transitionFlags});
  if (LLVM_LIKELY(existingChild)) {
    // If the child doesn't have a property map, but we do, update our map and
    // move it to the child.
    if (!existingChild->propertyMap_) {
      LLVM_DEBUG(
          dbgs() << "Updating property " << runtime.formatSymbolID(name)
                 << " in Class:" << selfHandle->getDebugAllocationId()
                 << " transitions Map to existing Class:"
                 << existingChild->getDebugAllocationId() << "\n");

      descPair->second.flags = newFlags;
      existingChild->propertyMap_.set(
          runtime, selfHandle->propertyMap_, runtime.getHeap());
    } else {
      LLVM_DEBUG(
          dbgs() << "Updating property " << runtime.formatSymbolID(name)
                 << " in Class:" << selfHandle->getDebugAllocationId()
                 << " transitions to existing Class:"
                 << existingChild->getDebugAllocationId() << "\n");
    }

    // In any case, clear our own map.
    selfHandle->propertyMap_.setNull(runtime.getHeap());

    return runtime.makeHandle(existingChild);
  }

  // We are updating the existing property and adding a transition to a new
  // hidden class.
  descPair->second.flags = newFlags;

  // Allocate the child.
  auto childHandle = runtime.makeHandle<HiddenClass>(
      runtime.ignoreAllocationFailure(HiddenClass::create(
          runtime,
          selfHandle->flags_,
          selfHandle,
          name,
          transitionFlags,
          selfHandle->numProperties_)));

  // Add it to the transition table.
  auto inserted = selfHandle->transitionMap_.insertNew(
      runtime, Transition(name, transitionFlags), childHandle);
  (void)inserted;
  assert(
      inserted &&
      "transition already exists when updating a property in hidden class");

  LLVM_DEBUG(
      dbgs() << "Updating property " << runtime.formatSymbolID(name)
             << " in Class:" << selfHandle->getDebugAllocationId()
             << " transitions Map to new Class:"
             << childHandle->getDebugAllocationId() << "\n");

  // Move the updated map to the child class.
  childHandle->propertyMap_.set(
      runtime, selfHandle->propertyMap_, runtime.getHeap());
  selfHandle->propertyMap_.setNull(runtime.getHeap());

  return childHandle;
}

Handle<HiddenClass> HiddenClass::makeAllNonConfigurable(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  if (selfHandle->flags_.allNonConfigurable)
    return selfHandle;

  if (!selfHandle->propertyMap_)
    initializeMissingPropertyMap(selfHandle, runtime);

  LLVM_DEBUG(
      dbgs() << "Class:" << selfHandle->getDebugAllocationId()
             << " making all non-configurable\n");

  // Keep a handle to our initial map. The order of properties in it will
  // remain the same as long as we are only doing property updates.
  auto mapHandle = runtime.makeHandle(selfHandle->propertyMap_);

  MutableHandle<HiddenClass> curHandle{runtime, *selfHandle};

  // TODO: this can be made much more efficient at the expense of moving some
  // logic from updateOwnProperty() here.
  DictPropertyMap::forEachProperty(
      mapHandle,
      runtime,
      [&runtime, &curHandle](SymbolID id, NamedPropertyDescriptor desc) {
        if (!desc.flags.configurable)
          return;
        PropertyFlags newFlags = desc.flags;
        newFlags.configurable = 0;

        assert(
            curHandle->propertyMap_ &&
            "propertyMap must exist after updateOwnProperty()");

        auto found =
            DictPropertyMap::find(curHandle->propertyMap_.get(runtime), id);
        assert(found && "property not found during enumeration");
        curHandle = *updateProperty(curHandle, runtime, *found, newFlags);
      });

  curHandle->flags_.allNonConfigurable = true;

  return std::move(curHandle);
}

Handle<HiddenClass> HiddenClass::makeAllReadOnly(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  if (selfHandle->flags_.allReadOnly)
    return selfHandle;

  if (!selfHandle->propertyMap_)
    initializeMissingPropertyMap(selfHandle, runtime);

  LLVM_DEBUG(
      dbgs() << "Class:" << selfHandle->getDebugAllocationId()
             << " making all read-only\n");

  // Keep a handle to our initial map. The order of properties in it will
  // remain the same as long as we are only doing property updates.
  auto mapHandle = runtime.makeHandle(selfHandle->propertyMap_);

  MutableHandle<HiddenClass> curHandle{runtime, *selfHandle};

  // TODO: this can be made much more efficient at the expense of moving some
  // logic from updateOwnProperty() here.
  DictPropertyMap::forEachProperty(
      mapHandle,
      runtime,
      [&runtime, &curHandle](SymbolID id, NamedPropertyDescriptor desc) {
        PropertyFlags newFlags = desc.flags;
        if (!newFlags.accessor) {
          newFlags.writable = 0;
          newFlags.configurable = 0;
        } else {
          newFlags.configurable = 0;
        }
        if (desc.flags == newFlags)
          return;

        assert(
            curHandle->propertyMap_ &&
            "propertyMap must exist after updateOwnProperty()");

        auto found =
            DictPropertyMap::find(curHandle->propertyMap_.get(runtime), id);
        assert(found && "property not found during enumeration");
        curHandle = *updateProperty(curHandle, runtime, *found, newFlags);
      });

  curHandle->flags_.allNonConfigurable = true;
  curHandle->flags_.allReadOnly = true;

  return std::move(curHandle);
}

Handle<HiddenClass> HiddenClass::updatePropertyFlagsWithoutTransitions(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    PropertyFlags flagsToClear,
    PropertyFlags flagsToSet,
    OptValue<llvh::ArrayRef<SymbolID>> props) {
  // Result must be in dictionary mode, since it's a non-empty orphan.
  MutableHandle<HiddenClass> classHandle{runtime};
  if (selfHandle->isDictionary()) {
    classHandle = *selfHandle;
  } else {
    classHandle = *copyToNewDictionary(selfHandle, runtime);
  }

  auto mapHandle =
      runtime.makeHandle<DictPropertyMap>(classHandle->propertyMap_);

  auto changeFlags = [&flagsToClear,
                      &flagsToSet](NamedPropertyDescriptor &desc) {
    desc.flags.changeFlags(flagsToClear, flagsToSet);
  };

  // If we have the subset of properties to update, only update them; otherwise,
  // update all properties.
  if (props) {
    // Iterate over the properties that exist on the property map.
    for (auto id : *props) {
      auto pos = DictPropertyMap::find(*mapHandle, id);
      if (!pos) {
        continue;
      }
      auto descPair = DictPropertyMap::getDescriptorPair(*mapHandle, *pos);
      changeFlags(descPair->second);
    }
  } else {
    DictPropertyMap::forEachMutablePropertyDescriptor(
        mapHandle, runtime, changeFlags);
  }

  return std::move(classHandle);
}

CallResult<std::pair<Handle<HiddenClass>, SlotIndex>> HiddenClass::reserveSlot(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  assert(
      !selfHandle->isDictionary() &&
      "Reserved slots can only be added in class mode");
  SlotIndex index = selfHandle->numProperties_;
  assert(
      index < InternalProperty::NumAnonymousInternalProperties &&
      "Reserved slot index is too large");

  return HiddenClass::addProperty(
      selfHandle,
      runtime,
      InternalProperty::getSymbolID(index),
      PropertyFlags{});
}

bool HiddenClass::areAllNonConfigurable(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  if (selfHandle->flags_.allNonConfigurable)
    return true;

  if (!forEachPropertyWhile(
          selfHandle,
          runtime,
          [](Runtime &, SymbolID, NamedPropertyDescriptor desc) {
            return !desc.flags.configurable;
          })) {
    return false;
  }

  selfHandle->flags_.allNonConfigurable = true;
  return true;
}

bool HiddenClass::areAllReadOnly(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  if (selfHandle->flags_.allReadOnly)
    return true;

  if (!forEachPropertyWhile(
          selfHandle,
          runtime,
          [](Runtime &, SymbolID, NamedPropertyDescriptor desc) {
            if (!desc.flags.accessor && desc.flags.writable)
              return false;
            return !desc.flags.configurable;
          })) {
    return false;
  }

  selfHandle->flags_.allNonConfigurable = true;
  selfHandle->flags_.allReadOnly = true;
  return true;
}

ExecutionStatus HiddenClass::addToPropertyMap(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    SymbolID name,
    NamedPropertyDescriptor desc) {
  assert(selfHandle->propertyMap_ && "the property map must be initialized");

  // Add the new field to the property map.
  MutableHandle<DictPropertyMap> updatedMap{
      runtime, selfHandle->propertyMap_.get(runtime)};

  if (LLVM_UNLIKELY(
          DictPropertyMap::add(updatedMap, runtime, name, desc) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  selfHandle->propertyMap_.setNonNull(runtime, *updatedMap, runtime.getHeap());
  return ExecutionStatus::RETURNED;
}

void HiddenClass::initializeMissingPropertyMap(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  assert(!selfHandle->propertyMap_ && "property map is already initialized");

  // Check whether we can steal our parent's map. If we can, we only need
  // to add or update a single property.
  if (selfHandle->parent_ &&
      selfHandle->parent_.getNonNull(runtime)->propertyMap_)
    return stealPropertyMapFromParent(selfHandle, runtime);

  LLVM_DEBUG(
      dbgs() << "Class:" << selfHandle->getDebugAllocationId()
             << " allocating new map\n");

  // First collect all entries in reverse order. This avoids recursion.
  using MapEntry = std::pair<SymbolID, PropertyFlags>;
  llvh::SmallVector<MapEntry, 4> entries;
  entries.reserve(selfHandle->numProperties_);
  {
    // Walk chain of parents using raw pointers.
    NoAllocScope _(runtime);
    for (auto *cur = *selfHandle; cur->numProperties_ > 0;
         cur = cur->parent_.getNonNull(runtime)) {
      auto tmpFlags = cur->propertyFlags_;
      tmpFlags.flagsTransition = 0;
      entries.emplace_back(cur->symbolID_, tmpFlags);
    }
  }

  assert(
      entries.size() <= DictPropertyMap::getMaxCapacity() &&
      "There shouldn't ever be this many properties");
  // Allocate the map with the correct size.
  auto res = DictPropertyMap::create(
      runtime,
      std::max(
          (DictPropertyMap::size_type)entries.size(),
          toRValue(DictPropertyMap::DEFAULT_CAPACITY)));
  assert(
      res != ExecutionStatus::EXCEPTION &&
      "Since the entries would fit, there shouldn't be an exception");
  MutableHandle<DictPropertyMap> mapHandle{runtime, res->get()};

  // Add the collected entries in reverse order. Note that there could be
  // duplicates.
  SlotIndex slotIndex = 0;
  for (auto it = entries.rbegin(), e = entries.rend(); it != e; ++it) {
    auto inserted = DictPropertyMap::findOrAdd(mapHandle, runtime, it->first);
    assert(
        inserted != ExecutionStatus::EXCEPTION &&
        "Space was already reserved, this couldn't have grown");

    inserted->first->flags = it->second;
    // If it is a new property, allocate the next slot.
    if (LLVM_LIKELY(inserted->second))
      inserted->first->slot = slotIndex++;
  }

  selfHandle->propertyMap_.setNonNull(runtime, *mapHandle, runtime.getHeap());
}

void HiddenClass::stealPropertyMapFromParent(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime) {
  // Most of this method uses raw pointers.
  NoAllocScope noAlloc(runtime);
  auto *self = *selfHandle;
  assert(
      self->parent_ && self->parent_.getNonNull(runtime)->propertyMap_ &&
      !self->propertyMap_ &&
      "stealPropertyMapFromParent() must be called with a valid parent with a property map");

  LLVM_DEBUG(
      dbgs() << "Class:" << self->getDebugAllocationId()
             << " stealing map from parent Class:"
             << self->parent_.getNonNull(runtime)->getDebugAllocationId()
             << "\n");

  // Success! Just steal our parent's map and add our own property.
  self->propertyMap_.set(
      runtime,
      self->parent_.getNonNull(runtime)->propertyMap_,
      runtime.getHeap());
  self->parent_.getNonNull(runtime)->propertyMap_.setNull(runtime.getHeap());

  // Does our class add a new property?
  if (LLVM_LIKELY(!self->propertyFlags_.flagsTransition)) {
    // This is a new property that we must now add.
    assert(
        self->numProperties_ - 1 ==
            self->propertyMap_.getNonNull(runtime)->size() &&
        "propertyMap->size() must match HiddenClass::numProperties-1 in "
        "new prop transition");

    // Create a descriptor for our property.
    NamedPropertyDescriptor desc{
        self->propertyFlags_, self->numProperties_ - 1};
    // Return to handle mode to add the property.
    noAlloc.release();
    addToPropertyMap(selfHandle, runtime, selfHandle->symbolID_, desc);
    return;
  }
  // Our class is updating the flags of an existing property. So we need
  // to find it and update it.

  assert(
      self->numProperties_ == self->propertyMap_.getNonNull(runtime)->size() &&
      "propertyMap->size() must match HiddenClass::numProperties in "
      "flag update transition");

  auto pos =
      DictPropertyMap::find(self->propertyMap_.get(runtime), self->symbolID_);
  assert(pos && "property must exist in flag update transition");
  auto tmpFlags = self->propertyFlags_;
  tmpFlags.flagsTransition = 0;
  DictPropertyMap::getDescriptorPair(self->propertyMap_.get(runtime), *pos)
      ->second.flags = tmpFlags;
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
