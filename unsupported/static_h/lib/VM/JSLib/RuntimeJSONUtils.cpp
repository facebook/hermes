/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/RuntimeJSONUtils.h"

#include "Object.h"

#include "hermes/Support/Compiler.h"
#include "hermes/Support/JSON.h"
#include "hermes/Support/UTF16Stream.h"
#include "hermes/VM/ArrayLike.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/PrimitiveBox.h"

#include "JSONLexer.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/SaveAndRestore.h"

namespace hermes {
namespace vm {

namespace {

/// This class wraps the functionality required to parse a JSON string into
/// a VM runtime value. It expects a UTF8 string as input, and returns a
/// HermesValue when parse is called.
class RuntimeJSONParser {
 public:
  static constexpr int32_t MAX_RECURSION_DEPTH =
#ifndef HERMES_LIMIT_STACK_DEPTH
      512
#else
      100
#endif
      ;

 private:
  /// The VM runtime.
  Runtime &runtime_;

  /// The lexer.
  JSONLexer lexer_;

  /// Stores the optional reviver parameter.
  /// https://es5.github.io/#x15.12.2
  Handle<Callable> reviver_;

  /// A temporary handle, to avoid creating new handles when a temporary one
  /// is needed to protect some HermesValue.
  MutableHandle<> tmpHandle_;

  /// How many more nesting levels we allow before error.
  /// Decremented every time a nested level is started,
  /// and incremented again when leaving the nest.
  /// If it drops below 0 while parsing, raise a stack overflow.
  int32_t remainingDepth_{MAX_RECURSION_DEPTH};

 public:
  explicit RuntimeJSONParser(
      Runtime &runtime,
      UTF16Stream &&jsonString,
      Handle<Callable> reviver)
      : runtime_(runtime),
        lexer_(runtime, std::move(jsonString)),
        reviver_(reviver),
        tmpHandle_(runtime) {}

  /// Parse JSON string through lexer_, create objects using runtime_.
  /// If errors occur, this function will return undefined, and the error
  /// should be kept inside SourceErrorManager.
  CallResult<HermesValue> parse();

 private:
  /// Parse a JSON value, starting from the current token.
  /// When this function is finished, the current token will be set
  /// to the next token after the current parsed value.
  CallResult<HermesValue> parseValue();

  /// Parse a JSON array, starting from the "[" token.
  /// When this function is finished, the current token must be "]".
  CallResult<HermesValue> parseArray();

  /// Parse a JSON object, starting from the "{" token.
  /// When this function is finished, the current token must be "}".
  CallResult<HermesValue> parseObject();

  /// Use reviver to filter the result.
  CallResult<HermesValue> revive(Handle<> value);

  /// The abstract operation Walk is a recursive abstract operation that takes
  /// two parameters: a holder object and the String name of a property in
  CallResult<HermesValue> operationWalk(
      Handle<JSObject> holder,
      Handle<> property);

  /// Given value and key, recursively call operationWalk to generate a new
  /// filtered value. It's a helper function for operationWalk, and does not
  /// need to return a value
  ExecutionStatus filter(Handle<JSObject> val, Handle<> key);
};

/// This class wraps the functionality required to stringify an object
/// as JSON.
class JSONStringifyer {
  /// The runtime.
  Runtime &runtime_;

  /// The ReplacerFunction, initialized from the "replacer" argument in
  /// stringify.
  MutableHandle<Callable> replacerFunction_;

  /// The gap string, which will be used to construct indent.
  /// Initialized from the "space" argument in stringify.
  /// gap_ will be nullptr if no gap is specified or if the gap is an empty
  /// string. This means that if gap_ is not nullptr, it will not be
  /// an empty string.
  MutableHandle<StringPrimitive> gap_;

  /// The PropertyList, constructed from the "replacer" argument in stringify.
  MutableHandle<JSArray> propertyList_;

  /// The stack, used at runtime by operationJA and operationJO to store
  /// `value_` for recursions.
  MutableHandle<PropStorage> stackValue_;

