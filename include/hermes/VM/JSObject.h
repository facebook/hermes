/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSOBJECT_H
#define HERMES_VM_JSOBJECT_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PropertyDescriptor.h"
#include "hermes/VM/StringView.h"
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

  /// This flag indicates this is a proxy exotic Object
  uint32_t proxyObject : 1;

  static constexpr unsigned kHashWidth = 24;
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

/// \name OwnKeysFlags
/// @{
/// Flags used when performing getOwnPropertyKeys operations.
///
/// \name IncludeSymbols
/// Include in the result keys which are formally (and in the implementation)
/// Symbols.
///
/// \name IncludeNonSymbols
/// Include in the result keys which are formally Strings.  In the
/// implementation, these may actually be numbers or other non-String primitive
/// types.
///
/// \name IncludeNonEnumerable
/// Normally only enumerable keys are included in the result.  If this is set,
/// include non-enumerable keys, too.  The keys included will only be of the
/// types specified by the above flags.
///
/// Either or both of IncludeSymbols and IncludeNonSymbols may be
/// specified.  If neither is specified, this may cause an assertion
/// failure if assertions are enabled.
/// @}
#define HERMES_VM__LIST_OwnKeysFlags(FLAG) \
  FLAG(IncludeSymbols)                     \
  FLAG(IncludeNonSymbols)                  \
  FLAG(IncludeNonEnumerable)

HERMES_VM__DECLARE_FLAGS_CLASS(OwnKeysFlags, HERMES_VM__LIST_OwnKeysFlags);

// Any method that could potentially invoke the garbage collector, directly or
// in-directly, cannot use a direct 'self' pointer and must instead use
// Handle<JSObject>.

struct ObjectVTable {
  VTable base;

  /// \return the range of indexes (end-exclusive) stored in indexed storage.
  std::pair<uint32_t, uint32_t> (
      *getOwnIndexedRange)(JSObject *self, Runtime *runtime);

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
  bool (*checkAllOwnIndexed)(
      JSObject *self,
      Runtime *runtime,
      CheckAllOwnIndexedMode mode);
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
      NeedsBarriers needsBarriers)
      : GCCell(&runtime->getHeap(), vtp),
        parent_(runtime, parent, &runtime->getHeap(), needsBarriers),
        clazz_(runtime, clazz, &runtime->getHeap(), needsBarriers),
        propStorage_(runtime, nullptr, &runtime->getHeap(), needsBarriers) {
    // Direct property slots are initialized by initDirectPropStorage.
  }

  /// Until we apply the NeedsBarriers pattern to all subtypes of JSObject, we
  /// will need versions that do not take the extra NeedsBarrier argument
  /// (defaulting to NoBarriers).
  JSObject(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz)
      : JSObject(runtime, vtp, parent, clazz, GCPointerBase::NoBarriers()) {
    // Direct property slots are initialized by initDirectPropStorage.
  }

 public:
  // This exists so some inlined anonymous namespace methods in the
  // impl .cpp file can have private access to JSObject.  The natural
  // approach of making those methods private inline methods of
  // JSObject results in them not actually being inlined.  Measurement
  // has justified doing this.
  struct Helper;

#ifdef HERMESVM_SERIALIZE
  /// A constructor used by deserializeion which performs no GC allocation.
  JSObject(Deserializer &d, const VTable *vtp);

  static void
  serializeObjectImpl(Serializer &s, const GCCell *cell, unsigned overlapSlots);
