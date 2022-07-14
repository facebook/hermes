/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HERMESVALUETRAITS_H
#define HERMES_VM_HERMESVALUETRAITS_H

#include "hermes/VM/CellKind.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include <cassert>
#include <type_traits>

namespace hermes {
namespace vm {

/// White-list classes than can be managed by a HermesValue.
template <class T>
struct IsGCObject : public std::false_type {};

#define HERMES_VM_GCOBJECT(name) \
  class name;                    \
  template <>                    \
  struct IsGCObject<name> : public std::true_type {}

// White-list objects that can be managed by HermesValue.
HERMES_VM_GCOBJECT(BigIntPrimitive);
HERMES_VM_GCOBJECT(StringPrimitive);
HERMES_VM_GCOBJECT(JSObject);
HERMES_VM_GCOBJECT(Callable);
HERMES_VM_GCOBJECT(BoundFunction);
HERMES_VM_GCOBJECT(NativeFunction);
HERMES_VM_GCOBJECT(FinalizableNativeFunction);
HERMES_VM_GCOBJECT(NativeConstructor);
HERMES_VM_GCOBJECT(JSFunction);
HERMES_VM_GCOBJECT(JSGeneratorFunction);
HERMES_VM_GCOBJECT(JSAsyncFunction);
HERMES_VM_GCOBJECT(GeneratorInnerFunction);
HERMES_VM_GCOBJECT(ArrayImpl);
HERMES_VM_GCOBJECT(Arguments);
HERMES_VM_GCOBJECT(Environment);
HERMES_VM_GCOBJECT(DictPropertyMap);
HERMES_VM_GCOBJECT(HiddenClass);
HERMES_VM_GCOBJECT(PropertyAccessor);
HERMES_VM_GCOBJECT(JSArray);
HERMES_VM_GCOBJECT(JSArrayBuffer);
HERMES_VM_GCOBJECT(JSDataView);
HERMES_VM_GCOBJECT(JSTypedArrayBase);
HERMES_VM_GCOBJECT(JSString);
HERMES_VM_GCOBJECT(JSBigInt);
HERMES_VM_GCOBJECT(JSNumber);
HERMES_VM_GCOBJECT(JSBoolean);
HERMES_VM_GCOBJECT(JSSymbol);
HERMES_VM_GCOBJECT(JSRegExp);
HERMES_VM_GCOBJECT(JSDate);
HERMES_VM_GCOBJECT(JSError);
HERMES_VM_GCOBJECT(JSCallSite);
HERMES_VM_GCOBJECT(JSGenerator);
HERMES_VM_GCOBJECT(Domain);
HERMES_VM_GCOBJECT(RequireContext);
HERMES_VM_GCOBJECT(HashMapEntry);
HERMES_VM_GCOBJECT(OrderedHashMap);
HERMES_VM_GCOBJECT(JSWeakMapImplBase);
HERMES_VM_GCOBJECT(JSWeakRef);
HERMES_VM_GCOBJECT(JSArrayIterator);
HERMES_VM_GCOBJECT(JSStringIterator);
HERMES_VM_GCOBJECT(JSRegExpStringIterator);
HERMES_VM_GCOBJECT(JSProxy);
HERMES_VM_GCOBJECT(JSCallableProxy);
HERMES_VM_GCOBJECT(DecoratedObject);
HERMES_VM_GCOBJECT(HostObject);
HERMES_VM_GCOBJECT(NativeState);

namespace testhelpers {
struct DummyObject;
}
template <>
struct IsGCObject<testhelpers::DummyObject> : public std::true_type {};

// Typed arrays use templates and cannot use the macro above
template <typename T, CellKind C>
class JSTypedArray;
template <typename T, CellKind C>
struct IsGCObject<JSTypedArray<T, C>> : public std::true_type {};

template <CellKind C>
class JSMapImpl;
template <CellKind C>
struct IsGCObject<JSMapImpl<C>> : public std::true_type {};

template <CellKind C>
class JSMapIteratorImpl;
template <CellKind C>
struct IsGCObject<JSMapIteratorImpl<C>> : public std::true_type {};

template <CellKind C>
class JSWeakMapImpl;
template <CellKind C>
struct IsGCObject<JSWeakMapImpl<C>> : public std::true_type {};

template <typename T, bool Uniqued>
class DynamicStringPrimitive;
template <typename T, bool Uniqued>
struct IsGCObject<DynamicStringPrimitive<T, Uniqued>> : public std::true_type {
};
template <typename T>
class ExternalStringPrimitive;
template <typename T>
struct IsGCObject<ExternalStringPrimitive<T>> : public std::true_type {};
template <typename T>
class BufferedStringPrimitive;
template <typename T>
struct IsGCObject<BufferedStringPrimitive<T>> : public std::true_type {};

template <size_t Size>
struct EmptyCell;
template <size_t Size>
struct IsGCObject<EmptyCell<Size>> : public std::true_type {};

template <typename HVType>
class ArrayStorageBase;
template <typename HVType>
struct IsGCObject<ArrayStorageBase<HVType>> : public std::true_type {};

template <typename HVType>
class SegmentedArrayBase;
template <typename HVType>
struct IsGCObject<SegmentedArrayBase<HVType>> : public std::true_type {};

template <typename T, bool isGCObject = IsGCObject<T>::value>
struct HermesValueTraits;

template <>
struct HermesValueTraits<HermesValue> {
  /// The type to be returned by Handle<T>::get().
  using value_type = HermesValue;
  /// The type to be returned by Handle<T>::operator->().
  using arrow_type = const HermesValue *;
  /// Whether this type is a GCCell
  static constexpr bool is_cell = false;

