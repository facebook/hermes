/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/PrimitiveBox.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSString

ObjectVTable JSString::vt{
    VTable(CellKind::StringObjectKind, sizeof(JSString)),
    JSString::_getOwnIndexedRangeImpl,
    JSString::_haveOwnIndexedImpl,
    JSString::_getOwnIndexedPropertyFlagsImpl,
    JSString::_getOwnIndexedImpl,
    JSString::_setOwnIndexedImpl,
    JSString::_deleteOwnIndexedImpl,
    JSString::_checkAllOwnIndexedImpl,
};

void StringObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
PrimitiveBox::PrimitiveBox(Deserializer &d, const VTable *vt)
    : JSObject(d, vt) {}

JSString::JSString(Deserializer &d, const VTable *vt) : PrimitiveBox(d, vt) {}

void StringObjectSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void StringObjectDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::StringObjectKind && "Expected StringObject");
  void *mem = d.getRuntime()->alloc(sizeof(JSString));
  auto *cell = new (mem) JSString(d, &JSString::vt.base);

  d.endObject(cell);
}
#endif

CallResult<HermesValue> JSString::create(
    Runtime *runtime,
    Handle<StringPrimitive> value,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSString));
  auto selfHandle = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSString(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));

  JSObject::addInternalProperties(selfHandle, runtime, 1, value);

  PropertyFlags pf;
  pf.writable = 0;
  pf.enumerable = 0;
  pf.configurable = 0;

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              pf,
              runtime->makeHandle(HermesValue::encodeDoubleValue(
                  value->getStringLength()))) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return selfHandle.getHermesValue();
}

void JSString::setPrimitiveString(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    Handle<StringPrimitive> string) {
  NamedPropertyDescriptor desc;
  bool res = JSObject::getOwnNamedDescriptor(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::length), desc);
  assert(res && "cannot find 'length' property");
  (void)res;

  JSObject::setNamedSlotValue(
      *selfHandle,
      runtime,
      desc,
      HermesValue::encodeDoubleValue(string->getStringLength()));
  return JSObject::setInternalProperty(
      *selfHandle,
      runtime,
      PrimitiveBox::primitiveValueIndex,
      string.getHermesValue());
}

bool JSString::_haveOwnIndexedImpl(
    JSObject *self,
    Runtime *runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self), runtime);
  return index < str->getStringLength();
}

OptValue<PropertyFlags> JSString::_getOwnIndexedPropertyFlagsImpl(
    JSObject *self,
    Runtime *runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self), runtime);
  if (index < str->getStringLength()) {
    PropertyFlags flags;
    flags.enumerable = 1;
    return flags;
  }

  return llvm::None;
}

std::pair<uint32_t, uint32_t> JSString::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime *runtime) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfObj), runtime);
  return {0, str->getStringLength()};
}

HermesValue
JSString::_getOwnIndexedImpl(JSObject *self, Runtime *runtime, uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self), runtime);
  return LLVM_LIKELY(index < str->getStringLength())
      ? runtime->getCharacterString(str->at(index)).getHermesValue()
      : HermesValue::encodeEmptyValue();
}

CallResult<bool> JSString::_setOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    uint32_t index,
    Handle<> valueHandle) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfHandle.get()), runtime);

  if (index < str->getStringLength())
    return false;

  // Property indexes beyond the end of the string must be added as named
  // properties.
  auto vr = valueToSymbolID(
      runtime, runtime->makeHandle(HermesValue::encodeNumberValue(index)));
  assert(
      vr != ExecutionStatus::EXCEPTION &&
      "valueToIdentifier() failed for uint32_t value");

  auto dr = JSObject::defineOwnProperty(
      selfHandle,
      runtime,
      **vr,
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      valueHandle);
  assert(
      dr != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() threw in JSString::_setOwnIndexedImpl()");
  return *dr;
}

bool JSString::_deleteOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfHandle.get()), runtime);

  // Only characters past the end of the string can be deleted (since they
  // already are).
  return index >= str->getStringLength();
}

//===----------------------------------------------------------------------===//
// class JSStringIterator