#endif

  static ObjectVTable vt;

  /// Default capacity of indirect property storage.
  static const PropStorage::size_type DEFAULT_PROPERTY_CAPACITY = 4;

  /// Number of property slots used by the implementation that are unnamed,
  /// meaning they are invisible to user code. Child classes should override
  /// this value by adding to it and defining a constant with the same name.
  static const PropStorage::size_type ANONYMOUS_PROPERTY_SLOTS = 0;

  /// Number of property slots used by the implementation that are named,
  /// meaning they are also visible to user code. Child classes should override
  /// this value by adding to it and defining a constant with the same name.
  static const PropStorage::size_type NAMED_PROPERTY_SLOTS = 0;

  /// Number of property slots allocated directly inside the object.
  static const PropStorage::size_type DIRECT_PROPERTY_SLOTS = 4;

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

  ~JSObject() = default;

  /// Must be called immediately after the construction of any JSObject.
  /// Asserts that the direct property storage is large enough to hold
  /// all internal properties, and initializes all non-overlapping
  /// direct property slots.
  /// \return a copy of self for convenience.
  template <typename T>
  static inline T *initDirectPropStorage(Runtime *runtime, T *self);

  /// ES9 9.1 O.[[Extensible]] internal slot
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

  /// \return true if this is a proxy exotic object.
  bool isProxyObject() const {
    return flags_.proxyObject;
  }

  /// \return the `__proto__` internal property, which may be nullptr.
  JSObject *getParent(Runtime *runtime) const {
    assert(
        !flags_.proxyObject && "getParent cannot be used with proxy objects");
    return parent_.get(runtime);
  }

  /// \return the hidden class of this object.
  HiddenClass *getClass(PointerBase *base) const {
    return clazz_.getNonNull(base);
  }

  /// \return the hidden class of this object.
  const GCPointer<HiddenClass> &getClassGCPtr() const {
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
  inline bool shouldCacheForIn(Runtime *runtime) const;

  /// Sets the internal prototype property. This corresponds to ES9 9.1.2.1
  /// OrdinarySetPrototypeOf.
  /// - Does nothing if the value doesn't change.
  /// - Fails if the object isn't extensible
  /// - Fails if it detects a prototype cycle.
  /// If opFlags.getThrowOnError() is true, then this will throw an appropriate
  /// TypeError for the above failures.  If false, then it will just return
  /// false. If \c self is a Proxy with a trap, and the trap throws an
  /// exception, that exception will be propagated regardless.
  static CallResult<bool> setParent(
      JSObject *self,
      Runtime *runtime,
      JSObject *parent,
      PropOpFlags opFlags = PropOpFlags());

  /// Return a reference to an internal property slot.
  static GCHermesValue &
  internalPropertyRef(JSObject *self, PointerBase *base, SlotIndex index) {
    assert(
        HiddenClass::debugIsPropertyDefined(
            self->clazz_.get(base),
            base,
            InternalProperty::getSymbolID(index)) &&
        "internal slot must be reserved");
    return namedSlotRef<PropStorage::Inline::Yes>(self, base, index);
  }

  static HermesValue
  getInternalProperty(JSObject *self, PointerBase *base, SlotIndex index) {
    return internalPropertyRef(self, base, index);
  }

  static void setInternalProperty(
      JSObject *self,
      Runtime *runtime,
      SlotIndex index,
      HermesValue value) {
    assert(
        HiddenClass::debugIsPropertyDefined(
            self->clazz_.get(runtime),
            runtime,
            InternalProperty::getSymbolID(index)) &&
        "internal slot must be reserved");
    return setNamedSlotValue<PropStorage::Inline::Yes>(
        self, runtime, index, value);
  }

  /// This is the proxy-aware version of getParent.  It has to
  /// allocate handles, so it's neither as simple or efficient as
  /// getParent, but it's needed.  If selfHandle has no parent, this
  /// will return a null JSObject (not a null value).
  static CallResult<PseudoHandle<JSObject>> getPrototypeOf(
      PseudoHandle<JSObject> selfHandle,
      Runtime *runtime);

  /// By default, returns a list of enumerable property names and symbols
  /// belonging to this object. Indexed property names will be represented as
  /// numbers for efficiency. The order of properties follows ES2015 - first
  /// properties whose string names look like indexes, in numeric order, then
  /// strings, in insertion order, then symbols, in insertion order.  \p
  /// okFlags can be used to exclude names and/or symbols, or include
  /// non-enumerable properties.
  /// \returns a JSArray containing the names.
  static CallResult<Handle<JSArray>> getOwnPropertyKeys(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      OwnKeysFlags okFlags);

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
      bool onlyEnumerable) {
    return getOwnPropertyKeys(
        selfHandle,
        runtime,
        OwnKeysFlags().plusIncludeNonSymbols().setIncludeNonEnumerable(
            !onlyEnumerable));
  }

  /// Return a list of property symbols keys belonging to this object.
  /// The order of properties follows ES2015 - insertion order.
  /// \returns a JSArray containing the symbols.
  static CallResult<Handle<JSArray>> getOwnPropertySymbols(
      Handle<JSObject> selfHandle,
      Runtime *runtime) {
    return getOwnPropertyKeys(
        selfHandle,
        runtime,
        OwnKeysFlags().plusIncludeSymbols().plusIncludeNonEnumerable());
  }

  /// Return a reference to a slot in the "named value" storage space by
  /// \p index.
  /// \pre inl == PropStorage::Inline::Yes -> index <
  ///   PropStorage::kValueToSegmentThreshold.
  template <PropStorage::Inline inl = PropStorage::Inline::No>
  static GCHermesValue &
  namedSlotRef(JSObject *self, PointerBase *runtime, SlotIndex index);

  /// Load a value from the "named value" storage space by \p index.
  /// \pre inl == PropStorage::Inline::Yes -> index <
  /// PropStorage::kValueToSegmentThreshold.
  template <PropStorage::Inline inl = PropStorage::Inline::No>
  static HermesValue
  getNamedSlotValue(JSObject *self, PointerBase *runtime, SlotIndex index) {
    return namedSlotRef<inl>(self, runtime, index);
  }

  /// Load a value from the "named value" storage space by the slot described by
  /// the property descriptor \p desc.
  static HermesValue getNamedSlotValue(
      JSObject *self,
      PointerBase *runtime,
      NamedPropertyDescriptor desc) {
    assert(!self->flags_.proxyObject && "getNamedSlotValue called on a Proxy");
    return getNamedSlotValue(self, runtime, desc.slot);
  }

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
      HermesValue value) {
    assert(!self->flags_.proxyObject && "setNamedSlotValue called on a Proxy");
    setNamedSlotValue(self, runtime, desc.slot, value);
  }

  /// Load a value using a named descriptor. Read the value either from
  /// named storage or indexed storage depending on the presence of the
  /// "Indexed" flag. Call the getter function if it's defined.
  /// \param selfHandle the object we are loading the property from
  /// \param propObj the object where the property was found (it could be
  ///   anywhere along the prototype chain).
  /// \param desc the property descriptor.
  static CallResult<PseudoHandle<>> getNamedPropertyValue_RJS(
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
  /// "Indexed" flag. Call the getter function if it's defined. If
  /// selfHandle is a proxy, this asserts.
  /// \param selfHandle the object we are loading the property from
  /// \param propObj the object where the property was found (it could be
  ///   anywhere along the prototype chain).
  /// \param desc the property descriptor.
  static CallResult<PseudoHandle<>> getComputedPropertyValue_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<JSObject> propObj,
      ComputedPropertyDescriptor desc);

  /// This adds some functionality to the other overload.  If propObj
  /// normal object, this behaves just like the other overload.  This
  /// is safe to use with proxies: if the desc has the proxyObject
  /// flag set, then \c nameValHandle is used to call the proxy has
  /// trap.  If the has trap returns false, then this returns an empty
  /// HermesValue, otherwise, the get trap is called (also using \c
  /// nameValHandle) and its result is returned.
  static CallResult<PseudoHandle<>> getComputedPropertyValue_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<JSObject> propObj,
      ComputedPropertyDescriptor desc,
      Handle<> nameValHandle);

  /// ES5.1 8.12.1.
  /// Extract a descriptor \p desc of an own named property \p name.
  /// This will return false if the object is a proxy.
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
  /// \return true or false if a definitive answer can be provided, llvh::None
  /// if the result is unknown.
  static OptValue<bool> tryGetOwnNamedDescriptorFast(
      JSObject *self,
      Runtime *runtime,
      SymbolID name,
      NamedPropertyDescriptor &desc);

  /// Tries to get a property without doing any allocation, while searching the
  /// prototype chain.
  /// If the property cannot be found on this object or any of its prototypes,
  /// or if this object's HiddenClass has an uninitialized property map, returns
  /// \p llvh::None.
  static OptValue<HermesValue>
  tryGetNamedNoAlloc(JSObject *self, PointerBase *base, SymbolID name);

  /// Parameter to getOwnComputedPrimitiveDescriptor
  enum class IgnoreProxy { No, Yes };

  /// ES5.1 8.12.1.
  /// \param nameValHandle the name of the property. It must be a primitive.
  /// If selfHandle refers to a proxy and \p ignoreProxy is Yes, desc
  /// will be untouched and false will be returned.  If selfHandle refers to a
  /// proxy, and \p ignoreProxy is No, then if [[GetOwnProperty]] on the
  /// proxy is undefined, then false will be returned, otherwise, desc will be
  /// filled in with the result, and true will be returned.  If selfHandle is
  /// not a proxy, then the flag is irrelevant.
  static CallResult<bool> getOwnComputedPrimitiveDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      IgnoreProxy ignoreProxy,
      ComputedPropertyDescriptor &desc);

  /// Provides the functionality of ES9 [[GetOwnProperty]] on selfHandle.  It
  /// calls getOwnComputedPrimitiveDescriptor() in the case when \p
  /// nameValHandle may be an object.  We will need to call toString() on the
  /// object first before we invoke getOwnComputedPrimitiveDescriptor(), to
  /// ensure the side-effect only happens once.  If selfHandle is a proxy, this
  /// will fill \p desc with the descriptor as specified in ES9 9.5.5, and
  /// return true if the descriptor is defined.
  static CallResult<bool> getOwnComputedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      ComputedPropertyDescriptor &desc);

  /// Like the other overload, except valueOrAccessor will be set to a value or
  /// PropertyAccessor corresponding to \p desc.flags.
  static CallResult<bool> getOwnComputedDescriptor(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      ComputedPropertyDescriptor &desc,
      MutableHandle<> &valueOrAccessor);

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
  /// If \p cacheEntry is not null, and the result is suitable for use in a
  /// property cache, populate the cache.
  static CallResult<PseudoHandle<>> getNamed_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropOpFlags opFlags = PropOpFlags(),
      PropertyCacheEntry *cacheEntry = nullptr);

  /// Like getNamed, but with a \c receiver.  The receiver is
  /// generally only relevant when JavaScript code is executed.  If an
  /// accessor is used, \c receiver is used as the \c this for the
  /// function call.  If a proxy trap is called, \c receiver is passed
  /// to the trap function.  Normally, \c receiver is the same as \c
  /// selfHandle, but it can be different when using \c Reflect.
  static CallResult<PseudoHandle<>> getNamedWithReceiver_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> receiver,
      PropOpFlags opFlags = PropOpFlags(),
      PropertyCacheEntry *cacheEntry = nullptr);

  // getNamedOrIndexed accesses a property with a SymbolIDs which may be
  // index-like.
  static CallResult<PseudoHandle<>> getNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      PropOpFlags opFlags = PropOpFlags());

  /// getComputed accesses a property with an arbitrary object key, implementing
  /// ES5.1 8.12.3 in full generality.
  static CallResult<PseudoHandle<>> getComputed_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle);

  /// getComputed accesses a property with an arbitrary object key and
  /// receiver value.
  static CallResult<PseudoHandle<>> getComputedWithReceiver_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      Handle<> receiver);

  /// The following three methods implement ES5.1 8.12.6
  /// hasNamed is an optimized path for checking existence of a property
  /// for SymbolID when it is statically known that the SymbolIDs is not
  /// index-like.
  static CallResult<bool>
  hasNamed(Handle<JSObject> selfHandle, Runtime *runtime, SymbolID name);

  /// hasNamedOrIndexed checks existence of a property for a SymbolID which may
  /// be index-like.
  static CallResult<bool> hasNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name);

  /// hasComputed checks existence of a property for arbitrary object key
  static CallResult<bool> hasComputed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle);

  /// The following five methods implement ES5.1 8.12.5.
  /// putNamed is an optimized path for setting a property with a SymbolID when
  /// it is statically known that the SymbolID is not index-like.
  static CallResult<bool> putNamed_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// like putNamed, but with a receiver
  static CallResult<bool> putNamedWithReceiver_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> valueHandle,
      Handle<> receiver,
      PropOpFlags opFlags = PropOpFlags());

  /// putNamedOrIndexed sets a property with a SymbolID which may be index-like.
  static CallResult<bool> putNamedOrIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// putComputed sets a property with an arbitrary object key.
  static CallResult<bool> putComputed_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      Handle<> valueHandle,
      PropOpFlags opFlags = PropOpFlags());

  /// putComputed sets a property with an arbitrary object key and receiver
  /// value
  static CallResult<bool> putComputedWithReceiver_RJS(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      Handle<> nameValHandle,
      Handle<> valueHandle,
      Handle<> receiver,
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

  /// Calls ObjectVTable::getOwnIndexed.
  static HermesValue
  getOwnIndexed(JSObject *self, Runtime *runtime, uint32_t index) {
    return self->getVT()->getOwnIndexed(self, runtime, index);
  }

  /// Calls ObjectVTable::setOwnIndexed.
  static CallResult<bool> setOwnIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value) {
    return selfHandle->getVT()->setOwnIndexed(
        selfHandle, runtime, index, value);
  }

  /// Calls ObjectVTable::deleteOwnIndexed.
  static bool deleteOwnIndexed(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index) {
    return selfHandle->getVT()->deleteOwnIndexed(selfHandle, runtime, index);
  }

  /// Calls ObjectVTable::checkAllOwnIndexed.
  static bool checkAllOwnIndexed(
      JSObject *self,
      Runtime *runtime,
      ObjectVTable::CheckAllOwnIndexedMode mode) {
    return self->getVT()->checkAllOwnIndexed(self, runtime, mode);
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
  /// \pre Cannot call this function with a name that looks like a valid array
  ///   index. Call \c defineOwnComputedPrimitive instead.
  static CallResult<bool> defineOwnProperty(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      SymbolID name,
      DefinePropertyFlags dpFlags,
      Handle<> valueOrAccessor,
      PropOpFlags opFlags = PropOpFlags()) {
#ifdef HERMES_SLOW_DEBUG
    // In slow debug, check if the symbol looks like an array index. If that's
    // the case, it should be using defineOwnComputed instead.
    auto nameView = runtime->getIdentifierTable().getStringView(runtime, name);
    assert(
        !toArrayIndex(nameView) &&
        "Array index property should use defineOwnComputed instead");
#endif
    return defineOwnPropertyInternal(
        selfHandle, runtime, name, dpFlags, valueOrAccessor, opFlags);
  }

  /// Same as \c defineOwnProperty, except the name can be a valid array index.
  /// Prefer \c defineOwnProperty and \c defineOwnComputedPrimitive instead.
  static CallResult<bool> defineOwnPropertyInternal(
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
  /// Don't call this function if you know the \p name is index-like, and the
  /// \p selfHandle has indexedStorage, or it'll lose its fast indexed property.
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
  static ExecutionStatus seal(Handle<JSObject> selfHandle, Runtime *runtime);
  /// ES5.1 15.2.3.9.
  /// Make all own properties non-configurable.
  /// Make all own data properties (not accessors) non-writable.
  /// Set [[Extensible]] to false.
  static ExecutionStatus freeze(Handle<JSObject> selfHandle, Runtime *runtime);
  /// ES5.1 15.2.3.10.
  /// Set [[Extensible]] slot on an ordinary object to false, preventing adding
  /// more properties.
  static void preventExtensions(JSObject *self);
  /// ES9 [[PreventExtensons]] internal method.  This works on Proxy
  /// objects and ordinary objects. If opFlags.getThrowOnError() is
  /// true, then this will throw an appropriate TypeError if the
  /// method would have returned false.
  static CallResult<bool> preventExtensions(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      PropOpFlags opFlags = PropOpFlags());

  /// ES9 9.1.3 [[IsExtensible]] internal method
  /// No properties are can be added.  This also handles the Proxy case.
  static CallResult<bool> isExtensible(
      PseudoHandle<JSObject> self,
      Runtime *runtime);
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
  /// the SymbolIDs won't get freed by gc. It is optional; if it is llvh::None,
  /// update every property.
  static void updatePropertyFlagsWithoutTransitions(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      PropertyFlags flagsToClear,
      PropertyFlags flagsToSet,
      OptValue<llvh::ArrayRef<SymbolID>> props);

  /// First call \p indexedCB, passing each indexed property's \c uint32_t
  /// index and \c ComputedPropertyDescriptor. Then call \p namedCB passing each
  /// named property's \c SymbolID and \c  NamedPropertyDescriptor as
  /// parameters.
  /// The callbacks return true to continue or false to stop immediately.
  ///
  /// Obviously the callbacks shouldn't be doing naughty things like modifying
  /// the property map or creating new hidden classes (even implicitly).
  ///
  /// A marker for the current gcScope is obtained in the beginning and the
  /// scope is flushed after every callback.
  /// \return false if the callback returned false, true otherwise.
  template <typename IndexedCB, typename NamedCB>
  static bool forEachOwnPropertyWhile(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      const IndexedCB &indexedCB,
      const NamedCB &namedCB);

  /// Return the type name of this object, if it can be found heuristically.
  /// There is no one definitive type name for an object. If no heuristic is
  /// able to produce a name, the empty string is returned.
  std::string getHeuristicTypeName(GC *gc);

  /// Accesses the name property on an object, returns the empty string if it
  /// doesn't exist or isn't a string.
  std::string getNameIfExists(PointerBase *base);

 protected:
  /// @name Virtual function implementations
  /// @{

  /// Add an estimate of the type name for this object as the name in heap
  /// snapshots.
  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);

  /// Add user-visible property names to a snapshot.
  static void _snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

  /// Add the location of the constructor for this object to the heap snapshot.
  static void
  _snapshotAddLocationsImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

  /// \return the range of indexes (end-exclusive) stored in indexed storage.
  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(
      JSObject *self,
      Runtime *runtime);

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
      Runtime *runtime,
      ObjectVTable::CheckAllOwnIndexedMode mode);

  /// @}

 private:
  // Internal API

  const ObjectVTable *getVT() const {
    return reinterpret_cast<const ObjectVTable *>(GCCell::getVT());
  }

  /// Allocate an instance of property storage with the specified size.
  static inline ExecutionStatus allocatePropStorage(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      PropStorage::size_type size);

  /// Allocate an instance of property storage with the specified size.
  /// If an allocation is required, a handle is allocated internally and the
  /// updated self value is returned. This means that the return value MUST
  /// be used by the caller.
  static inline CallResult<PseudoHandle<JSObject>> allocatePropStorage(
      PseudoHandle<JSObject> self,
      Runtime *runtime,
      PropStorage::size_type size);

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

  /// Calls ObjectVTable::getOwnIndexedRange.
  static std::pair<uint32_t, uint32_t> getOwnIndexedRange(
      JSObject *self,
      Runtime *runtime);

  /// Calls ObjectVTable::haveOwnIndexed.
  static bool haveOwnIndexed(JSObject *self, Runtime *runtime, uint32_t index);

  /// Calls ObjectVTable::getOwnIndexedPropertyFlags.
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
  inline GCHermesValue *directProps();
  inline const GCHermesValue *directProps() const;

 private:
  /// Byte offset to the first direct property slot in a JSObject.
  static inline constexpr size_t directPropsOffset();

  /// The number of direct property slots that would be unused due to overlap
  /// with C++ fields in a subclass of size \p sizeofDerived, if the number
  /// of direct properties in JSObject were unlimited.
  static constexpr size_t uncappedOverlapSlots(size_t sizeofDerived) {
    return sizeofDerived <= directPropsOffset()
        ? 0
        : (sizeofDerived - directPropsOffset() + sizeof(GCHermesValue) - 1) /
            sizeof(GCHermesValue);
  }

  /// The allocation size needed for a plain JSObject instance (including its
  /// direct property slots).
  static inline constexpr size_t cellSizeJSObject();

 public:
  // Implementation of cellSize. Do not use this directly.
  template <class C>
  static constexpr uint32_t cellSizeImpl() {
    static_assert(
        std::is_convertible<C *, JSObject *>::value, "must be a JSObject");
    return sizeof(C) < cellSizeJSObject() ? cellSizeJSObject() : sizeof(C);
  }

  /// The number of direct property slots that are unused due to overlap with
  /// C++ fields in the class Derived.
  ///
  /// Example layouts ([0-3] = direct props, [A-Z] = other fields):
  ///   JSObject: ABCD0123
  ///   Derived0: ABCDEF23  (2 overlap slots)
  ///   Derived1: ABCDEFGHI (4 overlap slots)
  template <typename Derived>
  static constexpr unsigned numOverlapSlots() {
    static_assert(
        std::is_convertible<Derived *, JSObject *>::value, "must be subclass");
    return uncappedOverlapSlots(sizeof(Derived)) >
            (size_t)JSObject::DIRECT_PROPERTY_SLOTS
        ? (size_t)JSObject::DIRECT_PROPERTY_SLOTS
        : uncappedOverlapSlots(sizeof(Derived));
  }
};

