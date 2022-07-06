/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HIDDENCLASS_H
#define HERMES_VM_HIDDENCLASS_H

#include "hermes/Support/OptValue.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/DictPropertyMap.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/PropertyDescriptor.h"
#include "hermes/VM/SegmentedArray.h"
#include "hermes/VM/WeakValueMap.h"

#include <functional>
#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace vm {

/// The storage type used for properties. Its size may be restricted depending
/// on the current configuration, for example because it must fit in a single
/// heap segment.
using PropStorage = ArrayStorageSmall;

/// The storage type used for large arrays that don't necessarily fit in a
/// single heap segment.
using BigStorage = SegmentedArray;

/// Flags associated with a hidden class.
struct ClassFlags {
  /// This class is in dictionary mode, meaning that adding and removing fields
  /// doesn't cause transitions but simply updates the property map.  (We may
  /// still change hidden classes; see dictionaryNoCacheMode, below).
  uint8_t dictionaryMode : 1;

  /// If dictionaryMode is set, this indicates whether the hidden class can
  /// be used as the key in property caches.  If we delete properties, or update
  /// properties, we create a new hidden class for the owning object
  /// (to invalidate any property caches referencing the old hidden
  /// class).  We may decide to limit the number of hidden classes
  /// created this way (currently we allow just one).  To do this, we
  /// set this property of the hidden class property, so that the new
  /// hidden class is never added to an property cache.
  uint8_t dictionaryNoCacheMode : 1;

  /// Set when we have index-like named properties (e.g. "0", "1", etc) defined
  /// using defineOwnProperty. Array accesses will have to check the named
  /// properties first. The absence of this flag is important as it indicates
  /// that named properties whose name is an integer index don't need to be
  /// searched for - they don't exist.
  uint8_t hasIndexLikeProperties : 1;

  /// All properties in this class are non-configurable. This flag can sometimes
  /// be set lazily, after we have checked whether all properties are non-
  /// configurable.
  uint8_t allNonConfigurable : 1;

  /// All properties in this class are both non-configurable and non-writable.
  /// It imples that \c allNonConfigurable is also set.
  /// This flag can sometimes be set lazily, after we have checked whether all
  /// properties are "read-only".
  uint8_t allReadOnly : 1;

  ClassFlags() {
    ::memset(this, 0, sizeof(*this));
  }
};

/// A "hidden class" describes a fixed set of properties, their property flags
/// and the order that they were created in. It is logically immutable (unless
/// it is in "dictionary mode", which is described below).
///
/// Overview
/// ========
/// Adding, deleting or updating a property of a "hidden class" is represented
/// as a transition to a new "hidden class", which encodes the new state of the
/// property set. We call the old class a "parent" and the new class a
/// "child". Starting from a given parent class, its children and their
/// children (etc...) form a tree.
///
/// Each class contains a transition table from itself to its children, keyed on
/// the new/updated property name (SymbolID) and the new/updated property flags
/// that caused each child to be created.
///
/// When a new empty JavaScript object is created, it is assigned an empty
/// "root" hidden class. Adding a new property causes a transition from the
/// root class to a new child class and the transition is recorded in the root
/// class transition table. Adding a second property causes another class to
/// be allocated and a transition to be recorded in its parent, and so on.
/// When a second empty JavaScript object is created and the same properties
/// are added in the same order, the existing classes will be found by looking
/// up in each transition table.
///
/// In this way, JavaScript objects which have the same properties added in the
/// same order will end up having the same hidden class identifying their set
/// of properties. That can decreases the memory dramatically (because we have
/// only one set description per class instead of one per object) and can be
/// used for caching property offsets and other attributes.
///
/// Dictionary Mode
/// ===============
/// When more than a predefined number of properties are added (\c
/// kDictionaryThreshold) or if a property is deleted, a new class is created
/// without a parent and placed in "dictionary mode". In that mode the class
/// is not shared - it belongs to exactly one object - and updates are done "in
/// place" instead of creating new child classes.
///
/// Property Maps
/// =============
/// Conceptually every hidden class has a property map - a table mapping from
/// a property name (SymbolID) to a property descriptor (slot + flags).
///
/// In order to conserve memory, we create the property map associated with a
/// class the first time it is needed. To delay creation further, if we are
/// looking for a property for a "put-by-name" operation, we can avoid needing
/// the map by looking for the property in the transition table first. Lastly,
/// when we transition from a parent class to a child class, we "steal" the
/// parent's property map and assign it to the child.
///
/// The desired effect is that only "leaf" classes have property maps and normal
/// property assignment doesn't create a map at all in the intermediate states
/// (except the first time).
class HiddenClass;
namespace detail {
/// Encode a transition from a hidden class to a child, keyed on the
/// name of the property and its property flags.
/// This is an internal type but has to be made public so we can define
/// a llvh::DenseMapInfo<> trait for it.
class Transition {
 public:
  SymbolID symbolID;
  PropertyFlags propertyFlags;

