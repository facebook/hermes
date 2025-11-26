/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/RuntimeJSONStringify.h"

#include "Object.h"

#include "hermes/Support/JSON.h"
#include "hermes/VM/ArrayLike.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/PrimitiveBox.h"

#include "llvh/ADT/SmallString.h"

namespace hermes {
namespace vm {

namespace {

/// This class wraps the functionality required to stringify an object
/// as JSON.
class JSONStringifyer {
  /// The runtime.
  Runtime &runtime_;

  /// Locals struct containing all the handles.
  struct : public Locals {
    /// The ReplacerFunction, initialized from the "replacer" argument in
    /// stringify.
    PinnedValue<Callable> replacerFunction;

    /// The gap string, which will be used to construct indent.
    /// Initialized from the "space" argument in stringify.
    /// gap will be nullptr if no gap is specified or if the gap is an empty
    /// string. This means that if gap is not nullptr, it will not be
    /// an empty string.
    PinnedValue<StringPrimitive> gap;

    /// The PropertyList, constructed from the "replacer" argument in stringify.
    PinnedValue<JSArray> propertyList;

    /// The stack, used at runtime by operationJA and operationJO to store
    /// `value_` for recursions.
    PinnedValue<PropStorage> stackValue;

    /// An additional stack just for operationJO to store `K` for recursions.
    PinnedValue<PropStorage> stackJO;

    /// A temporary handle, to avoid creating new handles when a temporary one
    /// is needed.
    /// Note: this should be used with care because it can be shared among
    /// functions.
    PinnedValue<> tmpHandle;

    /// A second temporary handle for when the first is taken.
    PinnedValue<> tmpHandle2;

    /// Handle used by operationStr to store the value.
    PinnedValue<> operationStrValue;

    /// Handle used by operationJO to store K.
    PinnedValue<JSArray> operationJOK;

    /// The holder argument passed to operationStr.
    /// We define a member variable here to avoid creating a new handle
    /// each time we are calling operationStr.
    PinnedValue<JSObject> operationStrHolder;
  } lv_;

  /// RAII manager for the Locals.
  LocalsRAII lraii_;

  /// The current depth of recursion, used at runtime by operationJA and
  /// operationJO. This is used as a stack overflow guard in stringifying. It
  /// also doubles as an indent counter for prettified JS.
  uint32_t depthCount_{0};

  /// The max amount that depthCount_ is allowed to reach. Once it's reached, an
  /// exception will be thrown.
  static constexpr int32_t kMaxRecursionDepth_ =
#ifndef HERMES_LIMIT_STACK_DEPTH
      512
#else
      100
#endif
      ;

  /// The output buffer. The serialization process will append into it.
  llvh::SmallVector<char16_t, 32> output_{};

 public:
  explicit JSONStringifyer(Runtime &runtime)
      : runtime_(runtime), lraii_(runtime, &lv_) {
    lv_.gap = nullptr;
  }