/// Convenience class for accessing the direct property slots of a JSObject.
class JSObjectAndDirectProps : public JSObject {
 public:
  GCHermesValue directProps_[DIRECT_PROPERTY_SLOTS];
};

GCHermesValue *JSObject::directProps() {
  return static_cast<JSObjectAndDirectProps *>(this)->directProps_;
}

const GCHermesValue *JSObject::directProps() const {
  return static_cast<const JSObjectAndDirectProps *>(this)->directProps_;
}

constexpr size_t JSObject::directPropsOffset() {
  return llvh::alignTo<alignof(GCHermesValue)>(sizeof(JSObject));
}

constexpr size_t JSObject::cellSizeJSObject() {
  static_assert(
      sizeof(JSObjectAndDirectProps) ==
          directPropsOffset() + sizeof(GCHermesValue) * DIRECT_PROPERTY_SLOTS,
      "unexpected padding");
  return sizeof(JSObjectAndDirectProps);
}

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
        getter(runtime, getter, &runtime->getHeap()),
        setter(runtime, setter, &runtime->getHeap()) {}

 public:
#ifdef HERMESVM_SERIALIZE
  /// Fast constructor used by deserialization. Don't do any GC allocation. Only
  /// calls super Constructor.
  PropertyAccessor(Deserializer &d);
