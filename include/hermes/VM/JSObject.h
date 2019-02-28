/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_JSOBJECT_H
#define HERMES_VM_JSOBJECT_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PropertyDescriptor.h"
#include "hermes/VM/TypesafeFlags.h"
#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

union DefinePropertyFlags {
  struct {
    uint32_t enumerable : 1;
    uint32_t writable : 1;
    uint32_t configurable : 1;

    uint32_t setEnumerable : 1;
    uint32_t setWritable : 1;
    uint32_t setConfigurable : 1;
    uint32_t setGetter : 1;
    uint32_t setSetter : 1;
    uint32_t setValue : 1;
    /// If set, indicates that the \c internalSetter flag must be set to true.
    /// This is strictly for internal use only, inside the object model.
    uint32_t enableInternalSetter : 1;
  };

  uint32_t _flags;

  /// Clear all flags on construction.
  DefinePropertyFlags() {
    _flags = 0;
  }

  /// \return true if all flags are clear.
  bool isEmpty() const {
    return _flags == 0;
  }

  /// Clear all bits.
  void clear() {
    _flags = 0;
  }

  /// \return true if this is an accessor.
  bool isAccessor() const {
    return setGetter || setSetter;
  }

  /// Return an instance of DefinePropertyFlags initialized for defining a
  /// "normal" property: writable, enumerable, configurable and setting its
  /// non-accessor value.
  static DefinePropertyFlags getDefaultNewPropertyFlags() {
    DefinePropertyFlags dpf{};
    dpf.setEnumerable = 1;
    dpf.enumerable = 1;
    dpf.setWritable = 1;
    dpf.writable = 1;
    dpf.setConfigurable = 1;
    dpf.configurable = 1;
    dpf.setValue = 1;
    return dpf;
  }

  /// Return an instance of DefinePropertyFlags initialized for defining a
  /// property which is writable, configurable and non-enumerable, and setting
  /// its non-accessor value.
  static DefinePropertyFlags getNewNonEnumerableFlags() {
    DefinePropertyFlags dpf{};
    dpf.setEnumerable = 1;
    dpf.enumerable = 0;
    dpf.setWritable = 1;
    dpf.writable = 1;
    dpf.setConfigurable = 1;
    dpf.configurable = 1;
    dpf.setValue = 1;
    return dpf;
  }
};

/// Flags associated with an object.
struct ObjectFlags {
  /// New properties cannot be added.
  uint32_t noExtend : 1;

  /// \c Object.seal() has been invoked on this object, marking all properties
  /// as non-configurable. When \c Sealed is set, \c NoExtend is always set too.
  uint32_t sealed : 1;

  /// \c Object.freeze() has been invoked on this object, marking all properties
  /// as non-configurable and non-writable. When \c Frozen is set, \c Sealed and
  /// must \c NoExtend are always set too.
  uint32_t frozen : 1;

  /// This object has indexed storage. This flag will not change at runtime, it
  /// is set at construction and its value never changes. It is not a state.
  uint32_t indexedStorage : 1;

  /// This flag is set to true when \c IndexedStorage is true and
  /// \c class->hasIndexLikeProperties are false. It allows our fast paths to do
  /// a simple bit check.
  uint32_t fastIndexProperties : 1;

  /// This flag indicates this is a special object whose properties are
  /// managed by C++ code, and not via the standard property storage
  /// mechanisms.
  uint32_t hostObject : 1;

  /// this is lazily created object that must be initialized before it can be
  /// used. Note that lazy objects must have no properties defined on them,
  uint32_t lazyObject : 1;

  static constexpr unsigned kHashWidth = 25;
  /// A non-zero object id value, assigned lazily. It is 0 before it is
  /// assigned. If an object started out as lazy, the objectID is the lazy
  /// object index used to identify when it gets initialized.
  uint32_t objectID : kHashWidth;

  ObjectFlags() {
    ::memset(this, 0, sizeof(*this));
  }
};

static_assert(
    sizeof(ObjectFlags) == sizeof(uint32_t),
    "ObjectFlags must be a single word");

/// \name PropOpFlags
/// @{
/// Flags used when performing property access operations.
///
/// \name ThrowOnError
/// Throw a TypeError exception when one of the following conditions is
/// encountered:
///   - changing a read-only property
///   - reconfigure a non-configurable property
///   - adding a new property to non-extensible object
///   - deleting a non-configurable property
///
/// \name MustExist
/// Throw a type error if the property doesn't exist.
///
/// \name InternalForce
/// Used to insert an internal property, forcing the insertion no matter what.
/// @}
#define HERMES_VM__LIST_PropOpFlags(FLAG) \
  FLAG(ThrowOnError)                      \
  FLAG(MustExist)                         \
  FLAG(InternalForce)

HERMES_VM__DECLARE_FLAGS_CLASS(PropOpFlags, HERMES_VM__LIST_PropOpFlags);

// Any method that could potentially invoke the garbage collector, directly or
// in-directly, cannot use a direct 'self' pointer and must instead use
// Handle<JSObject>.

struct ObjectVTable {
  VTable base;

  /// \return the range of indexes (end-exclusive) stored in indexed storage.
  std::pair<uint32_t, uint32_t> (*getOwnIndexedRange)(JSObject *self);

  /// Check whether property with index \p index exists in indexed storage and
  /// \return true if it does.
  bool (*haveOwnIndexed)(JSObject *self, Runtime *runtime, uint32_t index);