  /// An explicit constructor for creating DenseMap sentinel values.
  explicit Transition(SymbolID symbolID)
      : symbolID(symbolID), propertyFlags() {}
  Transition(SymbolID symbolID, PropertyFlags flags)
      : symbolID(symbolID), propertyFlags(flags) {}

  bool operator==(const Transition &a) const {
    return symbolID == a.symbolID && propertyFlags == a.propertyFlags;
  }
};

/// A front for WeakValueMap<Transition, HiddenClass> that is space-optimized
/// for the common case of 0 or 1 entry. See WeakValueMap for more details.
class TransitionMap {
 public:
  ~TransitionMap() {
    if (isLarge())
      delete large();
  }

  /// Return true if there is an entry with the given key and a valid value.
  bool containsKey(const Transition &key, GC &gc) {
    return (smallKey_ == key && smallValue().isValid()) ||
        (isLarge() && large()->containsKey(key));
  }

  /// Look for a \p key and return the corresponding HiddenClass, or nullptr if
  /// it is not found.
  HiddenClass *lookup(Runtime &runtime, const Transition &key) {
    if (smallKey_ == key) {
      return smallValue().get(runtime);
    } else if (isLarge()) {
      return large()->lookup(runtime, key);
    } else {
      return nullptr;
    }
  }

  /// Insert a key/value into the map if the key is not already there.
  /// \return true if it was inserted, false if the key was already there.
  bool insertNew(
      Runtime &runtime,
      const Transition &key,
      Handle<HiddenClass> value) {
    assert(
        key.symbolID != SymbolID::empty() &&
        "Should never insert an empty key");
    if (smallKey_ == key && smallValue().isValid()) {
      return false;
    }
    // Need to hold the lock when mutating smallKey and smallValue.
    WeakRefLock lk{runtime.getHeap().weakRefMutex()};
    if (isClean()) {
      smallKey_ = key;
      smallValue() = WeakRef<HiddenClass>(runtime, value);
      return true;
    }
    if (!isLarge())
      uncleanMakeLarge(runtime);
    return large()->insertNewLocked(runtime, key, value);
  }

  /// Insert key/value into the map. Used by deserialization.
  void insertUnsafe(Runtime &runtime, const Transition &key, WeakRefSlot *ptr);

  /// Accepts every valid WeakRef in the map.
  void markWeakRefs(WeakRefAcceptor &acceptor) {
    if (isLarge()) {
      large()->markWeakRefs(acceptor);
    } else if (!isClean()) {
      acceptor.accept(smallValue());
    }
  }

  /// \return estimated dynamically allocated memory owned by this map.
  size_t getMemorySize() const;

  /// \return true if the map is known to be empty. May have false negatives.
  bool isKnownEmpty() const {
    return isClean() || (isLarge() && large()->isKnownEmpty());
  }