#endif

  static VTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::PropertyAccessorKind;
  }

  GCPointer<Callable> getter{};
  GCPointer<Callable> setter{};

#if defined(HERMESVM_GC_HADES) && defined(HERMESVM_COMPRESSED_POINTERS)
  // Unused padding just to meet the minimum allocation requirements from Hades.
  int8_t _padding_[4];
#endif

  static CallResult<HermesValue>
  create(Runtime *runtime, Handle<Callable> getter, Handle<Callable> setter);
};

/// Allocation utility for JSObject and its subclasses. Ensures direct property
/// slots are initialized and that no allocation happens during construction.
/// Should be used in a placement new expression, whose result is passed through
/// one of the init* methods:
///
///   JSObjectAlloc<MyObjectType, HasFinalizer::Yes> mem{runtime};
///   return mem.initToHandle(new (mem) MyObjectType(foo(), bar(), baz()));
///
template <typename JSObjectType, HasFinalizer hasFinalizer = HasFinalizer::No>
class JSObjectAlloc {
 public:
  /// Allocate memory that can hold an instance of JSObjectType.
  JSObjectAlloc(Runtime *runtime)
      : runtime_(runtime),
        mem_(runtime->alloc</*fixedSize*/ true, hasFinalizer>(
            cellSize<JSObjectType>())),
        noAlloc_(runtime) {}