  /// Check whether property with index \p index exists in indexed storage and
  /// extract its \c PropertyFlags (if necessary checking whether the object is
  /// frozen or sealed). Only the \c enumerable, \c writable and
  /// \c configurable flags must be set in the result.
  /// \return PropertyFlags if the property exists.
  OptValue<PropertyFlags> (*getOwnIndexedPropertyFlags)(
      JSObject *self,
      Runtime *runtime,
      uint32_t index);

  /// Obtain an element from the "indexed storage" of this object. The storage
  /// itself is implementation dependent.
  /// \return the value of the element or "empty" if there is no such element.
  HermesValue (
      *getOwnIndexed)(JSObject *self, Runtime *runtime, uint32_t index);

  /// Set an element in the "indexed storage" of this object. Depending on the
  /// semantics of the "indexed storage" the storage capacity may need to be
  /// expanded (e.g. affecting Array.length), or the write may simply be ignored
  /// (in the case of typed arrays).
  /// It is the responsibility of the implementation of the method to check
  /// whether the object is "frozen" and fail. Note that some objects cannot be
  /// frozen, so they don't need to perform that check.
  /// \param value the value to be stored. In some cases (like type arrays), it
  ///     may need to be converted to a certain type. If the conversion fails,
  ///     a default value will be stored instead, but the write will succeed
  ///     (unless there was an exception when converting).
  /// \return true if the write succeeded, false if it was ignored because
  ///   the element is read-only, or exception status.
  CallResult<bool> (*setOwnIndexed)(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value);

  /// Delete an element in the "indexed storage". It is the responsibility of
  /// the implementation of the method to check whether the object is "sealed"
  /// and fail appropriately. Some objects cannot be frozen and don't need to
  /// perform that check at all.
  /// \return 'true' if the element was successfully deleted, or if it was
  ///     outside of the storage range. 'false' if this storage doesn't support
  ///     "holes"/deletion (e.g. typed arrays) or if the element is read-only.
  bool (*deleteOwnIndexed)(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index);

  /// Mode paramater to pass to \c checkAllOwnIndexed().
  enum class CheckAllOwnIndexedMode {
    NonConfigurable,
    /// Both non-configurable and non-writable.
    ReadOnly,
  };

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  bool (*checkAllOwnIndexed)(JSObject *self, CheckAllOwnIndexedMode mode);
};

/// This is intended to act like an opaque type that is actually PropStorage.
/// We use it to enforce at compile time that only the result of
/// JSObject::createPropStorage() can be used to construct the object.
/// Note that it can really be opaque because of our casting mechanics.
class JSObjectPropStorage : public GCCell {
 public:
  static bool classof(const GCCell *cell) {
    return PropStorage::classof(cell);
  }
};

/// This is the basic JavaScript Object class. All programmer-visible classes in
/// JavaScript (like Array, Function, Arguments, Number, String, etc) inherit
/// from it. At the highest level it is simply a collection of name/value
/// property pairs while subclasses provide additional functionality.
///
/// Subclasses can optionally implement "indexed storage". It is an efficient
/// mechanism for storing properties whose names are valid array indexes
/// according to ES5.1 sec 15.4. In other words, for storing arrays with an
/// uint32_t index. If "indexed storage" is available, Object will use it when
/// possible.
///
/// If indexed storage is available, but a numeric property with unusual flags
/// defined (e.g. non-enumerable, non-writable, etc), then the indexed storage
/// has to be "shadowed" by a named property. If at least one such property
/// exists, all indexed accesses must first check for a named property with the
/// same name. It comes with a significant cost, but fortunately such accesses
/// should be extremely rare.
///
/// All methods for accessing and manipulating properties are split into two
/// symmetrical groups: "named" and "computed".
///
/// Named accessors require a SymbolID as the property name and can *ONLY*
/// be used when either of these is true:
/// a) the string representation of the name is not a valid array index
///   according to ES5.1 sec 15.4.
/// b) the object does not have "indexed storage".
///
/// External users of the API cannot rely on b) so in practice "named" accessor
/// must be used only when the property name is known in advance (at compile
/// time) and is not an array index. Internally Object relies on b) to
/// delegate the work to the proper call.
///
/// Computed accessors allow any JavaScript value as the property name.
/// Conceptually the name is converted to a string (using ToString as defined
/// by the spec) and the string is used as a property key. In practice,
/// integer values are detected and used with the "indexed storage", if
/// available.
class JSObject : public GCCell {
  friend void ObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 protected:
  /// A light-weight constructor which performs no GC allocations. Its purpose
  /// to make sure all fields are initialized according to C++ without writing
  /// to them twice.
  template <typename NeedsBarriers>
  JSObject(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      JSObjectPropStorage *propStorage,
      NeedsBarriers needsBarriers)
      : GCCell(&runtime->getHeap(), vtp),
        parent_(parent, &runtime->getHeap(), needsBarriers),
        clazz_(clazz, &runtime->getHeap(), needsBarriers),
        propStorage_(
            unwrapPropStorage(propStorage),
            &runtime->getHeap(),
            needsBarriers) {}

  template <typename NeedsBarriers>
  JSObject(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      NeedsBarriers needsBarriers)
      : GCCell(&runtime->getHeap(), vtp),
        parent_(parent, &runtime->getHeap(), needsBarriers),
        clazz_(clazz, &runtime->getHeap(), needsBarriers),
        propStorage_(nullptr, &runtime->getHeap(), needsBarriers) {}