  /// Invoke \p callback on each (const) key and value. Values may be invalid.
  template <typename CallbackFunction>
  void forEachEntry(const CallbackFunction &callback) const {
    if (isLarge()) {
      large()->forEachEntry(callback);
    } else if (!isClean()) {
      callback(smallKey_, smallValue());
    }
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  void snapshotAddNodes(GC &gc, HeapSnapshot &snap);
  void snapshotAddEdges(GC &gc, HeapSnapshot &snap);
  void snapshotUntrackMemory(GC &gc);
#endif

 private:
  /// Clean = no transition has been inserted since construction.
  bool isClean() const {
    return smallKey_.symbolID == SymbolID::empty();
  }

  /// Large = allocated WeakValueMap contains any/all entries.
  bool isLarge() const {
    return smallKey_.symbolID == SymbolID::deleted();
  }

  /// Expand to large mode, assuming already unclean.
  void uncleanMakeLarge(Runtime &runtime);

  /// Accessors for each union member after asserting it's active.
  WeakRef<HiddenClass> &smallValue() {
    assert(!isLarge());
    return u.smallValue_;
  }
  const WeakRef<HiddenClass> &smallValue() const {
    assert(!isLarge());
    return u.smallValue_;
  }
  WeakValueMap<Transition, HiddenClass> *large() const {
    assert(isLarge());
    return u.large_;
  }

  Transition smallKey_{SymbolID::empty()};
  union U {
    U() : smallValue_((WeakRefSlot *)nullptr) {}
    WeakRef<HiddenClass> smallValue_;
    WeakValueMap<Transition, HiddenClass> *large_;
  } u;
};

} // namespace detail

class HiddenClass final : public GCCell {
  friend void HiddenClassBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 public:
  using Transition = detail::Transition;
  /// Adding more than this number of properties will switch to "dictionary
  /// mode".
  static constexpr unsigned kDictionaryThreshold = 64;

  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::HiddenClassKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::HiddenClassKind;
  }

  /// Create a "root" hidden class - one that doesn't define any properties, but
  /// is a starting point for a hierarchy.
  static CallResult<HermesValue> createRoot(Runtime &runtime);

  /// \return true if this hidden class is guaranteed to be a leaf.
  /// It can return false negatives, so it should only be used for stats
  /// reporting and such.
  bool isKnownLeaf() const {
    return transitionMap_.isKnownEmpty();
  }

  /// \return the number of own properties described by this hidden class.
  /// This corresponds to the size of the property map, if it is initialized.
  unsigned getNumProperties() const {
    return numProperties_;
  }

  /// \return true if this class is in "dictionary mode" - i.e. changes to it
  /// don't (normally) result in creation of new classes.
  bool isDictionary() const {
    return flags_.dictionaryMode;
  }

  /// \return true if this class is in "non-cacheable dictionary mode"
  /// - it is a dictionary, and the owning object has been modified in
  /// a way since becoming a dictionary that precludes inline caching
  /// (for example, a property has been deleted or updated).
  bool isDictionaryNoCache() const {
    assert(
        (!flags_.dictionaryNoCacheMode || flags_.dictionaryMode) &&
        "dictionaryNoCacheMode should only be set if dictionaryMode is set.");
    return flags_.dictionaryNoCacheMode;
  }

  bool getHasIndexLikeProperties() const {
    return flags_.hasIndexLikeProperties;
  }

  /// \return The for-in cache if one has been set, otherwise nullptr.
  BigStorage *getForInCache(Runtime &runtime) const {
    return forInCache_.get(runtime);
  }

  void setForInCache(BigStorage *arr, Runtime &runtime) {
    forInCache_.set(runtime, arr, runtime.getHeap());
  }

  void clearForInCache(Runtime &runtime) {
    forInCache_.setNull(runtime.getHeap());
  }

  /// Reset the property map, unless this class is in dictionary mode.
  /// May be called by the GC for any HiddenClass not in a Handle.
  void clearPropertyMap(GC &gc) {
    if (!isDictionary())
      propertyMap_.setNull(gc);
  }