ObjectVTable JSStringIterator::vt{
    VTable(CellKind::StringIteratorKind, sizeof(JSStringIterator)),
    JSStringIterator::_getOwnIndexedRangeImpl,
    JSStringIterator::_haveOwnIndexedImpl,
    JSStringIterator::_getOwnIndexedPropertyFlagsImpl,
    JSStringIterator::_getOwnIndexedImpl,
    JSStringIterator::_setOwnIndexedImpl,
    JSStringIterator::_deleteOwnIndexedImpl,
    JSStringIterator::_checkAllOwnIndexedImpl,
};

void StringIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSStringIterator *>(cell);
  mb.addField("iteratedString", &self->iteratedString_);
}

#ifdef HERMESVM_SERIALIZE
JSStringIterator::JSStringIterator(Deserializer &d) : JSObject(d, &vt.base) {
  d.readRelocation(&iteratedString_, RelocationKind::GCPointer);
  nextIndex_ = d.readInt<uint32_t>();
}

void StringIteratorSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSStringIterator>(cell);
  JSObject::serializeObjectImpl(s, cell);
  s.writeRelocation(self->iteratedString_.get(s.getRuntime()));
  s.writeInt<uint32_t>(self->nextIndex_);
  s.endObject(cell);
}

void StringIteratorDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::StringIteratorKind && "Expected StringIterator");
  void *mem = d.getRuntime()->alloc(sizeof(JSStringIterator));
  auto *cell = new (mem) JSStringIterator(d);
  d.endObject(cell);
}
#endif

CallResult<HermesValue> JSStringIterator::create(
    Runtime *runtime,
    Handle<StringPrimitive> string) {
  auto proto = Handle<JSObject>::vmcast(&runtime->stringIteratorPrototype);

  void *mem = runtime->alloc(sizeof(JSStringIterator));
  auto *self = JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
      new (mem) JSStringIterator(
          runtime,
          *proto,
          runtime->getHiddenClassForPrototypeRaw(*proto),
          *string));
  return HermesValue::encodeObjectValue(self);
}

CallResult<HermesValue> JSStringIterator::nextElement(
    Handle<JSStringIterator> self,
    Runtime *runtime) {
  // 4. Let s be the value of the [[IteratedString]] internal slot of O.
  auto s = runtime->makeHandle(self->iteratedString_);
  if (!s) {
    // 5. If s is undefined, return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, runtime->getUndefinedValue(), true)
        .getHermesValue();
  }

  // 6. Let position be the value of the [[StringIteratorNextIndex]] internal
  // slot of O.
  uint32_t position = self->nextIndex_;
  // 7. Let len be the number of elements in s.
  uint32_t len = s->getStringLength();

  if (position >= len) {
    // 8a. Set the value of the [[IteratedString]] internal slot of O to
    // undefined.
    self->iteratedString_ = nullptr;
    // 8b. Return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, runtime->getUndefinedValue(), true)
        .getHermesValue();
  }

  MutableHandle<StringPrimitive> resultString{runtime};

  // 9. Let first be the code unit value at index position in s.
  char16_t first = s->at(position);
  if (first < 0xd800 || first > 0xdbff || position + 1 == len) {
    // 10. If first < 0xD800 or first > 0xDBFF or position+1 = len,
    // let resultString be the string consisting of the single code unit first.
    resultString = runtime->getCharacterString(first).get();
  } else {
    // 11a. Let second the code unit value at index position+1 in the String S.
    char16_t second = s->at(position + 1);
    if (second < 0xdc00 || second > 0xdfff) {
      // 11b. If second < 0xDC00 or second > 0xDFFF, let resultString be the
      // string consisting of the single code unit first.
      resultString = runtime->getCharacterString(first).get();
    } else {
      // 11c. Let resultString be the string consisting of the code unit first
      // followed by the code unit second.
      char16_t charArr[2]{first, second};
      auto strRes = StringPrimitive::create(runtime, UTF16Ref{charArr, 2});
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      resultString = vmcast<StringPrimitive>(*strRes);
    }
  }

  // 13. Set the value of the [[StringIteratorNextIndex]] internal slot of O to
  // position+resultSize.
  self->nextIndex_ = position + resultString->getStringLength();

  // 14. Return CreateIterResultObject(resultString, false).
  return createIterResultObject(runtime, resultString, false).getHermesValue();
}