  /// Until we apply the NeedsBarriers pattern to all subtypes of JSObject, we
  /// will need versions that do not take the extra NeedsBarrier argument
  /// (defaulting to NoBarriers).
  JSObject(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      JSObjectPropStorage *propStorage)
      : JSObject(
            runtime,
            vtp,
            parent,
            clazz,
            propStorage,
            GCPointerBase::NoBarriers()) {}

  JSObject(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz)
      : JSObject(runtime, vtp, parent, clazz, GCPointerBase::NoBarriers()) {}

 public:
  static ObjectVTable vt;

  /// Default capacity of indirect property storage.
  static const PropStorage::size_type DEFAULT_PROPERTY_CAPACITY = 4;

  /// Number of property slots the class reserves for itself. Child classes
  /// should override this value by adding to it and defining a constant with
  /// the same name.
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS = 0;

  /// Number of property slots allocated directly inside the object.
  static const PropStorage::size_type DIRECT_PROPERTY_SLOTS = 6;

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(), CellKind::ObjectKind_first, CellKind::ObjectKind_last);
  }

  /// Attempts to allocate a JSObject with the given prototype.
  /// If allocation fails, the GC declares an OOM.
  static PseudoHandle<JSObject> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle);

  /// Attempts to allocate a JSObject with the standard Object prototype.
  /// If allocation fails, the GC declares an OOM.
  static PseudoHandle<JSObject> create(Runtime *runtime);

  /// Attempts to allocate a JSObject with the standard Object prototype and
  /// property storage preallocated. If allocation fails, the GC declares an
  /// OOM.
  /// \param propertyCount number of property storage slots preallocated.
  static PseudoHandle<JSObject> create(
      Runtime *runtime,
      unsigned propertyCount);

  /// Allocates a JSObject with the given hidden class and property storage
  /// preallocated. If allocation fails, the GC declares an
  /// OOM.
  /// \param clazz the hidden class for the new object.
  static PseudoHandle<JSObject> create(
      Runtime *runtime,
      Handle<HiddenClass> clazz);

  /// Attempts to allocate a JSObject and returns whether it succeeded or not.
  /// NOTE: This function always returns \c ExecutionStatus::RETURNED, it is
  /// only used in interfaces where other creators may throw a JS exception.
  static CallResult<HermesValue> createWithException(
      Runtime *runtime,
      Handle<JSObject> parentHandle);

  ~JSObject() = default;

  /// Allocate an instance of property storage with the specified capacity.
  static inline CallResult<Handle<JSObjectPropStorage>> createPropStorage(
      Runtime *runtime,
      PropStorage::size_type capacity);

  /// Allocate an instance of property storage with the specified capacity and
  /// size.
  /// Requires that \p size <= \p capacity.
  static inline CallResult<Handle<JSObjectPropStorage>> createPropStorage(
      Runtime *runtime,
      PropStorage::size_type capacity,
      PropStorage::size_type size);

  bool isExtensible() const {
    return !flags_.noExtend;
  }

  /// true if this a lazy object that must be initialized prior to use.
  bool isLazy() const {
    return flags_.lazyObject;
  }

  /// \return true if this is a HostObject.
  bool isHostObject() const {
    return flags_.hostObject;
  }

  /// \return the `__proto__` internal property, which may be nullptr.
  JSObject *getParent() const {
    return parent_;
  }

  /// \return the hidden class of this object.
  HiddenClass *getClass() const {
    return clazz_;
  }

  /// \return the object ID. Assign one if not yet exist. This ID can be used
  /// in Set or Map where hashing is required. We don't assign object an ID
  /// until we actually need it. An exception is lazily created objects where
  /// the object id is the provided lazy object index which is used when the
  /// object gets initialized.
  static ObjectID getObjectID(JSObject *self, Runtime *runtime);

  static void initializeLazyObject(
      Runtime *runtime,
      Handle<JSObject> lazyObject);

  /// Get the objectID, which must already have been assigned using \c
  /// getObjectID().
  ObjectID getAlreadyAssignedObjectID() const {
    assert(flags_.objectID && "ObjectID hasn't been assigned yet");
    return flags_.objectID;
  }

  /// Whether the set of properties owned by this object is uniquely defined
  /// by the identity of its hidden class.
  inline bool shouldCacheForIn() const;

  /// Sets the internal prototype property. This corresponds to ES6 9.1.2
  /// [[SetPrototypeOf]].
  /// - Does nothing if the value doesn't change.
  /// - Fails if the object isn't extensible
  /// - Fails if it detects a prototype cycle.
  static ExecutionStatus
  setParent(JSObject *self, Runtime *runtime, JSObject *parent);

  /// Allocate internal properties - it reserves \p count slots, starting from
  /// index 0, which are not accessible by name. This method can be called
  /// exactly once per object, before any other properties have been added.
  /// The new properties are initialized to \p valueHandle.
  static void addInternalProperties(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      unsigned count,
      Handle<> valueHandle);

  static HermesValue getInternalProperty(JSObject *self, SlotIndex index);

  static void setInternalProperty(
      JSObject *self,
      Runtime *runtime,
      SlotIndex index,
      HermesValue value);

  /// Return a list of property names belonging to this object. Indexed property
  /// names will be represented as numbers for efficiency. The order of
  /// properties follows ES2015 - first properties whose string names look like
  /// indexes, in numeric order, then the rest, in insertion order.
  /// \param onlyEnumerable if true, only enumerable properties will be
  ///   returned.
  /// \returns a JSArray containing the names.
  static CallResult<Handle<JSArray>> getOwnPropertyNames(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      bool onlyEnumerable);

  /// Return a list of property symbols keys belonging to this object.
  /// The order of properties follows ES2015 - insertion order.
  /// \returns a JSArray containing the symbols.
  static CallResult<Handle<JSArray>> getOwnPropertySymbols(
      Handle<JSObject> selfHandle,
      Runtime *runtime);

  /// Load a value from the "named value" storage space by \p index.
  /// \pre inl == PropStorage::Inline::Yes -> index <
  /// PropStorage::kValueToSegmentThreshold.
  template <PropStorage::Inline inl = PropStorage::Inline::No>
  static HermesValue getNamedSlotValue(JSObject *self, SlotIndex index);
  /// Load a value from the "named value" storage space by the slot described by
  /// the property descriptor \p desc.
  static HermesValue getNamedSlotValue(
      JSObject *self,
      NamedPropertyDescriptor desc);

  /// Store a value to the "named value" storage space by \p index.
  /// \pre inl == PropStorage::Inline::Yes -> index <
  /// PropStorage::kValueToSegmentThreshold.
  template <PropStorage::Inline inl = PropStorage::Inline::No>
  static void setNamedSlotValue(
      JSObject *self,
      Runtime *runtime,
      SlotIndex index,
      HermesValue value);
  /// Store a value to the "named value" storage space by the slot described by
  /// \p desc.
  static void setNamedSlotValue(
      JSObject *self,
      Runtime *runtime,
      NamedPropertyDescriptor desc,
      HermesValue value);

  /// Load a value using a named descriptor. Read the value either from
  /// named storage or indexed storage depending on the presence of the
  /// "Indexed" flag. Call the getter function if it's defined.
  /// \param selfHandle the object we are loading the property from
  /// \param propObj the object where the property was found (it could be
  ///   anywhere along the prototype chain).
  /// \param desc the property descriptor.
  static CallResult<HermesValue> getNamedPropertyValue(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<JSObject> propObj,
      NamedPropertyDescriptor desc);

  /// Load a value using a computed descriptor. Read the value either from
  /// named storage or indexed storage depending on the presence of the
  /// "Indexed" flag. This does not call the getter, and can be used to
  /// retrieve the accessor directly.
  static HermesValue getComputedSlotValue(
      JSObject *self,
      Runtime *runtime,
      ComputedPropertyDescriptor desc);

  /// Store a value using a computed descriptor. Store the value either to
  /// named storage or indexed storage depending on the presence of the
  /// "Indexed" flag. This does not call the setter, and can be used to
  /// set the accessor directly.  The \p gc parameter is necessary for write
  /// barriers.
  LLVM_NODISCARD static ExecutionStatus setComputedSlotValue(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      ComputedPropertyDescriptor desc,
      Handle<> value);

  /// Load a value using a computed descriptor. Read the value either from
  /// named storage or indexed storage depending on the presence of the
  /// "Indexed" flag. Call the getter function if it's defined.
  /// \param selfHandle the object we are loading the property from
  /// \param propObj the object where the property was found (it could be
  ///   anywhere along the prototype chain).
  /// \param desc the property descriptor.
  static CallResult<HermesValue> getComputedPropertyValue(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<JSObject> propObj,
      ComputedPropertyDescriptor desc);

  /// ES5.1 8.12.1.
  /// Extract a descriptor \p desc of an own named property \p name.
  static bool getOwnNamedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// ES5.1 8.12.1.
  /// An opportunistic fast path of \c getOwnNamedDescriptor(). If certain
  /// implementation-dependent conditions are met, it can look up a property
  /// quickly and succeed. If it fails, the "slow path" - \c
  /// getOwnNamedDescriptor() must be used.
  static bool tryGetOwnNamedDescriptorFast(
      JSObject *self,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// ES5.1 8.12.1.
  /// \param nameValHandle the name of the property. It must be a primitive.
  static CallResult<bool> getOwnComputedPrimitiveDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      ComputedPropertyDescriptor &desc);

  /// A wrapper to getOwnComputedPrimitiveDescriptor() in the case when
  /// \p nameValHandle may be an object.
  /// We will need to call toString() on the object first before we invoke
  /// getOwnComputedPrimitiveDescriptor(), to ensure the side-effect only
  /// happens once.
  static CallResult<bool> getOwnComputedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      ComputedPropertyDescriptor &desc);

  /// ES5.1 8.12.2.
  /// Extract a descriptor \p desc of a named property \p name in this object
  /// or along the prototype chain.
  /// \param expectedFlags if valid, we are searching for a property which, if
  ///   not found, we would create with these specific flags. This can speed
  ///   up the search in the negative case - when the property doesn't exist.
  /// \return the object instance containing the property, or nullptr.
  static JSObject *getNamedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropertyFlags expectedFlags,
      NamedPropertyDescriptor &desc);

  /// ES5.1 8.12.2.
  /// Wrapper around \c getNamedDescriptor() passing \c false to \c
  /// forPutNamed.
  static JSObject *getNamedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// ES5.1 8.12.2.
  /// Extract a descriptor \p desc of a named property \p name in this object
  /// or along the prototype chain.
  /// \param nameValHandle the name of the property. It must be a primitive.
  /// \param[out] propObj it is set to the object in the prototype chain
  ///   containing the property, or \c null if we didn't find the property.
  /// \param[out] desc if the property was found, set to the property
  ///   descriptor.
  static ExecutionStatus getComputedPrimitiveDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      MutableHandle<JSObject> &propObj,
      ComputedPropertyDescriptor &desc);

  /// A wrapper to getComputedPrimitiveDescriptor() in the case when
  /// \p nameValHandle may be an object, in which case we need to call
  /// \c toString() before we  invoke getComputedPrimitiveDescriptor(), to
  /// ensure the side-effect only happens once.
  /// The values of the output parameters are not defined if the call terminates
  /// with an exception.
  /// \param nameValHandle the name of the property.
  /// \param[out] propObj if the method terminates without an exception, it is
  ///    set to the object in the prototype chain containing the property, or
  ///    \c null if we didn't find the property.
  /// \param[out] desc if the property was found, set to the property
  ///   descriptor.
  static ExecutionStatus getComputedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      MutableHandle<JSObject> &propObj,
      ComputedPropertyDescriptor &desc);

  /// The following three methods implement ES5.1 8.12.3.
  /// getNamed is an optimized path for getting a property with a SymbolID when
  /// it is statically known that the SymbolID is not index-like.
  static CallResult<HermesValue> getNamed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropOpFlags opFlags = PropOpFlags());

  // getNamedOrIndexed accesses a property with a SymbolIDs which may be
  // index-like.
  static CallResult<HermesValue> getNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropOpFlags opFlags = PropOpFlags());

  /// getComputed accesses a property with an arbitrary object key, implementing
  /// ES5.1 8.12.3 in full generality.
  static CallResult<HermesValue> getComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle);

  /// The following three methods implement ES5.1 8.12.6
  /// hasNamed is an optimized path for checking existence of a property
  /// for SymbolID when it is statically known that the SymbolIDs is not
  /// index-like.
  static bool
  hasNamed(Handle<JSObject> selfHandle, Runtime *runtime, SymbolID name);

  /// hasNamedOrIndexed checks existence of a property for a SymbolID which may
  /// be index-like.
  static bool hasNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name);

  /// hasComputed checks existence of a property for arbitrary object key
  static CallResult<bool> hasComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle);

  /// The following three methods implement ES5.1 8.12.5.
  /// putNamed is an optimized path for setting a property with a SymbolID when
  /// it is statically known that the SymbolID is not index-like.
  static CallResult<bool> putNamed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// putNamedOrIndexed sets a property with a SymbolID which may be index-like.
  static CallResult<bool> putNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// putComputed sets a property with an arbitrary object key.
  static CallResult<bool> putComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// ES5.1 8.12.7.
  static CallResult<bool> deleteNamed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropOpFlags opFlags = PropOpFlags());
  /// ES5.1 8.12.7.
  static CallResult<bool> deleteComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// Obtain an element from the "indexed storage" of this object. The storage
  /// itself is implementation dependent.
  /// \return the value of the element or "empty" if there is no such element.
  static HermesValue
  getOwnIndexed(JSObject *self, Runtime *runtime, uint32_t index) {
    return self->getVT()->getOwnIndexed(self, runtime, index);
  }

  /// Set an element in the "indexed storage" of this object. Depending on the
  /// semantics of the "indexed storage" the storage capacity may need to be
  /// expanded (e.g. affecting Array.length), or the write may simply be ignored
  /// (in the case of typed arrays).
  /// \return true if the write succeeded, or false if it was ignored.
  static CallResult<bool> setOwnIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value) {
    return selfHandle->getVT()->setOwnIndexed(
        selfHandle, runtime, index, value);
  }

  /// Delete an element in the "indexed storage". It does affect the array's
  /// length.
  /// \return 'true' if the element was successfully deleted, or if it was
  ///     outside of the storage range. 'false' if this storage doesn't support
  ///     "holes"/deletion (e.g. typed arrays).
  static bool deleteOwnIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index) {
    return selfHandle->getVT()->deleteOwnIndexed(selfHandle, runtime, index);
  }

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  static bool checkAllOwnIndexed(
      JSObject *self,
      ObjectVTable::CheckAllOwnIndexedMode mode) {
    return self->getVT()->checkAllOwnIndexed(self, mode);
  }

  /// Define a new property or update an existing one following the rules
  /// described in ES5.1 8.12.9.
  /// \param dpFlags flags which in conjuction with the rules of ES5.1 8.12.9
  ///   describing how the property flags of an existing property should be
  ///   updated or the flags of a new property should be initialized.
  /// \param valueOrAccessor the value of the new property. If the property is
  ///   an accessor, it should be an instance of \c PropertyAccessor.
  /// \param opFlags flags modifying the behavior in case of error.
  /// \return \c true on success. In case of failure it returns an exception
  ///   or false, depending on the value of \c opFlags.ThrowOnError.
  /// Note: This can throw even if ThrowOnError is false,
  /// because ThrowOnError is only for specific kinds of errors,
  /// and this function will not swallow other kinds of errors.
  static CallResult<bool> defineOwnProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags = PropOpFlags());

  /// Define a new property, which must not already exist in this object.
  /// This is similar in intent to ES5.1 \c defineOwnProperty(), but is simpler
  /// and faster since it doesn't support updating of properties. It doesn't
  /// need to search for an existing property and it doesn't need the
  /// complicated set of rules in ES5.1 8.12.9 describing how to synthesize or
  /// update \c PropertyFlags based on instructions in \c DefinedPropertyFlags.
  ///
  /// It is frequently possible to use this method when defining properties of
  /// an object that the caller created since in that case the caller has full
  /// control over the properties in the object (and the prototype chain
  /// doesn't matter).
  ///
  /// \param propertyFlags the actual, final, value of \c PropertyFlags that
  ///   will be stored in the property descriptor.
  /// \param valueOrAccessor the value of the new property.
  LLVM_NODISCARD static ExecutionStatus defineNewOwnProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropertyFlags propertyFlags,
      Handle<> valueOrAccessor);

  /// ES5.1 8.12.9.
  /// \param nameValHandle the name of the property. It must be a primitive.
  static CallResult<bool> defineOwnComputedPrimitive(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags = PropOpFlags());

  /// ES5.1 8.12.9.
  /// A wrapper to \c defineOwnComputedPrimitive() in case \p nameValHandle is
  /// an object.
  /// We will need to call toString() on the object first before we invoke
  /// \c defineOwnComputedPrimitive(), to ensure the side-effect only happens
  /// once.
  static CallResult<bool> defineOwnComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags = PropOpFlags());

  /// ES5.1 15.2.3.8.
  /// Make all own properties non-configurable.
  /// Set [[Extensible]] to false.
  static void seal(Handle<JSObject> selfHandle, Runtime *runtime);
  /// ES5.1 15.2.3.9.
  /// Make all own properties non-configurable.
  /// Make all own data properties (not accessors) non-writable.
  /// Set [[Extensible]] to false.
  static void freeze(Handle<JSObject> selfHandle, Runtime *runtime);
  /// ES5.1 15.2.3.10.
  /// Set [[Extensible]] to false, preventing adding more properties.
  static void preventExtensions(JSObject *self);

  /// ES5.1 15.2.3.11.
  /// No properties are configurable.
  /// [[Extensible]] is false.
  static bool isSealed(PseudoHandle<JSObject> self, Runtime *runtime);
  /// ES5.1 15.2.3.12.
  /// No properties are configurable.
  /// No data properties (not accessors) are writable.
  /// [[Extensible]] is false.
  static bool isFrozen(PseudoHandle<JSObject> self, Runtime *runtime);

  /// Update the property flags in the list \p props on \p selfHandle,
  /// with provided \p flagsToClear and \p flagsToSet, and if it is not
  /// provided, update all properties.
  /// This method is efficient in updating multiple properties than updating
  /// them one by one because it creates at most one hidden class and mutates
  /// that hidden class without creating new transitions under the hood.
  /// \p flagsToClear and \p flagsToSet are masks for updating the property
  /// flags.
  /// \p props is a list of SymbolIDs for properties that need to be
  /// updated. It should contain a subset of properties in the object, so
  /// the SymbolIDs won't get freed by gc. It is optional; if it is llvm::None,
  /// update every property.
  static void updatePropertyFlagsWithoutTransitions(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      PropertyFlags flagsToClear,
      PropertyFlags flagsToSet,
      OptValue<llvm::ArrayRef<SymbolID>> props);

 protected:
  /// @name Virtual function implementations
  /// @{

  /// \return the range of indexes (end-exclusive) stored in indexed storage.
  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(JSObject *self);

  /// Check whether property with index \p index exists in indexed storage and
  /// \return true if it does.
  static bool
  _haveOwnIndexedImpl(JSObject *self, Runtime *runtime, uint32_t index);

  /// Check whether property with index \p index exists in indexed storage and
  /// extract its \c PropertyFlags (if necessary checking whether the object is
  /// frozen or sealed).
  /// \return PropertyFlags if the property exists.
  static OptValue<PropertyFlags> _getOwnIndexedPropertyFlagsImpl(
      JSObject *self,
      Runtime *runtime,
      uint32_t index);

  /// Obtain an element from the "indexed storage" of this object. The storage
  /// itself is implementation dependent.
  /// \return the value of the element or "empty" if there is no such element.
  static HermesValue
  _getOwnIndexedImpl(JSObject *self, Runtime *runtime, uint32_t index);

  /// Set an element in the "indexed storage" of this object. Depending on the
  /// semantics of the "indexed storage" the storage capacity may need to be
  /// expanded (e.g. affecting Array.length), or the write may simply be ignored
  /// (in the case of typed arrays).
  /// \return true if the write succeeded, or false if it was ignored.
  static CallResult<bool> _setOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value);

  /// Delete an element in the "indexed storage".
  /// \return 'true' if the element was successfully deleted, or if it was
  ///     outside of the storage range. 'false' if this storage doesn't support
  ///     "holes"/deletion (e.g. typed arrays).
  static bool _deleteOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index);

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  static bool _checkAllOwnIndexedImpl(
      JSObject *self,
      ObjectVTable::CheckAllOwnIndexedMode mode);

  /// @}

 private:
  // Internal API

  /// "Wrap" PropStorage by casting it to an opaque type.
  static JSObjectPropStorage *wrapPropStorage(PropStorage *ps) {
    return (JSObjectPropStorage *)ps;
  }

  /// Convenience method to "unwrap" PropStorage from JSObjectPropStorage.
  static PropStorage *unwrapPropStorage(JSObjectPropStorage *ps) {
    return vmcast_or_null<PropStorage>((GCCell *)ps);
  }

  const ObjectVTable *getVT() const {
    return reinterpret_cast<const ObjectVTable *>(GCCell::getVT());
  }

  /// Allocate storage for a new slot after the slot index itself has been
  /// allocated by the hidden class.
  /// Note that slot storage is never truly released once allocated. Released
  /// storage slots are put into a free list.
  static void allocateNewSlotStorage(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SlotIndex newSlotIndex,
      Handle<> valueHandle);

  /// Look for a property and return a \c PropertyPos identifying it and store
  /// its descriptor in \p desc.
  /// \param expectedFlags if valid, we are searching for a property which, if
  ///   not found, we would create with these specific flags. This can speed
  ///   up the search in the negative case - when the property doesn't exist.
  static OptValue<HiddenClass::PropertyPos> findProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropertyFlags expectedFlags,
      NamedPropertyDescriptor &desc);

  /// Look for a property and return a \c PropertyPos identifying it and store
  /// its descriptor in \p desc.
  static OptValue<HiddenClass::PropertyPos> findProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// ES5.1 8.12.9.
  static CallResult<bool> addOwnProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags);
  /// Performs the actual adding of the property for \c addOwnProperty()
  static ExecutionStatus addOwnPropertyImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropertyFlags propertyFlags,
      Handle<> valueOrAccessor);

  /// ES5.1 8.12.9.
  static CallResult<bool> updateOwnProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      HiddenClass::PropertyPos propertyPos,
      NamedPropertyDescriptor desc,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags);

  /// The result of \c checkPropertyUpdate.
  enum class PropertyUpdateStatus {
    /// The property cannot be updated.
    failed,
    /// The update only required changing the property flags, which was done.
    done,
    /// The update is valid: the property flags were changed but the property
    /// value needs to be set by the caller.
    needSet
  };

  /// Check whether a property can be updated based on the rules in
  /// ES5.1 8.12.9. If the update is valid, return the updated property flags
  /// and a value indicating whether the property value needs to be set as well.
  /// If the update cannot be performed, the call will either raise an exception
  /// or return failure, depending on \c PropOpFlags.throwOnError.
  ///
  /// \param currentFlags the current property flags.
  /// \param curValueOrAccessor the current value of the property.
  /// \return a pair of the updated property flags and a status, where the
  ///   status is one of:
  ///   * \c PropertyUpdateStatus::failed if the update cannot be performed.
  ///   * \c PropertyUpdateStatus::done if the update only required changing the
  //      property flags.
  ///   * \c PropertyUpdateStatus::needSet if the update is valid and the value
  ///     of the property must now be set by the caller.
  static CallResult<std::pair<PropertyUpdateStatus, PropertyFlags>>
  checkPropertyUpdate(
      Runtime *runtime,
      PropertyFlags currentFlags,
      DefinePropertyFlags dpFlags,
      HermesValue curValueOrAccessor,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags);

  /// \return the range of indexes (end-exclusive) stored in indexed storage.
  static std::pair<uint32_t, uint32_t> getOwnIndexedRange(JSObject *self);

  /// Check whether property with index \p index exists in indexed storage and
  /// \return true if it does.
  static bool haveOwnIndexed(JSObject *self, Runtime *runtime, uint32_t index);

  /// Check whether property with index \p index exists in indexed storage and
  /// extract its \c PropertyFlags (if necessary checking whether the object is
  /// frozen or sealed).
  /// \return PropertyFlags if the property exists.
  static OptValue<PropertyFlags>
  getOwnIndexedPropertyFlags(JSObject *self, Runtime *runtime, uint32_t index);

  /// A handler called when a data descriptor has the \c internalSetter flag
  /// set. It is invoked instead of updating the actual property value. The
  /// handler can update the property value by calling \c setNamedSlotValue() if
  /// it didn't manipulate the property storage.
  /// \returns a result logically equivalent to the result of \c putNamed().
  static CallResult<bool> internalSetter(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      NamedPropertyDescriptor desc,
      Handle<> value,
      PropOpFlags opFlags);

 protected:
  /// Flags affecting the entire object.
  ObjectFlags flags_{};

  /// The prototype of this object.
  GCPointer<JSObject> parent_;

  /// The dynamically derived "class" of the object, describing its fields in
  /// order.
  GCPointer<HiddenClass> clazz_{};

  /// Storage for property values.
  GCPointer<PropStorage> propStorage_{};

  /// Storage for direct property slots.
  GCHermesValue directProps_[DIRECT_PROPERTY_SLOTS];
};