  /// An opaque class representing a reference to a valid property in the
  /// property map.
  using PropertyPos = DictPropertyMap::PropertyPos;

  /// Call the supplied callback pass each property's \c SymbolID and \c
  /// NamedPropertyDescriptor as parameters.
  /// Obviously the callback shouldn't be doing naughty things like modifying
  /// the property map or creating new hidden classes (even implicitly).
  /// A marker for the current gcScope is obtained in the beginning and the
  /// scope is flushed after every callback.
  template <typename CallbackFunction>
  static void forEachProperty(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      const CallbackFunction &callback);

  /// Same as \p forEachProperty, but the callback cannot do any GC operations,
  /// such as allocating objects, modifying objects, or creating handles.
  /// \p forEachProperty is allowed to allocate, and thus can steal and cache
  /// the property map for the next query, so it is preferred.
  static void forEachPropertyNoAlloc(
      HiddenClass *self,
      PointerBase &base,
      std::function<void(SymbolID, NamedPropertyDescriptor)> callback);

  /// Same as forEachProperty() but the callback returns true to continue or
  /// false to stop immediately.
  /// A marker for the current gcScope is obtained in the beginning and the
  /// scope is flushed after every callback.
  /// \return false if the callback returned false, true otherwise.
  template <typename CallbackFunction>
  static bool forEachPropertyWhile(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      const CallbackFunction &callback);

  /// Look for a property in the property map. If the property is found, return
  /// a \c PropertyPos identifying it and store its descriptor in \p desc.
  /// \param expectedFlags if valid, we can search the transition table for this
  ///   property with these precise flags. If found in the transition table,
  ///   we don't need to create a property map.
  /// \return the "position" of the property, if found.
  static OptValue<PropertyPos> findProperty(
      PseudoHandle<HiddenClass> self,
      Runtime &runtime,
      SymbolID name,
      PropertyFlags expectedFlags,
      NamedPropertyDescriptor &desc);

  /// Same operation as \p findProperty, but does not do any allocations.
  /// This is slower than \p findProperty because it needs to traverse the full
  /// hidden class chain in the worst case.
  static llvh::Optional<NamedPropertyDescriptor>
  findPropertyNoAlloc(HiddenClass *self, PointerBase &base, SymbolID name);