  LLVM_NODISCARD ExecutionStatus init(Handle<> replacer, Handle<> space) {
    auto arrRes = PropStorage::create(runtime_, 4);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.stackValue = vmcast<PropStorage>(*arrRes);
    if (LLVM_UNLIKELY(
            (arrRes = PropStorage::create(runtime_, 4)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.stackJO = vmcast<PropStorage>(*arrRes);
    auto cr = initializeReplacer(replacer);
    if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return initializeSpace(space);
  }

  /// Stringify \p value.
  CallResult<HermesValue> stringify(Handle<> value);

 private:
  /// Check the type of replacer, initialize
  /// ReplacerFunction (lv_.replacerFunction) and PropertyList
  /// (lv_.propertyList). Covers step 3 and 4 in ES5.1 15.12.3.
  LLVM_NODISCARD ExecutionStatus initializeReplacer(Handle<> replacer);

  /// Check the type of space, initialize gap (lv_.gap).
  /// Covers step 5, 6, 7, 8 in ES5.1 15.12.3.
  ExecutionStatus initializeSpace(Handle<> space);

  /// Implement the Str(key, holder) abstract operation to serialize a value.
  /// According to the spec, \p key should always be a string.
  /// However if this function is called from operationJA, we
  /// we don't want to convert every index into string.
  /// Hence we leave the key as it is, and convert them to string if needed.
  /// The holder is always stored in lv_.operationStrHolder by caller.
  /// \return whether the result is not undefined.
  CallResult<bool> operationStr(HermesValue key);

  /// Implement the abstract operation Quote(value).
  /// It wraps a String value in double quotes and escapes characters within it.
  void operationQuote(StringView value);

  /// Implement the abstract operation JA(value). The value to operate on
  /// is always the current last element in lv_.stackValue.
  /// It serializes an array.
  ExecutionStatus operationJA();

  /// Implement the abstract operation JO(value). The value to operate on
  /// is always the current last element in lv_.stackValue.
  /// It serializes an object.
  ExecutionStatus operationJO();

  /// Append '\n' and indent to output_.
  /// The indent is constructed according to depthCount_.
  void indent();

  /// Push a value to stack when traversing the object recursively.
  /// It also checks if the value already exsists in the stack for
  /// cyclic recursion.
  /// \return true if the push is successful (no cyclic).
  CallResult<bool> pushValueToStack(HermesValue value);

  /// Pop the top of stack.
  void popValueFromStack();

  /// Append the string indicated as \p identifierID to output_.
  void appendToOutput(SymbolID identifierID);

  /// Append the string indicated as \p str to output_.
  void appendToOutput(const StringPrimitive *str);
};
} // namespace

ExecutionStatus JSONStringifyer::initializeReplacer(Handle<> replacer) {
  if (!vmisa<JSObject>(*replacer))
    return ExecutionStatus::RETURNED;
  // replacer is an object.

  lv_.replacerFunction = dyn_vmcast<Callable>(*replacer);
  if (lv_.replacerFunction.get())
    return ExecutionStatus::RETURNED;
  // replacer is not a callable.

  auto replacerArray = Handle<JSObject>::dyn_vmcast(replacer);
  CallResult<bool> isArrayRes =
      isArray(runtime_, dyn_vmcast<JSObject>(*replacerArray));
  if (LLVM_UNLIKELY(isArrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!*isArrayRes)
    return ExecutionStatus::RETURNED;
  // replacer is arrayish

  CallResult<uint64_t> lenRes = getArrayLikeLength_RJS(replacerArray, runtime_);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*lenRes > UINT32_MAX) {
    return runtime_.raiseRangeError("replacer array is too large");
  }
  uint32_t len = static_cast<uint32_t>(*lenRes);
  auto arrRes = JSArray::create(runtime_, len, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv_.propertyList = std::move(*arrRes);

  // Iterate through all indexes, in ascending order.
  GCScope gcScope{runtime_};
  auto marker = gcScope.createMarker();
  for (uint64_t i = 0, e = *lenRes; i < e; ++i) {
    gcScope.flushToMarker(marker);

    // Get the property value.
    auto propRes = getIndexed_RJS(runtime_, replacerArray, i);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    PseudoHandle<> v = std::move(*propRes);
    // Convert v to string and store into item, if v is string, number, JSString
    // or JSNumber.
    if (v->isString()) {
      lv_.tmpHandle = std::move(v);
    } else if (
        v->isNumber() || vmisa<JSNumber>(v.get()) || vmisa<JSString>(v.get())) {
      lv_.tmpHandle = std::move(v);
      auto strRes = toString_RJS(runtime_, lv_.tmpHandle);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.tmpHandle = strRes->getHermesValue();
    } else {
      lv_.tmpHandle = HermesValue::encodeUndefinedValue();
    }

    if (lv_.tmpHandle->isUndefined())
      continue;
    // We only add item to propertyList if item is not already an element.
    bool exists = false;
    auto len = lv_.propertyList->getEndIndex();
    for (uint32_t i = 0; i < len; ++i) {
      if (lv_.propertyList->at(runtime_, i)
              .getString(runtime_)
              ->equals(lv_.tmpHandle->getString())) {
        exists = true;
        break;
      }
    }
    if (!exists) {
      if (LLVM_UNLIKELY(
              JSArray::setElementAt(
                  lv_.propertyList, runtime_, len, lv_.tmpHandle) ==
              ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
    }
  }
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSONStringifyer::initializeSpace(Handle<> space) {
  lv_.tmpHandle = *space;
  if (vmisa<JSNumber>(*lv_.tmpHandle)) {
    auto numRes = toNumber_RJS(runtime_, lv_.tmpHandle);
    if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.tmpHandle = *numRes;
  } else if (vmisa<JSString>(*lv_.tmpHandle)) {
    auto strRes = toString_RJS(runtime_, lv_.tmpHandle);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.tmpHandle = strRes->getHermesValue();
  }
  if (lv_.tmpHandle->isNumber()) {
    auto intRes = toIntegerOrInfinity(runtime_, lv_.tmpHandle);
    assert(
        intRes != ExecutionStatus::EXCEPTION &&
        "toInteger on a number cannot throw");
    // Clamp result to [0,10].
    auto spaceCount =
        static_cast<int>(std::max(0.0, std::min(10.0, intRes->getNumber())));
    if (spaceCount > 0) {
      // Construct a string with spaceCount spaces.
      llvh::SmallString<32> spaces;
      for (int i = 0; i < spaceCount; ++i) {
        spaces.push_back(' ');
      }
      auto strRes = StringPrimitive::create(runtime_, spaces);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.gap = strRes->getString();
    }
  } else if (vmisa<StringPrimitive>(lv_.tmpHandle.getHermesValue())) {
    auto str = Handle<StringPrimitive>::vmcast(&lv_.tmpHandle);
    if (str->getStringLength() > 10) {
      auto strRes = StringPrimitive::slice(runtime_, str, 0, 10);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.gap = strRes->getString();
    } else if (str->getStringLength() > 0) {
      // If the string is empty, we don't set the gap.
      lv_.gap = str.get();
    }
  }
  return ExecutionStatus::RETURNED;
}

CallResult<bool> JSONStringifyer::operationStr(HermesValue key) {
  struct : public Locals {
    PinnedValue<> hValueHV;
    PinnedValue<Callable> toJSON;
  } lv;
  LocalsRAII lraii(runtime_, &lv);

  GCScopeMarkerRAII marker{runtime_};
  lv_.tmpHandle = key;

  // Str.1: access holder[key].
  auto propRes = JSObject::getComputed_RJS(
      lv_.operationStrHolder, runtime_, lv_.tmpHandle);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv_.operationStrValue = propRes->get();

  // Str.2. If Type(value) is Object or BigInt, then
  lv.hValueHV = *lv_.operationStrValue;
  if (vmisa<BigIntPrimitive>(*lv_.operationStrValue)) {
    CallResult<HermesValue> hObjRes = toObject(runtime_, lv.hValueHV);
    if (LLVM_UNLIKELY(hObjRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    assert(vmisa<JSBigInt>(*hObjRes) && "if not boxed bigint, then what?!");
    assert(vmisa<JSObject>(*hObjRes) && "if not object, then what?!");
    lv.hValueHV = std::move(*hObjRes);
  }

  if (vmisa<JSObject>(lv.hValueHV.getHermesValue())) {
    auto valueObj = Handle<JSObject>::vmcast(&lv.hValueHV);
    // Str.2.
    // Str.2.a: check if toJSON exists in value.
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getNamedWithReceiver_RJS(
                 valueObj,
                 runtime_,
                 Predefined::getSymbolID(Predefined::toJSON),
                 lv_.operationStrValue)) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // Str.2.b: check if toJSON is a Callable.
    auto toJSONPseudo = std::move(*propRes);
    if (vmisa<Callable>(toJSONPseudo.getHermesValue())) {
      lv.toJSON = vmcast<Callable>(toJSONPseudo.getHermesValue());
      if (!lv_.tmpHandle->isString()) {
        // Lazily convert key to a string.
        auto status = toString_RJS(runtime_, lv_.tmpHandle);
        assert(
            status != ExecutionStatus::EXCEPTION &&
            "toString on a property cannot fail");
        lv_.tmpHandle = status->getHermesValue();
      }
      // Call toJSON with key as argument, value as this.
      auto callRes = Callable::executeCall1(
          Handle<Callable>::vmcast(&lv.toJSON),
          runtime_,
          lv_.operationStrValue,
          *lv_.tmpHandle);
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.operationStrValue = std::move(*callRes);
    }
  }

  // Str.3.
  if (lv_.replacerFunction.get()) {
    // Str.3.a.
    if (!lv_.tmpHandle->isString()) {
      // Lazily convert key to a string.
      auto status = toString_RJS(runtime_, lv_.tmpHandle);
      assert(
          status != ExecutionStatus::EXCEPTION &&
          "toString on a property cannot fail");
      lv_.tmpHandle = status->getHermesValue();
    }
    // If ReplacerFunction exists, call it with key and value as argument,
    // holder as this.
    auto callRes = Callable::executeCall2(
        lv_.replacerFunction,
        runtime_,
        lv_.operationStrHolder,
        *lv_.tmpHandle,
        *lv_.operationStrValue);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.operationStrValue = std::move(*callRes);
  }

  // Str.4: unbox value if necessary.
  // If Type(value) is Object, then
  if (vmisa<JSNumber>(*lv_.operationStrValue)) {
    //  If value has a [[NumberData]] internal slot, then
    //      Set value to ? ToNumber(value).
    auto numRes = toNumber_RJS(runtime_, lv_.operationStrValue);
    if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.operationStrValue = *numRes;
  } else if (vmisa<JSString>(*lv_.operationStrValue)) {
    //  Else if value has a [[StringData]] internal slot, then
    //      Set value to ? ToString(value).
    auto strRes = toString_RJS(runtime_, lv_.operationStrValue);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv_.operationStrValue = strRes->getHermesValue();
  } else if (auto *jsBool = dyn_vmcast<JSBoolean>(*lv_.operationStrValue)) {
    //  Else if value has a [[BooleanData]] internal slot, then
    //      Set value to value.[[BooleanData]].
    lv_.operationStrValue =
        HermesValue::encodeBoolValue(jsBool->getPrimitiveBoolean());
  } else if (auto jsBigInt = dyn_vmcast<JSBigInt>(*lv_.operationStrValue)) {
    //  Else if value has a [[BigIntData]] internal slot, then
    //      Set value to value.[[BigIntData]]
    BigIntPrimitive *bigintData =
        JSBigInt::getPrimitiveBigInt(jsBigInt, runtime_);
    lv_.operationStrValue = HermesValue::encodeBigIntValue(bigintData);
  }

  // Str.5.
  if (lv_.operationStrValue->isNull()) {
    appendToOutput(Predefined::getSymbolID(Predefined::null));
    return true;
  }

  if (lv_.operationStrValue->isBool()) {
    if (lv_.operationStrValue->getBool()) {
      // Str.6.
      appendToOutput(Predefined::getSymbolID(Predefined::trueStr));
    } else {
      // Str.7.
      appendToOutput(Predefined::getSymbolID(Predefined::falseStr));
    }
    return true;
  }

  // Str.8.
  if (lv_.operationStrValue->isString()) {
    operationQuote(
        StringPrimitive::createStringView(
            runtime_, Handle<StringPrimitive>::vmcast(&lv_.operationStrValue)));
    return true;
  }

  // Str.9.
  if (lv_.operationStrValue->isNumber()) {
    if (std::isfinite(lv_.operationStrValue->getNumber())) {
      auto status = toString_RJS(runtime_, lv_.operationStrValue);
      assert(
          status != ExecutionStatus::EXCEPTION &&
          "toString on a number cannot fail");
      appendToOutput(status->get());
    } else {
      appendToOutput(Predefined::getSymbolID(Predefined::null));
    }
    return true;
  }

  // Str.10
  if (vmisa<BigIntPrimitive>(*lv_.operationStrValue)) {
    return runtime_.raiseTypeError("Do not know how to serialize a BigInt");
  }

  // Str.11.
  if (vmisa<JSObject>(*lv_.operationStrValue) &&
      !vmisa<Callable>(*lv_.operationStrValue)) {
    auto cr = pushValueToStack(*lv_.operationStrValue);

    if (cr == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_UNLIKELY(!*cr)) {
      return runtime_.raiseTypeError("cyclical structure in JSON object");
    }
    // Flush just before the recursive call (pushValueToStack can create
    // handles).
    marker.flush();
    CallResult<bool> isArrayRes =
        isArray(runtime_, vmcast<JSObject>(*lv_.operationStrValue));
    if (LLVM_UNLIKELY(isArrayRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    ExecutionStatus status = *isArrayRes ? operationJA() : operationJO();
    popValueFromStack();
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return true;
  }

  // Str.12.
  return false;
}

void JSONStringifyer::operationQuote(StringView value) {
  if (value.isASCII()) {
    quoteStringForJSON(
        output_, ASCIIRef{value.castToCharPtr(), value.length()});
  } else {
    quoteStringForJSON(
        output_, UTF16Ref{value.castToChar16Ptr(), value.length()});
  }
}

ExecutionStatus JSONStringifyer::operationJA() {
  struct : public Locals {
    PinnedValue<JSObject> arrayObject;
  } lv;
  LocalsRAII lraii(runtime_, &lv);

  GCScopeMarkerRAII marker{runtime_};

  // JA.3.
  auto stepBack = depthCount_;
  // JA.4.
  if (depthCount_ + 1 >= kMaxRecursionDepth_) {
    return runtime_.raiseStackOverflow(
        Runtime::StackOverflowKind::JSONStringify);
  }
  depthCount_++;
  output_.push_back(u'[');
  lv.arrayObject.castAndSetHermesValue<JSObject>(
      lv_.stackValue->at(lv_.stackValue->size() - 1).unboxToHV(runtime_));
  CallResult<uint64_t> lenRes =
      getArrayLikeLength_RJS(lv.arrayObject, runtime_);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*lenRes > 0) {
    // If array is not empty, we need to lead with an indent.
    indent();
  }
  // JA.5, 6, 7, 8.
  for (uint64_t index = 0; index < *lenRes; ++index) {
    if (index > 0) {
      // JA.10.
      output_.push_back(u',');
      indent();
    }
    // JA.8.a.
    lv_.operationStrHolder = vmcast<JSObject>(
        lv_.stackValue->at(lv_.stackValue->size() - 1).getObject(runtime_));
    // Flush just before the recursion in case any handles were created.
    marker.flush();
    auto status = operationStr(HermesValue::encodeTrustedNumberValue(index));
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_UNLIKELY(!status.getValue())) {
      // operationStr returns undefined, we need to replace with null.
      appendToOutput(Predefined::getSymbolID(Predefined::null));
    }
  }
  depthCount_ = stepBack;

  if (*lenRes > 0) {
    indent();
  }
  output_.push_back(u']');
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSONStringifyer::operationJO() {
  GCScopeMarkerRAII marker{runtime_};

  // JO.3.
  auto stepBack = depthCount_;
  // JO.4.
  if (depthCount_ + 1 >= kMaxRecursionDepth_) {
    return runtime_.raiseStackOverflow(
        Runtime::StackOverflowKind::JSONStringify);
  }
  depthCount_++;
  output_.push_back(u'{');
  auto beginningLoc = output_.size();
  indent();

  if (lv_.propertyList.get()) {
    // JO.5.
    lv_.operationJOK = lv_.propertyList.get();
  } else {
    // JO.6.
    lv_.tmpHandle = HermesValue::encodeObjectValue(
        lv_.stackValue->at(lv_.stackValue->size() - 1).getObject(runtime_));
    if (LLVM_LIKELY(
            !Handle<JSObject>::vmcast(&lv_.tmpHandle)->isProxyObject())) {
      // enumerableOwnProperties_RJS is the spec definition, and is
      // used below on proxies so the correct traps get called.  In
      // the common case of a non-proxy object, we can do less work by
      // calling getOwnPropertyNames.
      auto cr = JSObject::getOwnPropertyNames(
          Handle<JSObject>::vmcast(&lv_.tmpHandle), runtime_, true);
      if (cr == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.operationJOK = **cr;
    } else {
      CallResult<HermesValue> ownPropRes = enumerableOwnProperties_RJS(
          runtime_,
          Handle<JSObject>::vmcast(&lv_.tmpHandle),
          EnumerableOwnPropertiesKind::Key);
      if (ownPropRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      lv_.operationJOK = vmcast<JSArray>(*ownPropRes);
    }
  }

  marker.flush();

  // JO.8.
  bool hasElement = false;
  for (uint32_t index = 0, len = lv_.operationJOK->getEndIndex(); index < len;
       ++index) {
    // JO.8.a.
    // We are speculating that the Str operation will not return undefined,
    // and just append the key/value pair to the output. If it turns out
    // that the Str operation does return undefined, we roll back to
    // curLocation.
    auto savedLocation = output_.size();

    if (hasElement) {
      // JO.10.
      output_.push_back(u',');
      indent();
    }

    lv_.tmpHandle = lv_.operationJOK->at(runtime_, index).unboxToHV(runtime_);
    if (LLVM_UNLIKELY(!lv_.tmpHandle->isString())) {
      // property may come from getOwnPropertyNames, which may contain numbers.
      // getOwnPropertyNames and lv_.propertyList are both only populated
      // with strings, numbers, and undefined only.
      // None of them are objects, so toString cannot throw.
      assert(!lv_.tmpHandle->isObject() && "property name is an object");
      auto status = toString_RJS(runtime_, lv_.tmpHandle);
      assert(
          status != ExecutionStatus::EXCEPTION &&
          "toString on a property cannot fail");
      lv_.tmpHandle = status->getHermesValue();
    }
    // tmpHandle now contains property as string.
    // JO.8.b.i
    operationQuote(
        StringPrimitive::createStringView(
            runtime_, Handle<StringPrimitive>::vmcast(&lv_.tmpHandle)));
    // JO.8.b.ii
    output_.push_back(u':');
    // JO.8.b.iii
    if (lv_.gap.get()) {
      output_.push_back(u' ');
    }

    // JO.9.a.
    lv_.operationStrHolder = vmcast<JSObject>(
        lv_.stackValue->at(lv_.stackValue->size() - 1).getObject(runtime_));

    lv_.tmpHandle2 = lv_.operationJOK.getHermesValue();
    if (PropStorage::push_back(lv_.stackJO, runtime_, lv_.tmpHandle2) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    // Flush just before recursion (propStoragePushBack may create handles).
    marker.flush();
    auto result = operationStr(*lv_.tmpHandle);

    lv_.operationJOK =
        vmcast<JSArray>(lv_.stackJO->pop_back(runtime_).getObject(runtime_));

    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (LLVM_UNLIKELY(!result.getValue())) {
      // Str returns undefined, we need to roll back.
      output_.resize(savedLocation);
    } else {
      hasElement = true;
    }
  }
  // It's important to reset depthCount_ first, because the last
  // indent before } should be the old indent.
  depthCount_ = stepBack;

  if (hasElement) {
    indent();
  } else {
    // If the object is empty, we need to roll back the first indent.
    output_.resize(beginningLoc);
  }
  output_.push_back(u'}');
  return ExecutionStatus::RETURNED;
}

void JSONStringifyer::indent() {
  if (lv_.gap.get()) {
    output_.push_back(u'\n');
    for (uint32_t i = 0; i < depthCount_; ++i) {
      appendToOutput(lv_.gap.get());
    }
  }
}

CallResult<bool> JSONStringifyer::pushValueToStack(HermesValue value) {
  assert(vmisa<JSObject>(value) && "Can only push object to stack");

  for (uint32_t i = 0, len = lv_.stackValue->size(); i < len; ++i) {
    if (lv_.stackValue->at(i).getObject(runtime_) == value.getObject()) {
      return false;
    }
  }

  lv_.tmpHandle = value;
  if (PropStorage::push_back(lv_.stackValue, runtime_, lv_.tmpHandle) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return true;
}

void JSONStringifyer::popValueFromStack() {
  assert(lv_.stackValue->size() && "Cannot pop from an empty stack");
  lv_.stackValue->pop_back(runtime_);
}

void JSONStringifyer::appendToOutput(SymbolID identifierID) {
  appendToOutput(runtime_.getStringPrimFromSymbolID(identifierID));
}

void JSONStringifyer::appendToOutput(const StringPrimitive *str) {
  str->appendUTF16String(output_);
}

CallResult<HermesValue> JSONStringifyer::stringify(Handle<> value) {
  // All previous steps have been covered by the constructor.
  // Clear the output buffer.
  output_.clear();

  // Step 9, 10 in ES5.1 15.12.3.
  lv_.operationStrHolder = JSObject::create(runtime_).get();
  auto status = JSObject::defineOwnProperty(
      lv_.operationStrHolder,
      runtime_,
      Predefined::getSymbolID(Predefined::emptyString),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      value);
  assert(
      status != ExecutionStatus::EXCEPTION && *status &&
      "defineOwnProperty on a newly created object cannot fail");
  (void)status;

  // Step 11 in ES5.1 15.12.3.
  status = operationStr(
      HermesValue::encodeStringValue(
          runtime_.getPredefinedString(Predefined::emptyString)));
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (status.getValue()) {
    return StringPrimitive::create(runtime_, output_);
  } else {
    return HermesValue::encodeUndefinedValue();
  }
}

CallResult<HermesValue> runtimeJSONStringify(
    Runtime &runtime,
    Handle<> value,
    Handle<> replacer,
    Handle<> space) {
  GCScope gcScope{runtime, "runtimeJSONStringify"};

  JSONStringifyer stringifyer{runtime};
  if (stringifyer.init(replacer, space) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return stringifyer.stringify(value);
}

} // namespace vm
} // namespace hermes