/// \return an array that contains all enumerable properties of obj (including
/// those of its prototype etc.) at the indices [beginIndex, endIndex) (any
/// other part of the array is implementation-defined).
/// \param[out] beginIndex beginning of the range of indices storing names
/// \param[out] endIndex end (exclusive) of the range of indices storing names
CallResult<Handle<BigStorage>> getForInPropertyNames(
    Runtime *runtime,
    Handle<JSObject> obj,
    uint32_t &beginIndex,
    uint32_t &endIndex);

/// This object is the value of a property which has a getter and/or setter.
class PropertyAccessor final : public GCCell {
 protected:
  PropertyAccessor(Runtime *runtime, Callable *getter, Callable *setter)
      : GCCell(&runtime->getHeap(), &vt),
        getter(getter, &runtime->getHeap()),
        setter(setter, &runtime->getHeap()) {}

 public:
  static VTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::PropertyAccessorKind;
  }

  GCPointer<Callable> getter{};
  GCPointer<Callable> setter{};

  static CallResult<HermesValue>
  create(Runtime *runtime, Handle<Callable> getter, Handle<Callable> setter);
};

//===----------------------------------------------------------------------===//
// Object inline methods.

inline CallResult<Handle<JSObjectPropStorage>> JSObject::createPropStorage(
    Runtime *runtime,
    PropStorage::size_type capacity) {
  if (capacity <= DIRECT_PROPERTY_SLOTS)
    return runtime->makeNullHandle<JSObjectPropStorage>();

  auto res = PropStorage::create(runtime, capacity - DIRECT_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle<JSObjectPropStorage>(*res);
}

inline CallResult<Handle<JSObjectPropStorage>> JSObject::createPropStorage(
    Runtime *runtime,
    PropStorage::size_type capacity,
    PropStorage::size_type size) {
  if (capacity <= DIRECT_PROPERTY_SLOTS)
    return runtime->makeNullHandle<JSObjectPropStorage>();

  auto res = PropStorage::create(
      runtime, capacity - DIRECT_PROPERTY_SLOTS, size - DIRECT_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle<JSObjectPropStorage>(*res);
}

inline HermesValue JSObject::getInternalProperty(
    JSObject *self,
    SlotIndex index) {
  return getNamedSlotValue(self, NamedPropertyDescriptor{{}, index});
}

inline void JSObject::setInternalProperty(
    JSObject *self,
    Runtime *runtime,
    SlotIndex index,
    HermesValue value) {
  return setNamedSlotValue(
      self, runtime, NamedPropertyDescriptor{{}, index}, value);
}

template <PropStorage::Inline inl>
inline HermesValue JSObject::getNamedSlotValue(
    JSObject *self,
    SlotIndex index) {
  if (LLVM_LIKELY(index < DIRECT_PROPERTY_SLOTS))
    return self->directProps_[index];

  return self->propStorage_->at<inl>(index - DIRECT_PROPERTY_SLOTS);
}
inline HermesValue JSObject::getNamedSlotValue(
    JSObject *self,
    NamedPropertyDescriptor desc) {
  return getNamedSlotValue(self, desc.slot);
}

template <PropStorage::Inline inl>
inline void JSObject::setNamedSlotValue(
    JSObject *self,
    Runtime *runtime,
    SlotIndex index,
    HermesValue value) {
  if (LLVM_LIKELY(index < DIRECT_PROPERTY_SLOTS))
    return self->directProps_[index].set(value, &runtime->getHeap());

  self->propStorage_->at<inl>(index - DIRECT_PROPERTY_SLOTS)
      .set(value, &runtime->getHeap());
}
inline void JSObject::setNamedSlotValue(
    JSObject *self,
    Runtime *runtime,
    NamedPropertyDescriptor desc,
    HermesValue value) {
  return setNamedSlotValue(self, runtime, desc.slot, value);
}

inline HermesValue JSObject::getComputedSlotValue(
    JSObject *self,
    Runtime *runtime,
    ComputedPropertyDescriptor desc) {
  if (LLVM_LIKELY(desc.flags.indexed)) {
    assert(
        self->flags_.indexedStorage &&
        "indexed flag set but no indexed storage");
    return getOwnIndexed(self, runtime, desc.slot);
  }
  return getNamedSlotValue(self, desc.castToNamedPropertyDescriptorRef());
}

inline ExecutionStatus JSObject::setComputedSlotValue(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    ComputedPropertyDescriptor desc,
    Handle<> value) {
  if (LLVM_LIKELY(desc.flags.indexed)) {
    assert(
        selfHandle->flags_.indexedStorage &&
        "indexed flag set but no indexed storage");
    return setOwnIndexed(selfHandle, runtime, desc.slot, value).getStatus();
  }
  setNamedSlotValue(
      selfHandle.get(),
      runtime,
      desc.castToNamedPropertyDescriptorRef(),
      value.get());
  return ExecutionStatus::RETURNED;
}

inline bool JSObject::getOwnNamedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return findProperty(selfHandle, runtime, name, desc).hasValue();
}

inline bool JSObject::tryGetOwnNamedDescriptorFast(
    JSObject *self,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return HiddenClass::tryFindPropertyFast(self->clazz_, name, desc);
}

inline JSObject *JSObject::getNamedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return getNamedDescriptor(
      selfHandle, runtime, name, PropertyFlags::invalid(), desc);
}

inline std::pair<uint32_t, uint32_t> JSObject::getOwnIndexedRange(
    JSObject *self) {
  return self->getVT()->getOwnIndexedRange(self);
};

inline bool
JSObject::haveOwnIndexed(JSObject *self, Runtime *runtime, uint32_t index) {
  return self->getVT()->haveOwnIndexed(self, runtime, index);
}

inline OptValue<PropertyFlags> JSObject::getOwnIndexedPropertyFlags(
    JSObject *self,
    Runtime *runtime,
    uint32_t index) {
  return self->getVT()->getOwnIndexedPropertyFlags(self, runtime, index);
}

inline OptValue<HiddenClass::PropertyPos> JSObject::findProperty(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return findProperty(
      selfHandle, runtime, name, PropertyFlags::invalid(), desc);
}

inline OptValue<HiddenClass::PropertyPos> JSObject::findProperty(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    PropertyFlags expectedFlags,
    NamedPropertyDescriptor &desc) {
  return HiddenClass::findProperty(
      selfHandle->clazz_, runtime, name, expectedFlags, desc);
}

inline bool JSObject::shouldCacheForIn() const {
  return !clazz_->isDictionary() && !flags_.indexedStorage &&
      !flags_.hostObject;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSOBJECT_H