  /// The default initialization value of this type.
  static constexpr value_type defaultValue() {
    return HermesValue::encodeUndefinedValue();
  };
  /// Encode a \c HermesValue into the type.
  static value_type encode(HermesValue value) {
    return value;
  }
  /// Decode a \c HermesValue from the type.
  static HermesValue decode(value_type value) {
    return value;
  }
  static const HermesValue *arrow(const HermesValue &value) {
    return &value;
  }

 private:
  // arrow() must be called with a reference to a non-temporary object, as it
  // returns the address of its parameter. Forbid calls to arrow() with a
  // temporary object through this unimplemented overload.
  static void arrow(HermesValue &&value);
};

template <>
struct HermesValueTraits<SymbolID> {
  /// The type to be returned by Handle<T>::get().
  using value_type = SymbolID;
  /// The type to be returned by Handle<T>::operator->().
  using arrow_type = const SymbolID *;
  /// Whether this type is a GCCell
  static constexpr bool is_cell = false;

  /// The default initialization value of this type.
  static value_type defaultValue() {
    return SymbolID{};
  };
  static HermesValue encode(SymbolID value) {
    return HermesValue::encodeSymbolValue(value);
  }
  static value_type decode(HermesValue value) {
    return value.getSymbol();
  }
};

template <>
struct HermesValueTraits<BigIntPrimitive> {
  using value_type = BigIntPrimitive *;
  using arrow_type = value_type;
  static constexpr bool is_cell = true;

  static constexpr value_type defaultValue() {
    return value_type{};
  }
  static HermesValue encode(value_type value) {
    return HermesValue::encodeBigIntValueUnsafe(value);
  }
  static value_type decode(HermesValue value) {
    return (value_type)value.getBigInt();
  }
  static arrow_type arrow(const HermesValue &value) {
    auto *res = decode(value);
    assert(res && "dereferencing null handle");
    return res;
  }
  static arrow_type arrow(value_type ptr) {
    assert(ptr && "dereferencing null handle");
    return ptr;
  }
};

template <class T>
struct StringTraitsImpl {
  using value_type = T *;
  using arrow_type = value_type;
  static constexpr bool is_cell = true;

  static constexpr value_type defaultValue() {
    return value_type{};
  }
  static HermesValue encode(T *value) {
    return HermesValue::encodeStringValueUnsafe(value);
  }
  static T *decode(HermesValue value) {
    return (T *)value.getString();
  }
  static arrow_type arrow(const HermesValue &value) {
    auto *res = decode(value);
    assert(res && "dereferencing null handle");
    return res;
  }
  static arrow_type arrow(value_type ptr) {
    assert(ptr && "dereferencing null handle");
    return ptr;
  }
};

template <>
struct HermesValueTraits<StringPrimitive, true>
    : public StringTraitsImpl<StringPrimitive> {};
template <>
struct HermesValueTraits<BufferedStringPrimitive<char>, true>
    : public StringTraitsImpl<BufferedStringPrimitive<char>> {};
template <>
struct HermesValueTraits<BufferedStringPrimitive<char16_t>, true>
    : public StringTraitsImpl<BufferedStringPrimitive<char16_t>> {};

template <class T>
struct HermesValueTraits<T, true> {
  using value_type = T *;
  using arrow_type = value_type;
  static constexpr bool is_cell = true;

  static constexpr value_type defaultValue() {
    return value_type{};
  }
  static HermesValue encode(T *value) {
    return HermesValue::encodeObjectValueUnsafe(value);
  }
  static T *decode(HermesValue value) {
    return static_cast<T *>(value.getObject());
  }
  static arrow_type arrow(const HermesValue &value) {
    auto *res = decode(value);
    assert(res && "dereferencing null handle");
    return res;
  }
  static arrow_type arrow(value_type ptr) {
    assert(ptr && "dereferencing null handle");
    return ptr;
  }
};

/// A helper class to deduce whether it is safe to assign Handle<From> into
/// Handle<To>.
template <class From, class To>
struct IsHermesValueConvertible {
  using FromTraits = HermesValueTraits<From>;
  using ToTraits = HermesValueTraits<To>;

  static constexpr bool value =
      // Anything is convertable to HermesValue.
      std::is_same<To, HermesValue>::value ||
      // A type is convertable to itself.
      std::is_same<From, To>::value ||
      // An object can be converted to its base class.
      (FromTraits::is_cell && ToTraits::is_cell &&
       std::is_base_of<To, From>::value);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HERMESVALUETRAITS_H