  /// An additional stack just for operationJO to store `K` for recursions.
  MutableHandle<PropStorage> stackJO_;

  /// A temporary handle, to avoid creating new handles when a temporary one
  /// is needed.
  /// Note: this should be used with care because it can be shared among
  /// functions.
  MutableHandle<> tmpHandle_;

  /// A second temporary handle for when the first is taken.
  MutableHandle<> tmpHandle2_;

  /// Handle used by operationStr to store the value.
  MutableHandle<> operationStrValue_;

  /// Handle used by operationJO to store K.
  MutableHandle<JSArray> operationJOK_;

  /// The holder argument passed to operationStr.
  /// We define a member variable here to avoid creating a new handle
  /// each time we are calling operationStr.
  MutableHandle<JSObject> operationStrHolder_;

  /// The current depth of recursion, used at runtime by operationJA and
  /// operationJO. This is used as a stack overflow guard in stringifying. It
  /// also doubles as an indent counter for prettified JS.
  uint32_t depthCount_{0};

  /// The max amount that depthCount_ is allowed to reach. Once it's reached, an
  /// exception will be thrown.
  static constexpr uint32_t MAX_RECURSION_DEPTH{
      RuntimeJSONParser::MAX_RECURSION_DEPTH};

  /// The output buffer. The serialization process will append into it.
  llvh::SmallVector<char16_t, 32> output_{};

 public:
  explicit JSONStringifyer(Runtime &runtime)
      : runtime_(runtime),
        replacerFunction_(runtime),
        gap_{runtime, nullptr},
        propertyList_(runtime),
        stackValue_(runtime),
        stackJO_(runtime),
        tmpHandle_(runtime),
        tmpHandle2_(runtime),
        operationStrValue_(runtime),
        operationJOK_(runtime),
        operationStrHolder_(runtime) {}