  /// An optimistic fast path for \c findProperty(). If there is an allocated
  /// property map, this will return an OptValue containing either true or
  /// false. If there was no allocated property map, this returns llvh::None. If
  /// this fails by returning None, the "slow path", \c findProperty() itself,
  /// must be used.
  static OptValue<bool> tryFindPropertyFast(
      const HiddenClass *self,
      PointerBase &base,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// Performs a very slow linear search for the specified property. This should
  /// only be used for debug tests where we don't want to allocate a property
  /// map because doing so would change the behavior.
  /// \return true if the property is defined, false otherwise.
  static bool
  debugIsPropertyDefined(HiddenClass *self, PointerBase &base, SymbolID name);

  /// Delete a property which we found earlier using \c findProperty.
  /// \return the resulting new class.
  static Handle<HiddenClass> deleteProperty(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      PropertyPos pos);

  /// Add a new property. It must not already exist.
  /// \return the resulting new class and the index of the new property.
  static CallResult<std::pair<Handle<HiddenClass>, SlotIndex>> addProperty(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      SymbolID name,
      PropertyFlags propertyFlags);

  /// Update an existing property's flags and return the resulting class.
  /// \param pos is the position of the property into the property map.
  static Handle<HiddenClass> updateProperty(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      PropertyPos pos,
      PropertyFlags newFlags);

  /// Mark all properties as non-configurable.
  /// \return the resulting class
  static Handle<HiddenClass> makeAllNonConfigurable(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// Mark all properties as non-writable and non-configurable.
  /// \return the resulting class
  static Handle<HiddenClass> makeAllReadOnly(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// Update the flags for the properties in the list \p props with \p
  /// flagsToClear and \p flagsToSet. If in dictionary mode, the properties are
  /// updated on the hidden class directly; otherwise, create a new dictionary
  /// hidden class as result. Updating the properties mutates the property map
  /// directly without creating transitions.
  /// \p flagsToClear and \p flagsToSet are masks for updating the property
  /// flags.
  /// \p props is a list of SymbolIDs for properties that need to be updated
  /// made read-only. It should contain a subset of properties in the hidden
  /// class, so the SymbolIDs won't get freed by gc. It can be llvh::None; if it
  /// is llvh::None, update every property.
  /// \return the resulting hidden class.
  static Handle<HiddenClass> updatePropertyFlagsWithoutTransitions(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      PropertyFlags flagsToClear,
      PropertyFlags flagsToSet,
      OptValue<llvh::ArrayRef<SymbolID>> props);

  /// Create a new class where the next slot is reserved, by calling addProperty
  /// with an internal property name. Only slots with index less than
  /// InternalProperty::NumAnonymousInternalProperties can be reserved.
  /// \param selfHandle must not be in dictionary mode.
  /// \return the resulting new class and the index of the reserved slot.
  static CallResult<std::pair<Handle<HiddenClass>, SlotIndex>> reserveSlot(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// \return true if all properties are non-configurable
  static bool areAllNonConfigurable(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// \return true if all properties are non-writable and non-configurable
  static bool areAllReadOnly(Handle<HiddenClass> selfHandle, Runtime &runtime);

  HiddenClass(
      Runtime &runtime,
      ClassFlags flags,
      Handle<HiddenClass> parent,
      SymbolID symbolID,
      PropertyFlags propertyFlags,
      unsigned numProperties)
      : symbolID_(symbolID),
        propertyFlags_(propertyFlags),
        flags_(flags),
        numProperties_(numProperties),
        parent_(runtime, *parent, runtime.getHeap()) {
    assert(propertyFlags.isValid() && "propertyFlags must be valid");
  }

 private:
  /// Allocate a new hidden class instance with the supplied parameters.
  static CallResult<HermesValue> create(
      Runtime &runtime,
      ClassFlags flags,
      Handle<HiddenClass> parent,
      SymbolID symbolID,
      PropertyFlags propertyFlags,
      unsigned numProperties);

  /// Create a copy of this \c HiddenClass and ensure that the copy is
  /// in dictionary mode.  Requires that the current \C HiddenClass
  /// does not have the dictionaryNoCacheMode flag set; such
  /// dictionaries may not be copied.  If the current class has a
  /// property map, it will be moved to the new class. Otherwise a new
  /// property map will be created for the new class. In either case,
  /// the current class will have no property map and the new class
  /// will have one.  The \p noCache argument indicates whether the
  /// new dictionary \c HiddenClass's dictionaryNoCacheMode flag will
  /// be set.  \return the new class.
  static Handle<HiddenClass> copyToNewDictionary(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      bool noCache = false);

  /// Add a new property pair (\p name and \p desc) to the property map (which
  /// must have been initialized).
  static ExecutionStatus addToPropertyMap(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime,
      SymbolID name,
      NamedPropertyDescriptor desc);

  /// Construct a property map by walking back the chain of hidden classes and
  /// store it in \c propertyMap_.
  static void initializeMissingPropertyMap(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// Initialize the property map by transferring the parent's map to ourselves
  /// and adding a our property to it. It must only be called if we don't have a
  /// property map of our own but have a valid parent with a property map.
  static void stealPropertyMapFromParent(
      Handle<HiddenClass> selfHandle,
      Runtime &runtime);

  /// Free all non-GC managed resources associated with the object.
  static void _finalizeImpl(GCCell *cell, GC &gc);

  /// Mark all the weak references for an object.
  static void _markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor);

  /// \return the amount of non-GC memory being used by the given \p cell, which
  /// is assumed to be a HiddenClass.
  static size_t _mallocSizeImpl(GCCell *cell);

#ifdef HERMES_MEMORY_INSTRUMENTATION
  static std::string _snapshotNameImpl(GCCell *cell, GC &gc);
  static void _snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
#endif

 private:
  /// The symbol that was added when transitioning to this hidden class.
  const GCSymbolID symbolID_;
  /// The flags of the added symbol.
  const PropertyFlags propertyFlags_;

  /// Flags associated with this hidden class.
  ClassFlags flags_{};

  /// Total number of properties encoded in the entire chain from this class
  /// to the root. Note that some transitions do not introduce a new property,
  /// so this is not the same as the length of the transition chain.
  /// Before we enter "dictionary mode", this determines the offset of a new
  /// property.
  unsigned numProperties_;

  /// Optional property map of all properties defined by this hidden class.
  /// This includes \c symbolID_, \c parent_->symbolID_, \c
  /// parent_->parent_->symbolID_ and so on (in reverse order).
  /// It is constructed lazily when needed, or is "stolen" from the parent class
  /// when a transition is performed from the parent class to this one.
  ///
  /// NOTE: May be cleared by the GC for any HiddenClass not in a Handle.
  GCPointer<DictPropertyMap> propertyMap_{};

  /// This hash table encodes the transitions from this class to child classes
  /// keyed on the property being added (or updated) and its flags.
  detail::TransitionMap transitionMap_;

  /// The parent hidden class which contains a transition from itself to this
  /// one keyed on \c symbolID_+propertyFlags_. It can be null if there is no
  /// parent.
  GCPointer<HiddenClass> parent_;

  /// Cache that contains for-in property names for objects of this class.
  /// Never used in dictionary mode.
  GCPointer<BigStorage> forInCache_{};
};

//===----------------------------------------------------------------------===//
// HiddenClass inline methods.

template <typename CallbackFunction>
void HiddenClass::forEachProperty(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    const CallbackFunction &callback) {
  if (LLVM_UNLIKELY(!selfHandle->propertyMap_))
    initializeMissingPropertyMap(selfHandle, runtime);

  return DictPropertyMap::forEachProperty(
      runtime.makeHandle(selfHandle->propertyMap_), runtime, callback);
}

template <typename CallbackFunction>
bool HiddenClass::forEachPropertyWhile(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    const CallbackFunction &callback) {
  if (LLVM_UNLIKELY(!selfHandle->propertyMap_))
    initializeMissingPropertyMap(selfHandle, runtime);

  return DictPropertyMap::forEachPropertyWhile(
      runtime.makeHandle(selfHandle->propertyMap_), runtime, callback);
}

inline OptValue<bool> HiddenClass::tryFindPropertyFast(
    const HiddenClass *self,
    PointerBase &base,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  if (LLVM_LIKELY(self->propertyMap_)) {
    auto found =
        DictPropertyMap::find(self->propertyMap_.getNonNull(base), name);
    if (LLVM_LIKELY(found)) {
      desc = DictPropertyMap::getDescriptorPair(
                 self->propertyMap_.getNonNull(base), *found)
                 ->second;
    }
    return found.hasValue();
  } else if (self->numProperties_ == 0) {
    return false;
  }
  return llvh::None;
}

} // namespace vm
} // namespace hermes

// Enable using HiddenClass::Transition in DenseMap.
namespace llvh {

using namespace hermes::vm;

template <>
struct DenseMapInfo<HiddenClass::Transition> {
  static inline HiddenClass::Transition getEmptyKey() {
    return HiddenClass::Transition(SymbolID::empty());
  }

  static inline HiddenClass::Transition getTombstoneKey() {
    return HiddenClass::Transition(SymbolID::deleted());
  }

  static inline unsigned getHashValue(HiddenClass::Transition transition) {
    return transition.symbolID.unsafeGetRaw() ^ transition.propertyFlags._flags;
  }

  static inline bool isEqual(
      const HiddenClass::Transition &a,
      const HiddenClass::Transition &b) {
    return a == b;
  }
};

} // namespace llvh

#endif // HERMES_VM_HIDDENCLASS_H
