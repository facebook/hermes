/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/PrimitiveBox.h"

#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSString

const ObjectVTable JSString::vt{
    VTable(CellKind::JSStringKind, cellSize<JSString>()),
    JSString::_getOwnIndexedRangeImpl,
    JSString::_haveOwnIndexedImpl,
    JSString::_getOwnIndexedPropertyFlagsImpl,
    JSString::_getOwnIndexedImpl,
    JSString::_setOwnIndexedImpl,
    JSString::_deleteOwnIndexedImpl,
    JSString::_checkAllOwnIndexedImpl,
};

void JSStringBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSString>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSString *>(cell);
  mb.setVTable(&JSString::vt);
  mb.addField(&self->primitiveValue_);
}

CallResult<Handle<JSString>> JSString::create(
    Runtime &runtime,
    Handle<StringPrimitive> value,
    Handle<JSObject> parentHandle) {
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *parentHandle, numOverlapSlots<JSString>());
  auto obj =
      runtime.makeAFixed<JSString>(runtime, value, parentHandle, clazzHandle);

  auto selfHandle = JSObjectInit::initToHandle(runtime, obj);

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
              runtime.makeHandle(HermesValue::encodeDoubleValue(
                  value->getStringLength()))) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return selfHandle;
}

void JSString::setPrimitiveString(
    Handle<JSString> selfHandle,
    Runtime &runtime,
    Handle<StringPrimitive> string) {
  NamedPropertyDescriptor desc;
  bool res = JSObject::getOwnNamedDescriptor(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::length), desc);
  assert(res && "cannot find 'length' property");
  (void)res;

  // This is definitely not a proxy because we know strings have lengths.
  auto shv =
      SmallHermesValue::encodeNumberValue(string->getStringLength(), runtime);
  JSObject::setNamedSlotValueUnsafe(*selfHandle, runtime, desc, shv);
  selfHandle->primitiveValue_.set(runtime, *string, runtime.getHeap());
}

bool JSString::_haveOwnIndexedImpl(
    JSObject *self,
    Runtime &runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self), runtime);
  return index < str->getStringLength();
}

OptValue<PropertyFlags> JSString::_getOwnIndexedPropertyFlagsImpl(
    JSObject *self,
    Runtime &runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self), runtime);
  if (index < str->getStringLength()) {
    PropertyFlags flags;
    flags.enumerable = 1;
    return flags;
  }

  return llvh::None;
}

std::pair<uint32_t, uint32_t> JSString::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime &runtime) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfObj), runtime);
  return {0, str->getStringLength()};
}

HermesValue JSString::_getOwnIndexedImpl(
    PseudoHandle<JSObject> self,
    Runtime &runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(self.get()), runtime);

  NoAllocScope noAllocs{runtime};

  if (LLVM_LIKELY(index < str->getStringLength())) {
    auto chr = str->at(index);
    noAllocs.release();
    return runtime.getCharacterString(chr).getHermesValue();
  }

  return HermesValue::encodeEmptyValue();
}

CallResult<bool> JSString::_setOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    uint32_t index,
    Handle<> valueHandle) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfHandle.get()), runtime);

  if (index < str->getStringLength())
    return false;

  // Property indexes beyond the end of the string must be added as named
  // properties.
  auto vr = valueToSymbolID(
      runtime, runtime.makeHandle(HermesValue::encodeNumberValue(index)));
  assert(
      vr != ExecutionStatus::EXCEPTION &&
      "valueToIdentifier() failed for uint32_t value");

  // Can't call defineOwnComputedPrimitive because it would infinitely recurse
  // calling JSString::_setOwnIndexedImpl.
  auto dr = JSObject::defineOwnPropertyInternal(
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
    Runtime &runtime,
    uint32_t index) {
  auto *str = getPrimitiveString(vmcast<JSString>(selfHandle.get()), runtime);

  // Only characters past the end of the string can be deleted (since they
  // already are).
  return index >= str->getStringLength();
}

//===----------------------------------------------------------------------===//
// class JSStringIterator

const ObjectVTable JSStringIterator::vt{
    VTable(CellKind::JSStringIteratorKind, cellSize<JSStringIterator>()),
    JSStringIterator::_getOwnIndexedRangeImpl,
    JSStringIterator::_haveOwnIndexedImpl,
    JSStringIterator::_getOwnIndexedPropertyFlagsImpl,
    JSStringIterator::_getOwnIndexedImpl,
    JSStringIterator::_setOwnIndexedImpl,
    JSStringIterator::_deleteOwnIndexedImpl,
    JSStringIterator::_checkAllOwnIndexedImpl,
};

void JSStringIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSStringIterator>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSStringIterator *>(cell);
  mb.setVTable(&JSStringIterator::vt);
  mb.addField("iteratedString", &self->iteratedString_);
}

/// ES6.0 21.1.5.1 CreateStringIterator Abstract Operation
PseudoHandle<JSStringIterator> JSStringIterator::create(
    Runtime &runtime,
    Handle<StringPrimitive> string) {
  auto proto = Handle<JSObject>::vmcast(&runtime.stringIteratorPrototype);
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *proto, numOverlapSlots<JSStringIterator>());
  auto obj =
      runtime.makeAFixed<JSStringIterator>(runtime, proto, clazzHandle, string);
  return JSObjectInit::initToPseudoHandle(runtime, obj);
}