  LLVM_NODISCARD ExecutionStatus init(Handle<> replacer, Handle<> space) {
    auto arrRes = PropStorage::create(runtime_, 4);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    stackValue_ = vmcast<PropStorage>(*arrRes);
    if (LLVM_UNLIKELY(
            (arrRes = PropStorage::create(runtime_, 4)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    stackJO_ = vmcast<PropStorage>(*arrRes);
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
  /// ReplacerFunction (replacerFunction_) and PropertyList (propertyList_).
  /// Covers step 3 and 4 in ES5.1 15.12.3.
  LLVM_NODISCARD ExecutionStatus initializeReplacer(Handle<> replacer);

  /// Check the type of space, initialize gap (gap_).
  /// Covers step 5, 6, 7, 8 in ES5.1 15.12.3.
  ExecutionStatus initializeSpace(Handle<> space);

  /// Implement the Str(key, holder) abstract operation to serialize a value.
  /// According to the spec, \p key should always be a string.
  /// However if this function is called from operationJA, we
  /// we don't want to convert every index into string.
  /// Hence we leave the key as it is, and convert them to string if needed.
  /// The holder is always stored in operationStrHolder_ by caller.
  /// \return whether the result is not undefined.
  CallResult<bool> operationStr(HermesValue key);

  /// Implement the abstract operation Quote(value).
  /// It wraps a String value in double quotes and escapes characters within it.
  void operationQuote(StringView value);

  /// Implement the abstract operation JA(value). The value to operate on
  /// is always the current last element in stackValue_.
  /// It serializes an array.
  ExecutionStatus operationJA();

  /// Implement the abstract operation JO(value). The value to operate on
  /// is always the current last element in stackValue_.
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

CallResult<HermesValue> RuntimeJSONParser::parse() {
  // parseValue() requires one token to start with.
  if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto parRes = parseValue();
  if (LLVM_UNLIKELY(parRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Make sure the next token must be EOF.
  if (LLVM_UNLIKELY(lexer_.getCurToken()->getKind() != JSONTokenKind::Eof)) {
    return lexer_.errorWithChar(
        "Unexpected token: ", lexer_.getCurToken()->getFirstChar());
  }

  if (reviver_.get()) {
    if ((parRes = revive(runtime_.makeHandle(*parRes))) ==
        ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
  }
  return parRes;
}

CallResult<HermesValue> RuntimeJSONParser::parseValue() {
  llvh::SaveAndRestore<decltype(remainingDepth_)> oldDepth{
      remainingDepth_, remainingDepth_ - 1};
  if (remainingDepth_ <= 0) {
    return runtime_.raiseStackOverflow(Runtime::StackOverflowKind::JSONParser);
  }

  MutableHandle<> returnValue{runtime_};
  switch (lexer_.getCurToken()->getKind()) {
    case JSONTokenKind::String:
      returnValue = lexer_.getCurToken()->getString().getHermesValue();
      break;
    case JSONTokenKind::Number:
      returnValue =
          HermesValue::encodeDoubleValue(lexer_.getCurToken()->getNumber());
      break;
    case JSONTokenKind::LBrace: {
      auto parRes = parseObject();
      if (LLVM_UNLIKELY(parRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      returnValue = *parRes;
      break;
    }
    case JSONTokenKind::LSquare: {
      auto parRes = parseArray();
      if (LLVM_UNLIKELY(parRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      returnValue = *parRes;
      break;
    }
    case JSONTokenKind::True:
      returnValue = HermesValue::encodeBoolValue(true);
      break;
    case JSONTokenKind::False:
      returnValue = HermesValue::encodeBoolValue(false);
      break;
    case JSONTokenKind::Null:
      returnValue = HermesValue::encodeNullValue();
      break;

    default:
      if (lexer_.getCurToken()->getKind() == JSONTokenKind::Eof) {
        return lexer_.error("Unexpected end of input");
      }
      return lexer_.errorWithChar(
          "Unexpected token: ", lexer_.getCurToken()->getFirstChar());
  }

  if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return returnValue.getHermesValue();
}

CallResult<HermesValue> RuntimeJSONParser::parseArray() {
  assert(
      lexer_.getCurToken()->getKind() == JSONTokenKind::LSquare &&
      "Wrong entrance to parseArray");
  auto arrRes = JSArray::create(runtime_, 4, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto array = *arrRes;

  if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (lexer_.getCurToken()->getKind() != JSONTokenKind::RSquare) {
    MutableHandle<> indexValue{runtime_};
    GCScope gcScope{runtime_};
    auto marker = gcScope.createMarker();

    for (uint32_t index = 0;; ++index) {
      gcScope.flushToMarker(marker);

      auto parRes = parseValue();
      if (LLVM_UNLIKELY(parRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      indexValue = HermesValue::encodeDoubleValue(index);
      (void)JSObject::defineOwnComputedPrimitive(
          array,
          runtime_,
          indexValue,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          runtime_.makeHandle(*parRes));

      if (lexer_.getCurToken()->getKind() == JSONTokenKind::Comma) {
        if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        continue;
      } else if (lexer_.getCurToken()->getKind() == JSONTokenKind::RSquare) {
        break;
      } else {
        return lexer_.error("Expect ']'");
      }
    }
    assert(
        lexer_.getCurToken()->getKind() == JSONTokenKind::RSquare &&
        "Unexpected break for array parse");
  }

  return array.getHermesValue();
}

CallResult<HermesValue> RuntimeJSONParser::parseObject() {
  assert(
      lexer_.getCurToken()->getKind() == JSONTokenKind::LBrace &&
      "Wrong entrance to parseObject");
  auto object = runtime_.makeHandle(JSObject::create(runtime_));

  if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (lexer_.getCurToken()->getKind() != JSONTokenKind::RBrace) {
    MutableHandle<StringPrimitive> key{runtime_};
    GCScope gcScope{runtime_};
    auto marker = gcScope.createMarker();
    for (;;) {
      gcScope.flushToMarker(marker);

      if (LLVM_UNLIKELY(
              lexer_.getCurToken()->getKind() != JSONTokenKind::String)) {
        return lexer_.error("Expect a string key in JSON object");
      }
      key = lexer_.getCurToken()->getString().get();

      if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      if (lexer_.getCurToken()->getKind() != JSONTokenKind::Colon) {
        return lexer_.error("Expect ':' after the key in JSON object");
      }

      if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      auto parRes = parseValue();
      if (LLVM_UNLIKELY(parRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      (void)JSObject::defineOwnComputedPrimitive(
          object,
          runtime_,
          key,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          runtime_.makeHandle(*parRes));

      if (lexer_.getCurToken()->getKind() == JSONTokenKind::Comma) {
        if (LLVM_UNLIKELY(lexer_.advance() == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        continue;
      } else if (lexer_.getCurToken()->getKind() == JSONTokenKind::RBrace) {
        break;
      } else {
        return lexer_.error("Expect '}'");
      }
    }
    assert(
        lexer_.getCurToken()->getKind() == JSONTokenKind::RBrace &&
        "Unexpected stop for object parse");
  }

  return object.getHermesValue();
}

CallResult<HermesValue> RuntimeJSONParser::revive(Handle<> value) {
  auto root = runtime_.makeHandle(JSObject::create(runtime_));
  auto status = JSObject::defineOwnProperty(
      root,
      runtime_,
      Predefined::getSymbolID(Predefined::emptyString),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      value);
  (void)status;
  assert(
      status != ExecutionStatus::EXCEPTION && *status &&
      "defineOwnProperty on new object cannot fail");
  return operationWalk(
      root, runtime_.getPredefinedStringHandle(Predefined::emptyString));
}

CallResult<HermesValue> RuntimeJSONParser::operationWalk(
    Handle<JSObject> holder,
    Handle<> property) {
  // The operation is recursive so it needs a GCScope.
  GCScope gcScope(runtime_);

  llvh::SaveAndRestore<decltype(remainingDepth_)> oldDepth{
      remainingDepth_, remainingDepth_ - 1};
  if (remainingDepth_ <= 0) {
    return runtime_.raiseStackOverflow(Runtime::StackOverflowKind::JSONParser);
  }

  auto propRes = JSObject::getComputed_RJS(holder, runtime_, property);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  MutableHandle<> tmpHandle{runtime_};
  CallResult<bool> isArrayRes =
      isArray(runtime_, dyn_vmcast<JSObject>(propRes->get()));
  if (LLVM_UNLIKELY(isArrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto valHandle = runtime_.makeHandle(std::move(*propRes));
  if (*isArrayRes) {
    Handle<JSObject> objHandle = Handle<JSObject>::vmcast(valHandle);
    CallResult<uint64_t> lenRes = getArrayLikeLength(objHandle, runtime_);
    if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    GCScopeMarkerRAII marker(runtime_);
    for (uint64_t index = 0, e = *lenRes; index < e; ++index) {
      tmpHandle = HermesValue::encodeDoubleValue(index);
      // Note that deleting elements doesn't affect array length.
      if (LLVM_UNLIKELY(
              filter(objHandle, tmpHandle) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      marker.flush();
    }
  } else if (auto scopedObject = Handle<JSObject>::dyn_vmcast(valHandle)) {
    auto cr = JSObject::getOwnPropertyNames(scopedObject, runtime_, true);
    if (cr == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto keys = *cr;
    GCScopeMarkerRAII marker(runtime_);
    for (uint32_t index = 0, e = keys->getEndIndex(); index < e; ++index) {
      tmpHandle = keys->at(runtime_, index).unboxToHV(runtime_);
      if (LLVM_UNLIKELY(
              filter(scopedObject, tmpHandle) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      marker.flush();
    }
  }
  // We have delayed converting the property to a string if it was index.
  // Now we have to do it because we are passing it to the reviver.
  tmpHandle = *property;
  auto strRes = toString_RJS(runtime_, tmpHandle);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = strRes->getHermesValue();

  return Callable::executeCall2(
             reviver_, runtime_, holder, *tmpHandle, *valHandle)
      .toCallResultHermesValue();
}

ExecutionStatus RuntimeJSONParser::filter(Handle<JSObject> val, Handle<> key) {
  auto jsonRes = operationWalk(val, key);
  if (LLVM_UNLIKELY(jsonRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newElement = runtime_.makeHandle(*jsonRes);
  if (newElement->isUndefined()) {
    if (LLVM_UNLIKELY(
            JSObject::deleteComputed(val, runtime_, key) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputed(
                val,
                runtime_,
                key,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                newElement) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> runtimeJSONParse(
    Runtime &runtime,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver) {
  // Our parser requires UTF16 data that does not move during GCs, so
  // in most cases we'll need to copy, except for external 16-bit strings.
  UTF16Ref ref;
  SmallU16String<32> storage;
  if (LLVM_UNLIKELY(jsonString->isExternal() && !jsonString->isASCII())) {
    ref = jsonString->getStringRef<char16_t>();
  } else {
    StringPrimitive::createStringView(runtime, jsonString)
        .appendUTF16String(storage);
    ref = storage;
  }

  RuntimeJSONParser parser{runtime, UTF16Stream(ref), reviver};
  return parser.parse();
}

CallResult<HermesValue> runtimeJSONParseRef(
    Runtime &runtime,
    UTF16Stream &&stream) {
  RuntimeJSONParser parser{
      runtime, std::move(stream), Runtime::makeNullHandle<Callable>()};
  return parser.parse();
}

ExecutionStatus JSONStringifyer::initializeReplacer(Handle<> replacer) {
  if (!vmisa<JSObject>(*replacer))
    return ExecutionStatus::RETURNED;
  // replacer is an object.

  if ((replacerFunction_ = dyn_vmcast<Callable>(*replacer)))
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

  CallResult<uint64_t> lenRes = getArrayLikeLength(replacerArray, runtime_);
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
  propertyList_ = arrRes->get();

  // Iterate through all indexes, in ascending order.
  GCScope gcScope{runtime_};
  auto marker = gcScope.createMarker();
  for (uint64_t i = 0, e = *lenRes; i < e; ++i) {
    gcScope.flushToMarker(marker);

    // Get the property value.
    tmpHandle_ = HermesValue::encodeDoubleValue(i);
    auto propRes =
        JSObject::getComputed_RJS(replacerArray, runtime_, tmpHandle_);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    PseudoHandle<> v = std::move(*propRes);
    // Convert v to string and store into item, if v is string, number, JSString
    // or JSNumber.
    if (v->isString()) {
      tmpHandle_ = std::move(v);
    } else if (
        v->isNumber() || vmisa<JSNumber>(v.get()) || vmisa<JSString>(v.get())) {
      tmpHandle_ = std::move(v);
      auto strRes = toString_RJS(runtime_, tmpHandle_);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      tmpHandle_ = strRes->getHermesValue();
    } else {
      tmpHandle_ = HermesValue::encodeUndefinedValue();
    }

    if (tmpHandle_->isUndefined())
      continue;
    // We only add item to propertyList if item is not already an element.
    bool exists = false;
    auto len = propertyList_->getEndIndex();
    for (uint32_t i = 0; i < len; ++i) {
      if (propertyList_->at(runtime_, i)
              .getString(runtime_)
              ->equals(tmpHandle_->getString())) {
        exists = true;
        break;
      }
    }
    if (!exists) {
      JSArray::setElementAt(propertyList_, runtime_, len, tmpHandle_);
    }
  }
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSONStringifyer::initializeSpace(Handle<> space) {
  tmpHandle_ = *space;
  if (vmisa<JSNumber>(*tmpHandle_)) {
    auto numRes = toNumber_RJS(runtime_, tmpHandle_);
    if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    tmpHandle_ = *numRes;
  } else if (vmisa<JSString>(*tmpHandle_)) {
    auto strRes = toString_RJS(runtime_, tmpHandle_);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    tmpHandle_ = strRes->getHermesValue();
  }
  if (tmpHandle_->isNumber()) {
    auto intRes = toIntegerOrInfinity(runtime_, tmpHandle_);
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
      gap_ = strRes->getString();
    }
  } else if (auto str = Handle<StringPrimitive>::dyn_vmcast(tmpHandle_)) {
    if (str->getStringLength() > 10) {
      auto strRes = StringPrimitive::slice(runtime_, str, 0, 10);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      gap_ = strRes->getString();
    } else if (str->getStringLength() > 0) {
      // If the string is empty, we don't set the gap.
      gap_ = str.get();
    }
  }
  return ExecutionStatus::RETURNED;
}

CallResult<bool> JSONStringifyer::operationStr(HermesValue key) {
  GCScopeMarkerRAII marker{runtime_};
  tmpHandle_ = key;

  // Str.1: access holder[key].
  auto propRes =
      JSObject::getComputed_RJS(operationStrHolder_, runtime_, tmpHandle_);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  operationStrValue_.set(propRes->get());

  // Str.2. If Type(value) is Object or BigInt, then
  MutableHandle<> hValueHV{runtime_, *operationStrValue_};
  if (vmisa<BigIntPrimitive>(*operationStrValue_)) {
    CallResult<HermesValue> hObjRes = toObject(runtime_, hValueHV);
    if (LLVM_UNLIKELY(hObjRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    assert(vmisa<JSBigInt>(*hObjRes) && "if not boxed bigint, then what?!");
    assert(vmisa<JSObject>(*hObjRes) && "if not object, then what?!");
    hValueHV = std::move(*hObjRes);
  }

  if (auto valueObj = Handle<JSObject>::dyn_vmcast(hValueHV)) {
    // Str.2.
    // Str.2.a: check if toJSON exists in value.
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getNamedWithReceiver_RJS(
                 valueObj,
                 runtime_,
                 Predefined::getSymbolID(Predefined::toJSON),
                 operationStrValue_)) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // Str.2.b: check if toJSON is a Callable.
    if (auto toJSON = Handle<Callable>::dyn_vmcast(
            runtime_.makeHandle(std::move(*propRes)))) {
      if (!tmpHandle_->isString()) {
        // Lazily convert key to a string.
        auto status = toString_RJS(runtime_, tmpHandle_);
        assert(
            status != ExecutionStatus::EXCEPTION &&
            "toString on a property cannot fail");
        tmpHandle_ = status->getHermesValue();
      }
      // Call toJSON with key as argument, value as this.
      auto callRes = Callable::executeCall1(
          toJSON, runtime_, operationStrValue_, *tmpHandle_);
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      operationStrValue_ = std::move(*callRes);
    }
  }

  // Str.3.
  if (replacerFunction_) {
    // Str.3.a.
    if (!tmpHandle_->isString()) {
      // Lazily convert key to a string.
      auto status = toString_RJS(runtime_, tmpHandle_);
      assert(
          status != ExecutionStatus::EXCEPTION &&
          "toString on a property cannot fail");
      tmpHandle_ = status->getHermesValue();
    }
    // If ReplacerFunction exists, call it with key and value as argument,
    // holder as this.
    auto callRes = Callable::executeCall2(
        replacerFunction_,
        runtime_,
        operationStrHolder_,
        *tmpHandle_,
        *operationStrValue_);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    operationStrValue_ = std::move(*callRes);
  }

  // Str.4: unbox value if necessary.
  // If Type(value) is Object, then
  if (vmisa<JSNumber>(*operationStrValue_)) {
    //  If value has a [[NumberData]] internal slot, then
    //      Set value to ? ToNumber(value).
    auto numRes = toNumber_RJS(runtime_, operationStrValue_);
    if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    operationStrValue_ = *numRes;
  } else if (vmisa<JSString>(*operationStrValue_)) {
    //  Else if value has a [[StringData]] internal slot, then
    //      Set value to ? ToString(value).
    auto strRes = toString_RJS(runtime_, operationStrValue_);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    operationStrValue_ = strRes->getHermesValue();
  } else if (auto *jsBool = dyn_vmcast<JSBoolean>(*operationStrValue_)) {
    //  Else if value has a [[BooleanData]] internal slot, then
    //      Set value to value.[[BooleanData]].
    operationStrValue_ =
        HermesValue::encodeBoolValue(jsBool->getPrimitiveBoolean());
  } else if (auto jsBigInt = dyn_vmcast<JSBigInt>(*operationStrValue_)) {
    //  Else if value has a [[BigIntData]] internal slot, then
    //      Set value to value.[[BigIntData]]
    BigIntPrimitive *bigintData =
        JSBigInt::getPrimitiveBigInt(jsBigInt, runtime_);
    operationStrValue_ = HermesValue::encodeBigIntValue(bigintData);
  }

  // Str.5.
  if (operationStrValue_->isNull()) {
    appendToOutput(Predefined::getSymbolID(Predefined::null));
    return true;
  }

  if (operationStrValue_->isBool()) {
    if (operationStrValue_->getBool()) {
      // Str.6.
      appendToOutput(Predefined::getSymbolID(Predefined::trueStr));
    } else {
      // Str.7.
      appendToOutput(Predefined::getSymbolID(Predefined::falseStr));
    }
    return true;
  }

  // Str.8.
  if (operationStrValue_->isString()) {
    operationQuote(StringPrimitive::createStringView(
        runtime_, Handle<StringPrimitive>::vmcast(operationStrValue_)));
    return true;
  }

  // Str.9.
  if (operationStrValue_->isNumber()) {
    if (std::isfinite(operationStrValue_->getNumber())) {
      auto status = toString_RJS(runtime_, operationStrValue_);
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
  if (vmisa<BigIntPrimitive>(*operationStrValue_)) {
    return runtime_.raiseTypeError("Do not know how to serialize a BigInt");
  }

  // Str.11.
  if (vmisa<JSObject>(*operationStrValue_) &&
      !vmisa<Callable>(*operationStrValue_)) {
    auto cr = pushValueToStack(*operationStrValue_);

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
        isArray(runtime_, vmcast<JSObject>(*operationStrValue_));
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
  quoteStringForJSON(output_, value);
}

ExecutionStatus JSONStringifyer::operationJA() {
  GCScopeMarkerRAII marker{runtime_};

  // JA.3.
  auto stepBack = depthCount_;
  // JA.4.
  if (depthCount_ + 1 >= MAX_RECURSION_DEPTH) {
    return runtime_.raiseStackOverflow(
        Runtime::StackOverflowKind::JSONStringify);
  }
  depthCount_++;
  output_.push_back(u'[');
  CallResult<uint64_t> lenRes = getArrayLikeLength(
      runtime_.makeHandle(vmcast<JSObject>(
          stackValue_->at(stackValue_->size() - 1).getObject(runtime_))),
      runtime_);
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
    operationStrHolder_ = vmcast<JSObject>(
        stackValue_->at(stackValue_->size() - 1).getObject(runtime_));
    // Flush just before the recursion in case any handles were created.
    marker.flush();
    auto status = operationStr(HermesValue::encodeDoubleValue(index));
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
  if (depthCount_ + 1 >= MAX_RECURSION_DEPTH) {
    return runtime_.raiseStackOverflow(
        Runtime::StackOverflowKind::JSONStringify);
  }
  depthCount_++;
  output_.push_back(u'{');
  auto beginningLoc = output_.size();
  indent();

  if (propertyList_) {
    // JO.5.
    operationJOK_ = propertyList_.get();
  } else {
    // JO.6.
    tmpHandle_ = HermesValue::encodeObjectValue(
        stackValue_->at(stackValue_->size() - 1).getObject(runtime_));
    if (LLVM_LIKELY(!Handle<JSObject>::vmcast(tmpHandle_)->isProxyObject())) {
      // enumerableOwnProperties_RJS is the spec definition, and is
      // used below on proxies so the correct traps get called.  In
      // the common case of a non-proxy object, we can do less work by
      // calling getOwnPropertyNames.
      auto cr = JSObject::getOwnPropertyNames(
          Handle<JSObject>::vmcast(tmpHandle_), runtime_, true);
      if (cr == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      operationJOK_ = **cr;
    } else {
      CallResult<HermesValue> ownPropRes = enumerableOwnProperties_RJS(
          runtime_,
          Handle<JSObject>::vmcast(tmpHandle_),
          EnumerableOwnPropertiesKind::Key);
      if (ownPropRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      operationJOK_ = vmcast<JSArray>(*ownPropRes);
    }
  }

  marker.flush();

  // JO.8.
  bool hasElement = false;
  for (uint32_t index = 0, len = operationJOK_->getEndIndex(); index < len;
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

    tmpHandle_ = operationJOK_->at(runtime_, index).unboxToHV(runtime_);
    if (LLVM_UNLIKELY(!tmpHandle_->isString())) {
      // property may come from getOwnPropertyNames, which may contain numbers.
      // getOwnPropertyNames and propertyList_ are both only populated
      // with strings, numbers, and undefined only.
      // None of them are objects, so toString cannot throw.
      assert(!tmpHandle_->isObject() && "property name is an object");
      auto status = toString_RJS(runtime_, tmpHandle_);
      assert(
          status != ExecutionStatus::EXCEPTION &&
          "toString on a property cannot fail");
      tmpHandle_ = status->getHermesValue();
    }
    // tmpHandle now contains property as string.
    // JO.8.b.i
    operationQuote(StringPrimitive::createStringView(
        runtime_, Handle<StringPrimitive>::vmcast(tmpHandle_)));
    // JO.8.b.ii
    output_.push_back(u':');
    // JO.8.b.iii
    if (gap_.get()) {
      output_.push_back(u' ');
    }

    // JO.9.a.
    operationStrHolder_ = vmcast<JSObject>(
        stackValue_->at(stackValue_->size() - 1).getObject(runtime_));

    tmpHandle2_ = operationJOK_.getHermesValue();
    if (PropStorage::push_back(stackJO_, runtime_, tmpHandle2_) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    // Flush just before recursion (propStoragePushBack may create handles).
    marker.flush();
    auto result = operationStr(*tmpHandle_);

    operationJOK_ =
        vmcast<JSArray>(stackJO_->pop_back(runtime_).getObject(runtime_));

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
  if (gap_.get()) {
    output_.push_back(u'\n');
    for (uint32_t i = 0; i < depthCount_; ++i) {
      appendToOutput(gap_.get());
    }
  }
}

CallResult<bool> JSONStringifyer::pushValueToStack(HermesValue value) {
  assert(vmisa<JSObject>(value) && "Can only push object to stack");

  for (uint32_t i = 0, len = stackValue_->size(); i < len; ++i) {
    if (stackValue_->at(i).getObject(runtime_) == value.getObject()) {
      return false;
    }
  }

  tmpHandle_ = value;
  if (PropStorage::push_back(stackValue_, runtime_, tmpHandle_) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return true;
}

void JSONStringifyer::popValueFromStack() {
  assert(stackValue_->size() && "Cannot pop from an empty stack");
  stackValue_->pop_back(runtime_);
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
  operationStrHolder_ = JSObject::create(runtime_).get();
  auto status = JSObject::defineOwnProperty(
      operationStrHolder_,
      runtime_,
      Predefined::getSymbolID(Predefined::emptyString),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      value);
  assert(
      status != ExecutionStatus::EXCEPTION && *status &&
      "defineOwnProperty on a newly created object cannot fail");
  (void)status;

  // Step 11 in ES5.1 15.12.3.
  status = operationStr(HermesValue::encodeStringValue(
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