  /// Provides access to the allocated memory for a "placement new" expression.
  operator void *() {
    return mem_;
  }

  /// Initialize direct properties of obj and return it in a handle.
  Handle<JSObjectType> initToHandle(JSObjectType *obj) {
    noAlloc_.release();
    return runtime_->makeHandle(init(obj));
  }

  /// Initialize direct properties of obj and return it in a pseudo-handle.
  PseudoHandle<JSObjectType> initToPseudoHandle(JSObjectType *obj) {
    noAlloc_.release();
    return createPseudoHandle(init(obj));
  }

  /// Initialize direct properties of obj and return it as a raw HermesValue.
  HermesValue initToHermesValue(JSObjectType *obj) {
    return HermesValue::encodeObjectValue(init(obj));
  }

  ~JSObjectAlloc() {
    assert(!mem_ && "Must call init* at least once");
  }

 private:
  JSObjectType *init(JSObjectType *obj) {
    assert(obj == mem_ && "Must call init* no more than once");

    // Check that the object looks well-formed.
    assert(JSObjectType::classof(obj) && "Mismatched CellKind");

    mem_ = nullptr;
    return JSObjectType::initDirectPropStorage(runtime_, obj);
  }

  /// Runtime used for heap and handle allocation.
  Runtime *runtime_;