//===----------------------------------------------------------------------===//
// class JSNumber

ObjectVTable JSNumber::vt{
    VTable(CellKind::NumberObjectKind, sizeof(JSNumber)),
    JSNumber::_getOwnIndexedRangeImpl,
    JSNumber::_haveOwnIndexedImpl,
    JSNumber::_getOwnIndexedPropertyFlagsImpl,
    JSNumber::_getOwnIndexedImpl,
    JSNumber::_setOwnIndexedImpl,
    JSNumber::_deleteOwnIndexedImpl,
    JSNumber::_checkAllOwnIndexedImpl,
};

void NumberObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSNumber::JSNumber(Deserializer &d, const VTable *vt) : PrimitiveBox(d, vt) {}

void NumberObjectSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void NumberObjectDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::NumberObjectKind && "Expected NumberObject");
  void *mem = d.getRuntime()->alloc(sizeof(JSNumber));
  auto *cell = new (mem) JSNumber(d, &JSNumber::vt.base);
  d.endObject(cell);
}
#endif

CallResult<HermesValue> JSNumber::create(
    Runtime *runtime,
    double value,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSNumber));
  auto selfHandle = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSNumber(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));

  JSObject::addInternalProperties(
      selfHandle,
      runtime,
      1,
      runtime->makeHandle(HermesValue::encodeDoubleValue(value)));

  return selfHandle.getHermesValue();
}

//===----------------------------------------------------------------------===//
// class JSBoolean

ObjectVTable JSBoolean::vt{
    VTable(CellKind::BooleanObjectKind, sizeof(JSBoolean)),
    JSBoolean::_getOwnIndexedRangeImpl,
    JSBoolean::_haveOwnIndexedImpl,
    JSBoolean::_getOwnIndexedPropertyFlagsImpl,
    JSBoolean::_getOwnIndexedImpl,
    JSBoolean::_setOwnIndexedImpl,
    JSBoolean::_deleteOwnIndexedImpl,
    JSBoolean::_checkAllOwnIndexedImpl,
};

void BooleanObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSBoolean::JSBoolean(Deserializer &d, const VTable *vt) : PrimitiveBox(d, vt) {}

void BooleanObjectSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void BooleanObjectDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::BooleanObjectKind && "Expected BooleanObject");
  void *mem = d.getRuntime()->alloc(sizeof(JSBoolean));
  auto *cell = new (mem) JSBoolean(d, &JSBoolean::vt.base);
  d.endObject(cell);
}
#endif

CallResult<HermesValue>
JSBoolean::create(Runtime *runtime, bool value, Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSBoolean));
  auto selfHandle = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSBoolean(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));

  JSObject::addInternalProperties(
      selfHandle, runtime, 1, runtime->getBoolValue(value));
  return selfHandle.getHermesValue();
}

//===----------------------------------------------------------------------===//
// class JSSymbol

ObjectVTable JSSymbol::vt{
    VTable(CellKind::SymbolObjectKind, sizeof(JSSymbol)),
    _getOwnIndexedRangeImpl,
    _haveOwnIndexedImpl,
    _getOwnIndexedPropertyFlagsImpl,
    _getOwnIndexedImpl,
    _setOwnIndexedImpl,
    _deleteOwnIndexedImpl,
    _checkAllOwnIndexedImpl,
};

void SymbolObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSSymbol::JSSymbol(Deserializer &d) : PrimitiveBox(d, &vt.base) {}

void SymbolObjectSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void SymbolObjectDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::SymbolObjectKind && "Expected SymbolObject");
  void *mem = d.getRuntime()->alloc(sizeof(JSSymbol));
  auto *cell = new (mem) JSSymbol(d);
  d.endObject(cell);
}
#endif

CallResult<HermesValue> JSSymbol::create(
    Runtime *runtime,
    SymbolID value,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSSymbol));
  auto selfHandle = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSSymbol(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));

  JSObject::addInternalProperties(
      selfHandle, runtime, 1, runtime->makeHandle(value));

  return selfHandle.getHermesValue();
}

} // namespace vm
} // namespace hermes
