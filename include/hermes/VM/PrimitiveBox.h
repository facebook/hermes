/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PRIMITIVEBOX_H
#define HERMES_VM_PRIMITIVEBOX_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// A container object for primitive HermesValues.
class PrimitiveBox : public JSObject {
  using Super = JSObject;

 protected:
  PrimitiveBox(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz)
      : JSObject(runtime, vt, parent, clazz) {}

  static constexpr SlotIndex primitiveValuePropIndex() {
    return numOverlapSlots<PrimitiveBox>() + ANONYMOUS_PROPERTY_SLOTS - 1;
  }

 public:
#ifdef HERMESVM_SERIALIZE
  PrimitiveBox(Deserializer &d, const VTable *vt);
#endif

  // We need one slot for the boxed value.
  static const PropStorage::size_type ANONYMOUS_PROPERTY_SLOTS =
      Super::ANONYMOUS_PROPERTY_SLOTS + 1;

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::PrimitiveBoxKind_first,
        CellKind::PrimitiveBoxKind_last);
  }

  /// \return the [[PrimitiveValue]] internal property.
  static HermesValue getPrimitiveValue(JSObject *self) {
    return JSObject::getDirectSlotValue<
        PrimitiveBox::primitiveValuePropIndex()>(self);
  }

  /// Set the [[PrimitiveValue]] internal property.
  static void
  setPrimitiveValue(JSObject *self, Runtime *runtime, SmallHermesValue value) {
    return JSObject::setDirectSlotValue<
        PrimitiveBox::primitiveValuePropIndex()>(
        self, value, &runtime->getHeap());
  }
};

/// String object.
class JSString final : public PrimitiveBox {
 public:
  using Super = PrimitiveBox;

#ifdef HERMESVM_SERIALIZE
  JSString(Deserializer &d, const VTable *vt);
#endif

  // We need one more slot for the length property.
  static const PropStorage::size_type NAMED_PROPERTY_SLOTS =
      Super::NAMED_PROPERTY_SLOTS + 1;
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::StringObjectKind;
  }

  static CallResult<Handle<JSString>> create(
      Runtime *runtime,
      Handle<StringPrimitive> value,
      Handle<JSObject> prototype);

  static CallResult<Handle<JSString>> create(
      Runtime *runtime,
      Handle<JSObject> prototype) {
    return create(
        runtime,
        runtime->getPredefinedStringHandle(Predefined::emptyString),
        prototype);
  }

  /// Set the [[PrimitiveValue]] internal property from a string.
  static void setPrimitiveString(
      Handle<JSString> selfHandle,
      Runtime *runtime,
      Handle<StringPrimitive> string);

  /// Return the [[PrimitiveValue]] internal property as a string.
  static const StringPrimitive *getPrimitiveString(
      JSObject *self,
      Runtime *runtime) {
    return getPrimitiveValue(self).getString();
  }

  JSString(Runtime *runtime, Handle<JSObject> parent, Handle<HiddenClass> clazz)
      : PrimitiveBox(runtime, &vt.base, *parent, *clazz) {
    flags_.indexedStorage = true;
    flags_.fastIndexProperties = true;
  }

 protected:
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

  /// \return the range of indexes (end-exclusive) in the array.
  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(
      JSObject *selfObj,
      Runtime *runtime);

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
};

/// StringIterator object.
/// See ES6 21.1.5.3 for Properties of String Iterator Instances.
class JSStringIterator : public JSObject {
  using Super = JSObject;

  friend void StringIteratorBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::StringIteratorKind;
  }

  static PseudoHandle<JSStringIterator> create(
      Runtime *runtime,
      Handle<StringPrimitive> string);

  /// Iterate to the next element and return.
  static CallResult<HermesValue> nextElement(
      Handle<JSStringIterator> self,
      Runtime *runtime);

#ifdef HERMESVM_SERIALIZE
  explicit JSStringIterator(Deserializer &d);

  friend void StringIteratorSerialize(Serializer &s, const GCCell *cell);
  friend void StringIteratorDeserialize(Deserializer &d, CellKind kind);
#endif

  JSStringIterator(
      Runtime *runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<StringPrimitive> iteratedString)
      : JSObject(runtime, &vt.base, *parent, *clazz),
        iteratedString_(runtime, *iteratedString, &runtime->getHeap()) {}

 private:
  /// [[IteratedString]]
  /// This is null if iteration has been completed.
  GCPointer<StringPrimitive> iteratedString_;

  /// [[StringIteratorNextIndex]]
  uint32_t nextIndex_{0};
};

/// Number object.
class JSNumber final : public PrimitiveBox {
 public:
  static const ObjectVTable vt;

#ifdef HERMESVM_SERIALIZE
  JSNumber(Deserializer &d, const VTable *vt);
#endif

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::NumberObjectKind;
  }

  static Handle<JSNumber>
  create(Runtime *runtime, double value, Handle<JSObject> prototype);

  static Handle<JSNumber> create(Runtime *runtime, Handle<JSObject> prototype) {
    return create(runtime, 0.0, prototype);
  }

  JSNumber(Runtime *runtime, Handle<JSObject> parent, Handle<HiddenClass> clazz)
      : PrimitiveBox(runtime, &vt.base, *parent, *clazz) {}
};

/// Boolean object.
class JSBoolean final : public PrimitiveBox {
 public:
  static const ObjectVTable vt;

#ifdef HERMESVM_SERIALIZE
  JSBoolean(Deserializer &d, const VTable *vt);
#endif

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BooleanObjectKind;
  }

  static PseudoHandle<JSBoolean>
  create(Runtime *runtime, bool value, Handle<JSObject> prototype);

  static PseudoHandle<JSBoolean> create(
      Runtime *runtime,
      Handle<JSObject> prototype) {
    return create(runtime, false, prototype);
  }

  JSBoolean(
      Runtime *runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : PrimitiveBox(runtime, &vt.base, *parent, *clazz) {}
};

/// Symbol object.
class JSSymbol final : public PrimitiveBox {
 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::SymbolObjectKind;
  }

  static PseudoHandle<JSSymbol>
  create(Runtime *runtime, SymbolID value, Handle<JSObject> prototype);

  static PseudoHandle<JSSymbol> create(
      Runtime *runtime,
      Handle<JSObject> prototype) {
    return create(runtime, SymbolID{}, prototype);
  }

  /// Return the [[PrimitiveValue]] internal property as a string.
  static const PseudoHandle<SymbolID> getPrimitiveSymbol(JSObject *self) {
    return PseudoHandle<SymbolID>::create(
        HermesValueTraits<SymbolID>::decode(getPrimitiveValue(self)));
  }

#ifdef HERMESVM_SERIALIZE
  explicit JSSymbol(Deserializer &d);

  friend void SymbolObjectDeserialize(Deserializer &d, CellKind kind);
#endif

  JSSymbol(Runtime *runtime, Handle<JSObject> parent, Handle<HiddenClass> clazz)
      : PrimitiveBox(runtime, &vt.base, *parent, *clazz) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PRIMITIVEBOX_H