  /// Allocated memory for the object.
  void *mem_;

  /// Asserts no additional allocation happens until the object is constructed.
  NoAllocScope noAlloc_;
};

//===----------------------------------------------------------------------===//
// Object inline methods.

template <typename IndexedCB, typename NamedCB>
bool JSObject::forEachOwnPropertyWhile(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    const IndexedCB &indexedCB,
    const NamedCB &namedCB) {
  auto range = getOwnIndexedRange(*selfHandle, runtime);
  GCScopeMarkerRAII gcMarker{runtime};
  for (auto i = range.first; i != range.second; ++i) {
    auto optPF = getOwnIndexedPropertyFlags(*selfHandle, runtime, i);
    if (!optPF)
      continue;
    ComputedPropertyDescriptor desc{*optPF, i};
    desc.flags.indexed = true;
    if (!indexedCB(runtime, i, desc))
      return false;
    gcMarker.flush();
  }

  return HiddenClass::forEachPropertyWhile(
      runtime->makeHandle(selfHandle->clazz_), runtime, namedCB);
}

inline ExecutionStatus JSObject::allocatePropStorage(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    PropStorage::size_type size) {
  if (LLVM_LIKELY(size <= DIRECT_PROPERTY_SLOTS))
    return ExecutionStatus::RETURNED;

  auto res = PropStorage::create(
      runtime, size - DIRECT_PROPERTY_SLOTS, size - DIRECT_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  selfHandle->propStorage_.set(
      runtime, vmcast<PropStorage>(*res), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

inline CallResult<PseudoHandle<JSObject>> JSObject::allocatePropStorage(
    PseudoHandle<JSObject> self,
    Runtime *runtime,
    PropStorage::size_type size) {
  if (LLVM_LIKELY(size <= DIRECT_PROPERTY_SLOTS))
    return self;

  Handle<JSObject> selfHandle = runtime->makeHandle(std::move(self));
  if (LLVM_UNLIKELY(
          allocatePropStorage(selfHandle, runtime, size) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return PseudoHandle<JSObject>{selfHandle};
}

template <typename T>
inline T *JSObject::initDirectPropStorage(Runtime *runtime, T *self) {
  constexpr auto count = numOverlapSlots<T>() + T::ANONYMOUS_PROPERTY_SLOTS +
      T::NAMED_PROPERTY_SLOTS;
  static_assert(
      count <= DIRECT_PROPERTY_SLOTS,
      "smallPropStorage size must fit in direct properties");
  GCHermesValue::uninitialized_fill(
      self->directProps() + numOverlapSlots<T>(),
      self->directProps() + DIRECT_PROPERTY_SLOTS,
      GCHermesValue(),
      &runtime->getHeap());
  return self;
}

template <PropStorage::Inline inl>
inline GCHermesValue &
JSObject::namedSlotRef(JSObject *self, PointerBase *runtime, SlotIndex index) {
  if (LLVM_LIKELY(index < DIRECT_PROPERTY_SLOTS))
    return self->directProps()[index];

  return self->propStorage_.getNonNull(runtime)->at<inl>(
      index - DIRECT_PROPERTY_SLOTS);
}

template <PropStorage::Inline inl>
inline void JSObject::setNamedSlotValue(
    JSObject *self,
    Runtime *runtime,
    SlotIndex index,
    HermesValue value) {
  // NOTE: even though it is tempting to implement this in terms of assignment
  // to namedSlotRef(), it is a slight performance regression, which is not
  // entirely unexpected.
  if (LLVM_LIKELY(index < DIRECT_PROPERTY_SLOTS))
    return self->directProps()[index].set(value, &runtime->getHeap());

  self->propStorage_.get(runtime)
      ->at<inl>(index - DIRECT_PROPERTY_SLOTS)
      .set(value, &runtime->getHeap());
}

inline HermesValue JSObject::getComputedSlotValue(
    JSObject *self,
    Runtime *runtime,
    ComputedPropertyDescriptor desc) {
  assert(!self->flags_.proxyObject && "getComputedSlotValue called on a Proxy");
  if (LLVM_LIKELY(desc.flags.indexed)) {
    assert(
        self->flags_.indexedStorage &&
        "indexed flag set but no indexed storage");
    return getOwnIndexed(self, runtime, desc.slot);
  }
  return getNamedSlotValue(
      self, runtime, desc.castToNamedPropertyDescriptorRef());
}

inline ExecutionStatus JSObject::setComputedSlotValue(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    ComputedPropertyDescriptor desc,
    Handle<> value) {
  assert(
      !selfHandle->isProxyObject() && "setComputedSlotValue called on a Proxy");
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

inline OptValue<bool> JSObject::tryGetOwnNamedDescriptorFast(
    JSObject *self,
    Runtime *runtime,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return HiddenClass::tryFindPropertyFast(
      self->clazz_.getNonNull(runtime), runtime, name, desc);
}

inline OptValue<HermesValue>
JSObject::tryGetNamedNoAlloc(JSObject *self, PointerBase *base, SymbolID name) {
  for (JSObject *curr = self; curr; curr = curr->parent_.get(base)) {
    auto found =
        HiddenClass::findPropertyNoAlloc(curr->getClass(base), base, name);
    if (found) {
      return getNamedSlotValue(curr, base, found.getValue().slot);
    }
  }
  // It wasn't found on any of the parents of this object, declare it
  // un-findable.
  return llvh::None;
}

inline JSObject *JSObject::getNamedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    NamedPropertyDescriptor &desc) {
  return getNamedDescriptor(
      selfHandle, runtime, name, PropertyFlags::invalid(), desc);
}

inline CallResult<PseudoHandle<>> JSObject::getNamed_RJS(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    PropOpFlags opFlags,
    PropertyCacheEntry *cacheEntry) {
  return getNamedWithReceiver_RJS(
      selfHandle, runtime, name, selfHandle, opFlags, cacheEntry);
}

inline CallResult<PseudoHandle<>> JSObject::getComputed_RJS(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    Handle<> nameValHandle) {
  return getComputedWithReceiver_RJS(
      selfHandle, runtime, nameValHandle, selfHandle);
}

inline CallResult<bool> JSObject::putNamed_RJS(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    SymbolID name,
    Handle<> valueHandle,
    PropOpFlags opFlags) {
  return putNamedWithReceiver_RJS(
      selfHandle, runtime, name, valueHandle, selfHandle, opFlags);
}

inline CallResult<bool> JSObject::putComputed_RJS(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    Handle<> nameValHandle,
    Handle<> valueHandle,
    PropOpFlags opFlags) {
  return putComputedWithReceiver_RJS(
      selfHandle, runtime, nameValHandle, valueHandle, selfHandle, opFlags);
}

inline std::pair<uint32_t, uint32_t> JSObject::getOwnIndexedRange(
    JSObject *self,
    Runtime *runtime) {
  return self->getVT()->getOwnIndexedRange(self, runtime);
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
  auto ret = HiddenClass::findProperty(
      createPseudoHandle(selfHandle->clazz_.getNonNull(runtime)),
      runtime,
      name,
      expectedFlags,
      desc);
  assert(
      !(selfHandle->flags_.proxyObject && ret) &&
      "Proxy objects should never have own properties");
  return ret;
}

inline bool JSObject::shouldCacheForIn(Runtime *runtime) const {
  return !clazz_.get(runtime)->isDictionary() && !flags_.indexedStorage &&
      !flags_.hostObject && !flags_.proxyObject;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSOBJECT_H