/// ES6.0 21.1.5.2.1 %StringIteratorPrototype%.next ( ) 4-14
CallResult<HermesValue> JSStringIterator::nextElement(
    Handle<JSStringIterator> self,
    Runtime &runtime) {
  // 4. Let s be the value of the [[IteratedString]] internal slot of O.
  auto s = runtime.makeHandle(self->iteratedString_);
  if (!s) {
    // 5. If s is undefined, return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
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
    self->iteratedString_.setNull(runtime.getHeap());
    // 8b. Return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
        .getHermesValue();
  }

  MutableHandle<StringPrimitive> resultString{runtime};

  // 9. Let first be the code unit value at index position in s.
  char16_t first = s->at(position);
  if (first < 0xd800 || first > 0xdbff || position + 1 == len) {
    // 10. If first < 0xD800 or first > 0xDBFF or position+1 = len,
    // let resultString be the string consisting of the single code unit first.
    resultString = runtime.getCharacterString(first).get();
  } else {
    // 11a. Let second the code unit value at index position+1 in the String S.
    char16_t second = s->at(position + 1);
    if (second < 0xdc00 || second > 0xdfff) {
      // 11b. If second < 0xDC00 or second > 0xDFFF, let resultString be the
      // string consisting of the single code unit first.
      resultString = runtime.getCharacterString(first).get();
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
// class JSBigInt

const ObjectVTable JSBigInt::vt{
    VTable(CellKind::JSBigIntKind, cellSize<JSBigInt>()),
    JSBigInt::_getOwnIndexedRangeImpl,
    JSBigInt::_haveOwnIndexedImpl,
    JSBigInt::_getOwnIndexedPropertyFlagsImpl,
    JSBigInt::_getOwnIndexedImpl,
    JSBigInt::_setOwnIndexedImpl,
    JSBigInt::_deleteOwnIndexedImpl,
    JSBigInt::_checkAllOwnIndexedImpl,
};

void JSBigIntBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSBigInt>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSBigInt *>(cell);
  mb.setVTable(&JSBigInt::vt);
  mb.addField(&self->primitiveValue_);
}

CallResult<Handle<JSBigInt>> JSBigInt::create(
    Runtime &runtime,
    Handle<BigIntPrimitive> value,
    Handle<JSObject> parentHandle) {
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *parentHandle, numOverlapSlots<JSBigInt>());
  auto obj =
      runtime.makeAFixed<JSBigInt>(runtime, value, parentHandle, clazzHandle);

  return JSObjectInit::initToHandle(runtime, obj);
}

//===----------------------------------------------------------------------===//
// class JSNumber

const ObjectVTable JSNumber::vt{
    VTable(CellKind::JSNumberKind, cellSize<JSNumber>()),
    JSNumber::_getOwnIndexedRangeImpl,
    JSNumber::_haveOwnIndexedImpl,
    JSNumber::_getOwnIndexedPropertyFlagsImpl,
    JSNumber::_getOwnIndexedImpl,
    JSNumber::_setOwnIndexedImpl,
    JSNumber::_deleteOwnIndexedImpl,
    JSNumber::_checkAllOwnIndexedImpl,
};

void JSNumberBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSNumber>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSNumber::vt);
}

PseudoHandle<JSNumber> JSNumber::create(
    Runtime &runtime,
    double value,
    Handle<JSObject> parentHandle) {
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *parentHandle, numOverlapSlots<JSNumber>());
  auto obj =
      runtime.makeAFixed<JSNumber>(runtime, value, parentHandle, clazzHandle);
  return JSObjectInit::initToPseudoHandle(runtime, obj);
}

//===----------------------------------------------------------------------===//
// class JSBoolean

const ObjectVTable JSBoolean::vt{
    VTable(CellKind::JSBooleanKind, cellSize<JSBoolean>()),
    JSBoolean::_getOwnIndexedRangeImpl,
    JSBoolean::_haveOwnIndexedImpl,
    JSBoolean::_getOwnIndexedPropertyFlagsImpl,
    JSBoolean::_getOwnIndexedImpl,
    JSBoolean::_setOwnIndexedImpl,
    JSBoolean::_deleteOwnIndexedImpl,
    JSBoolean::_checkAllOwnIndexedImpl,
};

void JSBooleanBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSBoolean>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSBoolean::vt);
}

PseudoHandle<JSBoolean>
JSBoolean::create(Runtime &runtime, bool value, Handle<JSObject> parentHandle) {
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *parentHandle, numOverlapSlots<JSBoolean>());
  auto obj =
      runtime.makeAFixed<JSBoolean>(runtime, value, parentHandle, clazzHandle);
  return JSObjectInit::initToPseudoHandle(runtime, obj);
}

//===----------------------------------------------------------------------===//
// class JSSymbol

const ObjectVTable JSSymbol::vt{
    VTable(CellKind::JSSymbolKind, cellSize<JSSymbol>()),
    _getOwnIndexedRangeImpl,
    _haveOwnIndexedImpl,
    _getOwnIndexedPropertyFlagsImpl,
    _getOwnIndexedImpl,
    _setOwnIndexedImpl,
    _deleteOwnIndexedImpl,
    _checkAllOwnIndexedImpl,
};

void JSSymbolBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSSymbol>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSSymbol *>(cell);
  mb.setVTable(&JSSymbol::vt);
  mb.addField(&self->primitiveValue_);
}

PseudoHandle<JSSymbol> JSSymbol::create(
    Runtime &runtime,
    SymbolID value,
    Handle<JSObject> parentHandle) {
  auto clazzHandle = runtime.getHiddenClassForPrototype(
      *parentHandle, numOverlapSlots<JSSymbol>());
  auto *obj =
      runtime.makeAFixed<JSSymbol>(runtime, value, parentHandle, clazzHandle);
  return JSObjectInit::initToPseudoHandle(runtime, obj);
}

} // namespace vm
} // namespace hermes
