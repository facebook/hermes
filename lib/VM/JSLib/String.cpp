/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.5 Initialize the String constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/Platform/Unicode/PlatformUnicode.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringView.h"

#if defined(__ANDROID__)
#include "hermes/Platform/Unicode/PlatformUnicode.h"
#endif

#ifdef __APPLE__
#include <CoreFoundation/CFString.h>
#endif

#include <locale>

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
/// String.

Handle<JSObject> createStringConstructor(Runtime *runtime) {
  auto stringPrototype =
      Handle<PrimitiveBox>::vmcast(&runtime->stringPrototype);

  auto cons = defineSystemConstructor<JSString>(
      runtime,
      Predefined::getSymbolID(Predefined::String),
      stringConstructor,
      stringPrototype,
      1,
      CellKind::StringObjectKind);

  // String.prototype.xxx methods.
  void *ctx = nullptr;
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::toString),
      ctx,
      stringPrototypeToString,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::valueOf),
      ctx,
      stringPrototypeToString,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::charAt),
      ctx,
      stringPrototypeCharAt,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::charCodeAt),
      ctx,
      stringPrototypeCharCodeAt,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::codePointAt),
      ctx,
      stringPrototypeCodePointAt,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::concat),
      ctx,
      stringPrototypeConcat,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::endsWith),
      ctx,
      stringPrototypeEndsWith,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::slice),
      ctx,
      stringPrototypeSlice,
      2);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::split),
      ctx,
      stringPrototypeSplit,
      2);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::substring),
      ctx,
      stringPrototypeSubstring,
      2);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::toLowerCase),
      ctx,
      stringPrototypeToLowerCase,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::toLocaleLowerCase),
      ctx,
      stringPrototypeToLocaleLowerCase,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::toUpperCase),
      ctx,
      stringPrototypeToUpperCase,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::toLocaleUpperCase),
      ctx,
      stringPrototypeToLocaleUpperCase,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::substr),
      ctx,
      stringPrototypeSubstr,
      2);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::trim),
      ctx,
      stringPrototypeTrim,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::trimLeft),
      ctx,
      stringPrototypeTrimLeft,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::trimRight),
      ctx,
      stringPrototypeTrimRight,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::includes),
      (void *)false,
      stringPrototypeIncludesOrStartsWith,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::indexOf),
      ctx,
      stringPrototypeIndexOf,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::lastIndexOf),
      ctx,
      stringPrototypeLastIndexOf,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::localeCompare),
      ctx,
      stringPrototypeLocaleCompare,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::match),
      ctx,
      stringPrototypeMatch,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::normalize),
      ctx,
      stringPrototypeNormalize,
      0);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::padEnd),
      (void *)false,
      stringPrototypePad,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::padStart),
      (void *)true,
      stringPrototypePad,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::repeat),
      ctx,
      stringPrototypeRepeat,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::replace),
      ctx,
      stringPrototypeReplace,
      2);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::search),
      ctx,
      stringPrototypeSearch,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::startsWith),
      (void *)true,
      stringPrototypeIncludesOrStartsWith,
      1);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.enumerable = 0;
  (void)defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::SymbolIterator),
      Predefined::getSymbolID(Predefined::squareSymbolIterator),
      ctx,
      stringPrototypeSymbolIterator,
      0,
      dpf);

  // String.xxx() methods.
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::fromCharCode),
      ctx,
      stringFromCharCode,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::fromCodePoint),
      ctx,
      stringFromCodePoint,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::raw),
      ctx,
      stringRaw,
      1);

  return cons;
}

CallResult<HermesValue>
stringConstructor(void *, Runtime *runtime, NativeArgs args) {
  if (args.getArgCount() == 0) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }

  if (!args.isConstructorCall() && args.getArg(0).isSymbol()) {
    auto str = symbolDescriptiveString(
        runtime, Handle<SymbolID>::vmcast(args.getArgHandle(0)));
    if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return str->getHermesValue();
  }

  auto sRes = toString_RJS(runtime, args.getArgHandle(0));
  if (sRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto s = toHandle(runtime, std::move(*sRes));

  if (!args.isConstructorCall()) {
    // Not a constructor call, just return the string value.
    return s.getHermesValue();
  }

  // Constructor call: initialize the JSString.
  auto self = args.vmcastThis<JSString>();
  JSString::setPrimitiveString(self, runtime, s);

  return self.getHermesValue();
}

CallResult<HermesValue>
stringFromCharCode(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  uint32_t n = args.getArgCount();
  auto builder = StringBuilder::createStringBuilder(runtime, SafeUInt32{n});
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  for (unsigned i = 0; i < n; ++i) {
    // Call a function that may throw, let the runtime record it.
    auto res = toUInt16(runtime, args.getArgHandle(i));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    char16_t ch = res->getNumber();
    builder->appendCharacter(ch);
  }
  return HermesValue::encodeStringValue(*builder->getStringPrimitive());
}

CallResult<HermesValue>
stringFromCodePoint(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let codePoints be a List containing the arguments passed to this
  // function.
  // 2. Let length be the number of elements in codePoints.
  uint32_t length = args.getArgCount();
  // 3. Let elements be a new List.
  llvm::SmallVector<char16_t, 32> elements{};
  // 4. Let nextIndex be 0.
  uint32_t nextIndex = 0;

  MutableHandle<> next{runtime};
  MutableHandle<> nextCP{runtime};

  GCScopeMarkerRAII marker{gcScope};
  // 5. Repeat while nextIndex < length
  for (; nextIndex < length; marker.flush()) {
    marker.flush();
    // 5a. Let next be codePoints[nextIndex].
    next = args.getArg(nextIndex);
    // 5b. Let nextCP be toNumber_RJS(next).
    auto nextCPRes = toNumber_RJS(runtime, next);
    if (LLVM_UNLIKELY(nextCPRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nextCP = *nextCPRes;

    // 5d. If SameValue(nextCP, ToInteger(nextCP)) is false, throw a RangeError
    // exception.
    auto nextCPInt = toInteger(runtime, nextCP);
    if (LLVM_UNLIKELY(nextCPInt == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!isSameValue(*nextCP, *nextCPInt)) {
      return runtime->raiseRangeError(
          TwineChar16("Code point must be an integer: ") + nextCP->getNumber());
    }

    // 5e. If nextCP < 0 or nextCP > 0x10FFFF, throw a RangeError exception.
    if (nextCP->getNumber() < 0 || nextCP->getNumber() > 0x10FFFF) {
      return runtime->raiseRangeError(
          TwineChar16("Code point out of bounds: ") + nextCP->getNumber());
    }

    // 5f. Append the elements of the UTF16Encoding (10.1.1) of nextCP to the
    // end of elements.
    // Safe to get as uint32_t because we've done int and bounds checking.
    utf16Encoding(nextCP->getNumberAs<uint32_t>(), elements);

    // 5g. Let nextIndex be nextIndex + 1.
    ++nextIndex;
  }

  // 6. Return the String value whose elements are, in order, the elements in
  // the List elements. If length is 0, the empty string is returned.
  return StringPrimitive::createEfficient(runtime, elements);
}

/// ES6.0 21.1.2.4 String.raw ( template , ...substitutions )
CallResult<HermesValue> stringRaw(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let substitutions be a List consisting of all of the arguments passed to
  // this function, starting with the second argument.
  // If fewer than two arguments were passed, the List is empty.
  // 2. Let numberOfSubstitutions be the number of elements in substitutions.
  uint32_t numberOfSubstitutions =
      args.getArgCount() < 2 ? 0 : args.getArgCount() - 1;

  // 3. Let cooked be ToObject(template).
  auto cookedRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(cookedRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto cooked = runtime->makeHandle<JSObject>(*cookedRes);

  // 5. Let raw be ToObject(Get(cooked, "raw")).
  auto getRes = JSObject::getNamed_RJS(
      cooked, runtime, Predefined::getSymbolID(Predefined::raw));
  if (LLVM_UNLIKELY(getRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto rawRes = toObject(runtime, runtime->makeHandle(*getRes));
  if (LLVM_UNLIKELY(rawRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto raw = runtime->makeHandle<JSObject>(*rawRes);

  // 7. Let literalSegments be ToLength(Get(raw, "length"))
  auto lengthRes = JSObject::getNamed_RJS(
      raw, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto literalSegmentsRes = toLength(runtime, runtime->makeHandle(*lengthRes));
  if (LLVM_UNLIKELY(literalSegmentsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  int64_t literalSegments = literalSegmentsRes->getNumberAs<int64_t>();
  // 9. If literalSegments ≤ 0, return the empty string.
  if (literalSegments <= 0) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }

  // 10. Let stringElements be a new List.
  llvm::SmallVector<char16_t, 32> stringElements{};

  // 11. Let nextIndex be 0.
  MutableHandle<> nextIndex{runtime, HermesValue::encodeNumberValue(0)};

  MutableHandle<> tmpHandle{runtime};
  MutableHandle<StringPrimitive> nextSeg{runtime};
  MutableHandle<> next{runtime};
  MutableHandle<StringPrimitive> nextSub{runtime};

  // 12. Repeat
  GCScopeMarkerRAII marker{gcScope};
  for (;; marker.flush()) {
    // 12. a. Let nextKey be ToString(nextIndex).
    // 12. b. Let nextSeg be ToString(Get(raw, nextKey)).
    auto nextSegPropRes = JSObject::getComputed_RJS(raw, runtime, nextIndex);
    if (LLVM_UNLIKELY(nextSegPropRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    tmpHandle = *nextSegPropRes;
    auto nextSegRes = toString_RJS(runtime, tmpHandle);
    if (LLVM_UNLIKELY(nextSegRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nextSeg = nextSegRes->get();

    // 12. d. Append in order the code unit elements of nextSeg to the end of
    // stringElements.
    nextSeg->copyUTF16String(stringElements);

    // 12. e. If nextIndex + 1 = literalSegments, then
    if (nextIndex->getNumberAs<int64_t>() + 1 == literalSegments) {
      // 12. i. Return the String value whose code units are, in order, the
      // elements in the List stringElements. If stringElements has no elements,
      // the empty string is returned.
      return StringPrimitive::createEfficient(runtime, stringElements);
    }

    if (nextIndex->getNumberAs<int64_t>() < numberOfSubstitutions) {
      // 12. f. If nextIndex < numberOfSubstitutions, let next be
      // substitutions[nextIndex].
      // Add one to nextIndex to get index in substitutions.
      next = args.getArg(nextIndex->getNumberAs<int64_t>() + 1);
      // 12. h. Let nextSub be ToString(next).
      auto nextSubRes = toString_RJS(runtime, next);
      if (LLVM_UNLIKELY(nextSubRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      nextSub = nextSubRes->get();
      // 12. j. Append in order the code unit elements of nextSub to the end of
      // stringElements.
      nextSub->copyUTF16String(stringElements);
    }

    // 12. g. Else, let next be the empty String.
    // Omitted because nothing happens.

    // 12. k. Let nextIndex be nextIndex + 1.
    nextIndex =
        HermesValue::encodeNumberValue(nextIndex->getNumberAs<int64_t>() + 1);
  }
}

//===----------------------------------------------------------------------===//
/// String.prototype.

CallResult<HermesValue>
stringPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  if (args.getThisArg().isString()) {
    return args.getThisArg();
  }

  // Not a String value, must be a string object.
  auto *strPtr = dyn_vmcast<JSString>(args.getThisArg());
  if (strPtr) {
    // Only return the string if called on a String object.
    return JSString::getPrimitiveValue(strPtr, runtime);
  }
  return runtime->raiseTypeError(
      "String.prototype.toString() called on non-string object");
}

CallResult<HermesValue>
stringPrototypeCharAt(void *, Runtime *runtime, NativeArgs args) {
  Handle<> thisValue{&args.getThisArg()};
  // Call a function that may throw, let the runtime record it.
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  auto intRes = toInteger(runtime, runtime->makeHandle(args.getArg(0)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto position = intRes->getNumber();
  auto size = S->getStringLength();
  if (position < 0 || position >= size) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }
  auto result = runtime->getCharacterString(
      StringPrimitive::createStringView(runtime, S)[position]);
  return HermesValue::encodeStringValue(result.get());
}

CallResult<HermesValue>
stringPrototypeCharCodeAt(void *, Runtime *runtime, NativeArgs args) {
  Handle<> thisValue{&args.getThisArg()};
  // Call a function that may throw, let the runtime record it.
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  auto intRes = toInteger(runtime, runtime->makeHandle(args.getArg(0)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto position = intRes->getNumber();
  auto size = S->getStringLength();
  if (position < 0 || position >= size) {
    return HermesValue::encodeNaNValue();
  }
  return HermesValue::encodeDoubleValue(
      StringPrimitive::createStringView(runtime, S)[position]);
}

CallResult<HermesValue>
stringPrototypeCodePointAt(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 2. Let S be ToString(O).
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  // 4. Let position be ToInteger(pos).
  auto positionRes = toInteger(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(positionRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double position = positionRes->getNumber();

  // 6. Let size be the number of elements in S.
  double size = S->getStringLength();

  // 7. If position < 0 or position ≥ size, return undefined.
  if (position < 0 || position >= size) {
    return HermesValue::encodeUndefinedValue();
  }

  auto strView = StringPrimitive::createStringView(runtime, S);

  // 8. Let first be the code unit value of the element at index position in the
  // String S.
  char16_t first = strView[position];

  // 9. If first < 0xD800 or first > 0xDBFF or position+1 = size, return first.
  if (first < 0xD800 || first > 0xDBFF || position + 1 == size) {
    return HermesValue::encodeNumberValue(first);
  }

  // 10. Let second be the code unit value of the element at index position+1 in
  // the String S.
  // Safe to access because we ensured that position + 1 < size.
  char16_t second = strView[position + 1];

  // 11. If second < 0xDC00 or second > 0xDFFF, return first.
  if (second < 0xDC00 || second > 0xDFFF) {
    return HermesValue::encodeNumberValue(first);
  }

  // 12. Return UTF16Decode(first, second).
  return HermesValue::encodeNumberValue(utf16Decode(first, second));
}

CallResult<HermesValue>
stringPrototypeConcat(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  // Track the total characters in the result.
  SafeUInt32 size(S->getStringLength());
  uint32_t argCount = args.getArgCount();

  // Store the results of toStrings and concat them at the end.
  auto arrRes = ArrayStorage::create(runtime, argCount, argCount);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strings = runtime->makeHandle<ArrayStorage>(*arrRes);

  // Run toString on the arguments to figure out the final size.
  auto marker = gcScope.createMarker();
  for (uint32_t i = 0; i < argCount; ++i) {
    auto strRes = toString_RJS(runtime, args.getArgHandle(i));
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // Allocations can't be performed here,
    // and we know we're in bounds because we preallocated.
    strings->at(i).set(strRes->getHermesValue(), &runtime->getHeap());
    uint32_t strLength = strRes->get()->getStringLength();

    size.add(strLength);
    if (LLVM_UNLIKELY(size.isOverflowed())) {
      return runtime->raiseRangeError("resulting string length exceeds limit");
    }

    gcScope.flushToMarker(marker);
  }

  // Allocate the complete result.
  auto builder = StringBuilder::createStringBuilder(runtime, size);
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Copy 'this' argument first.
  builder->appendStringPrim(S);
  MutableHandle<StringPrimitive> element{runtime};

  // Copy the rest of the strings.
  for (uint32_t i = 0; i < argCount; i++) {
    element = strings->at(i).getString();
    builder->appendStringPrim(element);
  }
  return builder->getStringPrimitive().getHermesValue();
}

CallResult<HermesValue>
stringPrototypeSlice(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  double len = S->getStringLength();

  auto intRes = toInteger(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double intStart = intRes->getNumber();

  double intEnd;
  if (args.getArg(1).isUndefined()) {
    intEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toInteger(runtime, args.getArgHandle(1))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    intEnd = intRes->getNumber();
  }

  size_t from =
      intStart < 0 ? std::max(len + intStart, 0.0) : std::min(intStart, len);
  size_t to = intEnd < 0 ? std::max(len + intEnd, 0.0) : std::min(intEnd, len);
  size_t span = to > from ? to - from : 0;
  assert(from + span <= len && "invalid index computed in slice");

  return StringPrimitive::slice(runtime, S, from, span);
}

/// Works slightly differently from the given implementation in the spec.
/// Given a string \p S and a starting point \p q, finds the first match of
/// \p R such that it starts on or after index \p q in \p S.
/// \param q starting point in S. Requires: q <= S->getStringLength().
/// \param R a RegExp or a String.
static CallResult<RegExpMatch> splitMatch(
    Runtime *runtime,
    Handle<StringPrimitive> S,
    uint32_t q,
    Handle<> R) {
  if (auto regexp = Handle<JSRegExp>::dyn_vmcast(R)) {
    return JSRegExp::search(regexp, runtime, S, q);
  }

  // Not searching for a RegExp, manually do string matching.
  auto RHandle = Handle<StringPrimitive>::dyn_vmcast(R);
  assert(RHandle && "SplitMatch() called without RegExp or String");

  auto SStr = StringPrimitive::createStringView(runtime, S);
  auto RStr = StringPrimitive::createStringView(runtime, RHandle);
  RegExpMatch match{};

  // Handle empty string separately.
  if (SStr.empty()) {
    if (RStr.empty()) {
      match.push_back({{0, 0}});
    }
    return match;
  }

  auto sliced = SStr.slice(q);
  auto searchResult =
      std::search(sliced.begin(), sliced.end(), RStr.begin(), RStr.end());

  if (searchResult != sliced.end()) {
    uint32_t i = q + (searchResult - sliced.begin());
    match.push_back({{i, RHandle->getStringLength()}});
  }
  return match;
}

// TODO: implement this following ES6 21.2.5.11.
CallResult<HermesValue> splitInternal(
    Runtime *runtime,
    Handle<> string,
    Handle<> limit,
    Handle<> separator) {
  auto strRes = toString_RJS(runtime, string);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  // Final array.
  auto arrRes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto A = toHandle(runtime, std::move(*arrRes));
  uint32_t lengthA = 0;

  // Limit on the number of items allowed in the result array.
  uint32_t lim;
  if (limit->isUndefined()) {
    // No limit supplied, make it max.
    lim = 0xffffffff; // 2 ^ 32 - 1
  } else {
    auto intRes = toUInt32_RJS(runtime, limit);
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lim = intRes->getNumber();
  }

  // The pattern which we want to separate on. Can be a JSRegExp or a string.
  // TODO: implement sticky ('y') flag behavior.
  bool unicodeMatching = false;
  MutableHandle<> R{runtime};
  if (vmisa<JSRegExp>(separator.get())) {
    R = separator.get();
    unicodeMatching =
        JSRegExp::getFlagBits(vmcast<JSRegExp>(separator.get())).unicode;
  } else {
    auto strRes = toString_RJS(runtime, separator);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    R = strRes->getHermesValue();
  }

  if (lim == 0) {
    // Don't want any elements, so we're done.
    return A.getHermesValue();
  }

  // If there's no separator, then don't split the string and return.
  if (LLVM_UNLIKELY(separator->isUndefined())) {
    (void)JSArray::setElementAt(A, runtime, 0, S);
    if (LLVM_UNLIKELY(
            JSArray::setLengthProperty(A, runtime, 1) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    return A.getHermesValue();
  }

  uint32_t s = S->getStringLength();

  if (s == 0) {
    // S is the empty string.
    auto matchResult = splitMatch(runtime, S, 0, R);
    if (LLVM_UNLIKELY(matchResult == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    } else if (!matchResult->empty()) {
      // Matched the entirety of S, so return the empty array.
      return A.getHermesValue();
    }
    // Didn't match S, so add it to the array and return.
    (void)JSArray::setElementAt(A, runtime, 0, S);
    if (LLVM_UNLIKELY(
            JSArray::setLengthProperty(A, runtime, 1) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    return A.getHermesValue();
  }

  // End of the last match.
  uint32_t p = 0;
  // Place to attempt the start of the next match.
  uint32_t q = p;

  GCScopeMarkerRAII gcMarker{runtime};

  // Main loop: continue while we have space to find another match.
  while (q < s) {
    gcMarker.flush();

    // Find the next valid match. We know that q < s.
    // ES5.1's SplitMatch only finds matches at q, but we find matches at or
    // after q, so if it fails, we know we're done.
    auto matchResult = splitMatch(runtime, S, q, R);

    if (LLVM_UNLIKELY(matchResult == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;

    auto match = *matchResult;

    if (match.empty()) {
      // There's no matches at or after index q, so we're done searching.
      // Note: This behavior differs from the spec implementation,
      // because we check for matches at or after q.
      break;
    }
    // Found a match, so go ahead and update q and e,
    // such that the match is the range [q,e).
    q = match[0]->location;
    uint32_t e = q + match[0]->length;
    if (e == p) {
      // The end of this match is the same as the end of the last match,
      // so we matched with the empty string.
      // We don't want to match the empty string at this location again,
      // so increment q in order to start the next search at the next position.
      // ES6 21.2.5.11:
      // If e = p, let q be AdvanceStringIndex(S, q, unicodeMatching).
      q = advanceStringIndex(S.get(), q, unicodeMatching);
    } else {
      // Found a non-empty string match.
      // Add everything from the last match to the current one to A.
      // This has length q-p because q is the start of the current match,
      // and p was the end (exclusive) of the last match.
      auto strRes = StringPrimitive::slice(runtime, S, p, q - p);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      JSArray::setElementAt(
          A, runtime, lengthA, runtime->makeHandle<StringPrimitive>(*strRes));
      ++lengthA;
      if (lengthA == lim) {
        // Reached the limit, return early.
        if (LLVM_UNLIKELY(
                JSArray::setLengthProperty(A, runtime, lengthA) ==
                ExecutionStatus::EXCEPTION))
          return ExecutionStatus::EXCEPTION;
        return A.getHermesValue();
      }
      // Update p to point to the end of this match, maintaining the
      // invariant that it points to the end of the last match encountered.
      p = e;
      // Add all the capture groups to A. Start at i=1 to skip the full match.
      for (uint32_t i = 1, m = match.size(); i < m; ++i) {
        const auto &range = match[i];
        if (!range) {
          JSArray::setElementAt(
              A, runtime, lengthA, Runtime::getUndefinedValue());
        } else {
          if (LLVM_UNLIKELY(
                  (strRes = StringPrimitive::slice(
                       runtime, S, range->location, range->length)) ==
                  ExecutionStatus::EXCEPTION)) {
            return ExecutionStatus::EXCEPTION;
          }
          JSArray::setElementAt(
              A,
              runtime,
              lengthA,
              runtime->makeHandle<StringPrimitive>(*strRes));
        }
        ++lengthA;
        if (lengthA == lim) {
          // Reached the limit, return early.
          if (LLVM_UNLIKELY(
                  JSArray::setLengthProperty(A, runtime, lengthA) ==
                  ExecutionStatus::EXCEPTION))
            return ExecutionStatus::EXCEPTION;
          return A.getHermesValue();
        }
      }
      // Start position of the next search is updated to the end of this match.
      q = p;
    }
  }

  // Add the rest of the string (after the last match) to A.
  auto elementStrRes = StringPrimitive::slice(runtime, S, p, s - p);
  if (LLVM_UNLIKELY(elementStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  JSArray::setElementAt(
      A,
      runtime,
      lengthA,
      runtime->makeHandle<StringPrimitive>(*elementStrRes));
  ++lengthA;

  if (LLVM_UNLIKELY(
          JSArray::setLengthProperty(A, runtime, lengthA) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return A.getHermesValue();
}

CallResult<HermesValue>
stringPrototypeSplit(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  // 2. ReturnIfAbrupt(O).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 3. If separator is neither undefined nor null, then
  auto separator = args.getArgHandle(0);
  if (!separator->isUndefined() && !separator->isNull()) {
    // a. Let splitter be GetMethod(separator, @@split).
    auto methodRes = getMethod(
        runtime,
        separator,
        runtime->makeHandle(Predefined::getSymbolID(Predefined::SymbolSplit)));
    // b. ReturnIfAbrupt(splitter).
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If splitter is not undefined, then
    //   i. Return Call(splitter, separator, «‍O, limit»).
    if (!methodRes->getHermesValue().isUndefined()) {
      Handle<Callable> splitter =
          Handle<Callable>::vmcast(runtime, methodRes->getHermesValue());
      return Callable::executeCall2(
          splitter, runtime, separator, O.getHermesValue(), args.getArg(1));
    }
  }
  return splitInternal(
      runtime,
      args.getThisHandle(),
      args.getArgHandle(1),
      args.getArgHandle(0));
}

CallResult<HermesValue>
stringPrototypeSubstring(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  double len = S->getStringLength();

  auto intRes = toInteger(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double intStart = intRes->getNumber();

  double intEnd;
  if (args.getArg(1).isUndefined()) {
    intEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toInteger(runtime, args.getArgHandle(1))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    intEnd = intRes->getNumber();
  }

  size_t finalStart = std::min(std::max(intStart, 0.0), len);
  size_t finalEnd = std::min(std::max(intEnd, 0.0), len);
  size_t from = std::min(finalStart, finalEnd);
  size_t to = std::max(finalStart, finalEnd);

  return StringPrimitive::slice(runtime, S, from, to > from ? to - from : 0);
}

static CallResult<HermesValue> convertCase(
    Runtime *runtime,
    Handle<StringPrimitive> S,
    const bool upperCase,
    const bool useCurrentLocale) {
  // Copying is unavoidable in this function, do it early on.
  SmallU16String<32> buff;
  // Must copy instead of just getting the reference, because later operations
  // may trigger GC and hence invalid pointers inside S.
  S->copyUTF16String(buff);
  UTF16Ref str = buff.arrayRef();

  if (!useCurrentLocale) {
    // Try a fast path for ASCII strings.
    // First, bitwise-or all the characters to see if any one isn't ASCII.
    char16_t mask = 0;
    // Also, check if we have to do work or we can just return S directly.
    bool noop = true;
    if (upperCase) {
      for (const auto c : str) {
        mask |= c;
        // It's still a noop if the character isn't a lowercase ASCII.
        noop &= !('a' <= c && c <= 'z');
      }
    } else {
      for (const auto c : str) {
        mask |= c;
        // It's still a noop if the character isn't an uppercase ASCII.
        noop &= !('A' <= c && c <= 'Z');
      }
    }
    if (mask <= 127) {
      if (noop) {
        // We don't have to allocate anything.
        return S.getHermesValue();
      }

      if (str.size() == 1) {
        // Use the Runtime stored representations of single-character strings.
        char16_t c = str[0];
        if (upperCase) {
          char16_t isLower = 'a' <= c && c <= 'z';
          return runtime->getCharacterString(c & ~(isLower << 5))
              .getHermesValue();
        } else {
          char16_t isUpper = 'A' <= c && c <= 'Z';
          return runtime->getCharacterString(c | (isUpper << 5))
              .getHermesValue();
        }
      }

      SafeUInt32 len(S->getStringLength());
      auto builder = StringBuilder::createStringBuilder(runtime, len);
      if (builder == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      char16_t ch;
      if (upperCase) {
        for (const char16_t c : str) {
          // If it is lower, then clear the 5th bit, else do nothing.
          char16_t isLower = 'a' <= c && c <= 'z';
          ch = c & ~(isLower << 5);
          builder->appendCharacter(ch);
        }
      } else {
        for (const char16_t c : str) {
          // If it is upper, then set the 5th bit, else do nothing.
          char16_t isUpper = 'A' <= c && c <= 'Z';
          ch = c | (isUpper << 5);
          builder->appendCharacter(ch);
        }
      }
      return HermesValue::encodeStringValue(*builder->getStringPrimitive());
    }
  }
  platform_unicode::convertToCase(
      buff,
      upperCase ? platform_unicode::CaseConversion::ToUpper
                : platform_unicode::CaseConversion::ToLower,
      useCurrentLocale);
  return StringPrimitive::create(runtime, buff);
}

CallResult<HermesValue>
stringPrototypeToLowerCase(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, toHandle(runtime, std::move(*res)), false, false);
}

CallResult<HermesValue>
stringPrototypeToLocaleLowerCase(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, toHandle(runtime, std::move(*res)), false, true);
}

CallResult<HermesValue>
stringPrototypeToUpperCase(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, toHandle(runtime, std::move(*res)), true, false);
}

CallResult<HermesValue>
stringPrototypeToLocaleUpperCase(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, toHandle(runtime, std::move(*res)), true, true);
}
CallResult<HermesValue>
stringPrototypeSubstr(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  double stringLen = S->getStringLength();

  auto intRes = toInteger(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double start = intRes->getNumber();

  double length;
  if (args.getArg(1).isUndefined()) {
    length = stringLen;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toInteger(runtime, args.getArgHandle(1))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    length = intRes->getNumber();
  }

  if (start < 0) {
    start = std::max(stringLen + start, 0.0);
  }
  double adjustedLength = std::min(std::max(length, 0.0), stringLen - start);
  if (adjustedLength <= 0) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  } else {
    return StringPrimitive::slice(
        runtime,
        S,
        static_cast<size_t>(start),
        static_cast<size_t>(adjustedLength));
  }
}

/// \return the number of characters to trim from the begin iterator.
static size_t trimLeft(
    StringView::const_iterator begin,
    StringView::const_iterator end) {
  size_t toTrim = 0;
  while (begin != end &&
         (isWhiteSpaceChar(*begin) || isLineTerminatorChar(*begin))) {
    ++begin;
    ++toTrim;
  }
  return toTrim;
}

/// \return the number of characters to trim from the end iterator.
static size_t trimRight(
    StringView::const_iterator begin,
    StringView::const_iterator end) {
  size_t toTrim = 0;
  while (begin != end &&
         (isWhiteSpaceChar(*(end - 1)) || isLineTerminatorChar(*(end - 1)))) {
    --end;
    ++toTrim;
  }
  return toTrim;
}

CallResult<HermesValue>
stringPrototypeTrim(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t beginIdx = 0, endIdx = S->getStringLength();
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    beginIdx = trimLeft(begin, end);
    begin += beginIdx;
    endIdx -= trimRight(begin, end);
  }

  return StringPrimitive::slice(runtime, S, beginIdx, endIdx - beginIdx);
}

CallResult<HermesValue>
stringPrototypeTrimLeft(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t beginIdx = 0;
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    beginIdx = trimLeft(begin, end);
  }

  return StringPrimitive::slice(
      runtime, S, beginIdx, S->getStringLength() - beginIdx);
}

CallResult<HermesValue>
stringPrototypeTrimRight(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t endIdx = S->getStringLength();
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    endIdx -= trimRight(begin, end);
  }

  return StringPrimitive::slice(runtime, S, 0, endIdx);
}

/// Shared implementation of string.indexOf and string.lastIndexOf
/// Given a haystack ('this'), needle, and position, return the index
/// of the first (reverse=false) or last (reverse=true) substring match of
/// needle within haystack that is not smaller (normal) or larger (reverse) than
/// position.
/// This provides an implementation of ES5.1 15.5.4.7 (reverse=false),
/// and ES5.1 15.5.4.8 (reverse=true)
/// \param runtime  the runtime to use for argument coercions
/// \param args     the arguments passed to indexOf / lastIndexOf
/// \param reverse  whether we are running lastIndexOf (true) or indexOf (false)
/// \returns        Hermes-encoded index of the substring match, or -1 on
///                 failure
static CallResult<HermesValue>
stringDirectedIndexOf(Runtime *runtime, NativeArgs args, bool reverse) {
  auto thisValue = args.getThisHandle();
  // Call a function that may throw, let the runtime record it.
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  auto searchStrRes = toString_RJS(runtime, args.getArgHandle(0));
  if (searchStrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = toHandle(runtime, std::move(*searchStrRes));

  double pos;
  if (reverse) {
    auto intRes = toNumber_RJS(runtime, runtime->makeHandle(args.getArg(1)));
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    Handle<> numPos = runtime->makeHandle(intRes.getValue());
    if (std::isnan(numPos->getNumber())) {
      pos = std::numeric_limits<double>::infinity();
    } else {
      if (LLVM_UNLIKELY(
              (intRes = toInteger(runtime, numPos)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      pos = intRes->getNumber();
    }
  } else {
    auto intRes = toInteger(runtime, runtime->makeHandle(args.getArg(1)));
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    pos = intRes->getNumber();
  }

  double len = S->getStringLength();
  uint32_t start = static_cast<uint32_t>(std::min(std::max(pos, 0.), len));

  // TODO: good candidate for Boyer-Moore on large needles/haystacks
  // TODO: good candidate for memchr on length-1 needles
  auto SView = StringPrimitive::createStringView(runtime, S);
  auto searchStrView = StringPrimitive::createStringView(runtime, searchStr);
  double ret = -1;
  if (reverse) {
    uint32_t lastPossibleMatchEnd =
        std::min(SView.length(), start + searchStrView.length());
    auto foundIter = std::search(
        SView.rbegin() + (SView.length() - lastPossibleMatchEnd),
        SView.rend(),
        searchStrView.rbegin(),
        searchStrView.rend());
    if (foundIter != SView.rend() || searchStrView.empty()) {
      ret = SView.rend() - foundIter - searchStrView.length();
    }
  } else {
    auto foundIter = std::search(
        SView.begin() + start,
        SView.end(),
        searchStrView.begin(),
        searchStrView.end());
    if (foundIter != SView.end() || searchStrView.empty()) {
      ret = foundIter - SView.begin();
    }
  }
  return HermesValue::encodeDoubleValue(ret);
}

CallResult<HermesValue>
stringPrototypeIndexOf(void *, Runtime *runtime, NativeArgs args) {
  return stringDirectedIndexOf(runtime, args, false);
}

CallResult<HermesValue>
stringPrototypeLastIndexOf(void *, Runtime *runtime, NativeArgs args) {
  return stringDirectedIndexOf(runtime, args, true);
}

CallResult<HermesValue>
stringPrototypeLocaleCompare(void *, Runtime *runtime, NativeArgs args) {
  auto thisValue = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto sRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(sRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*sRes));

  auto tRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(tRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // "That" string.
  auto T = toHandle(runtime, std::move(*tRes));

  llvm::SmallVector<char16_t, 32> left;
  llvm::SmallVector<char16_t, 32> right;

  StringPrimitive::createStringView(runtime, S).copyUTF16String(left);
  StringPrimitive::createStringView(runtime, T).copyUTF16String(right);
  int comparisonResult = platform_unicode::localeCompare(left, right);
  assert(comparisonResult >= -1 && comparisonResult <= 1);
  return HermesValue::encodeNumberValue(comparisonResult);
}

CallResult<HermesValue>
stringPrototypeMatch(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  // 2. ReturnIfAbrupt(O).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 3. If regexp is neither undefined nor null, then
  auto regexp = args.getArgHandle(0);
  if (!regexp->isUndefined() && !regexp->isNull()) {
    // a. Let matcher be GetMethod(regexp, @@match).
    auto methodRes = getMethod(
        runtime,
        regexp,
        runtime->makeHandle(Predefined::getSymbolID(Predefined::SymbolMatch)));
    // b. ReturnIfAbrupt(matcher).
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If matcher is not undefined, then
    //   i. Return Call(matcher, regexp, «‍O»).
    if (!methodRes->getHermesValue().isUndefined()) {
      Handle<Callable> matcher =
          Handle<Callable>::vmcast(runtime, methodRes->getHermesValue());
      return Callable::executeCall1(
          matcher, runtime, regexp, O.getHermesValue());
    }
  }
  // 4. Let S be ToString(O).
  auto strRes = toString_RJS(runtime, O);
  // 5. ReturnIfAbrupt(S).
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  // 6. Let rx be RegExpCreate(regexp, undefined) (see 21.2.3.2.3).
  // 7. ReturnIfAbrupt(rx).
  auto regRes = regExpCreate(runtime, regexp, Runtime::getUndefinedValue());
  if (regRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSRegExp> rx = regRes.getValue();

  // 8. Return Invoke(rx, @@match, «‍S»).
  auto propRes = JSObject::getNamed_RJS(
      rx, runtime, Predefined::getSymbolID(Predefined::SymbolMatch));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto func = dyn_vmcast<Callable>(propRes.getValue());
  if (LLVM_UNLIKELY(!func)) {
    return runtime->raiseTypeError(
        "RegExp.prototype[@@match] must be callable.");
  }
  return Callable::executeCall1(
      runtime->makeHandle(func), runtime, rx, S.getHermesValue());
}

CallResult<HermesValue>
stringPrototypeNormalize(void *, Runtime *runtime, NativeArgs args) {
  using platform_unicode::NormalizationForm;

  // 1. Let O be RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. Let S be ToString(O).
  auto sRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(sRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*sRes));

  NormalizationForm form;

  // 4. If form is not provided or form is undefined, let form be "NFC".
  if (args.getArg(0).isUndefined()) {
    form = NormalizationForm::C;
  } else {
    // 5. Let f be ToString(form).
    auto fRes = toString_RJS(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(fRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto f = toHandle(runtime, std::move(*fRes));

    // 7. If f is not one of "NFC", "NFD", "NFKC", or "NFKD", throw a RangeError
    // exception.
    auto sv = StringPrimitive::createStringView(runtime, f);
    if (sv.equals(ASCIIRef{"NFC", 3})) {
      form = NormalizationForm::C;
    } else if (sv.equals(ASCIIRef{"NFD", 3})) {
      form = NormalizationForm::D;
    } else if (sv.equals(ASCIIRef{"NFKC", 4})) {
      form = NormalizationForm::KC;
    } else if (sv.equals(ASCIIRef{"NFKD", 4})) {
      form = NormalizationForm::KD;
    } else {
      return runtime->raiseRangeError(
          TwineChar16("Invalid normalization form: ") + *f);
    }
  }

  // 8. Let ns be the String value that is the result of normalizing S into the
  // normalization form named by f as specified in
  // http://www.unicode.org/reports/tr15/tr15-29.html.
  llvm::SmallVector<char16_t, 32> ns;
  S->copyUTF16String(ns);
  platform_unicode::normalize(ns, form);

  // 9. Return ns.
  return StringPrimitive::createEfficient(runtime, ns);
}

CallResult<HermesValue>
stringPrototypePad(void *ctx, Runtime *runtime, NativeArgs args) {
  bool padStart = (bool)ctx;

  // 1. Let O be ? RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 2. Let S be ? ToString(O).
  auto sRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(sRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*sRes));

  // 3. Let intMaxLength be ? ToLength(maxLength).
  auto intMaxLengthRes = toLength(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intMaxLengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  const uint64_t intMaxLength = intMaxLengthRes->getNumberAs<int64_t>();

  // 4. Let stringLength be the number of elements in S.
  const uint32_t stringLength = S->getStringLength();
  SafeUInt32 size{stringLength};

  // 5. If intMaxLength is not greater than stringLength, return S.
  if (intMaxLength <= stringLength) {
    return S.getHermesValue();
  }

  MutableHandle<StringPrimitive> filler{runtime};

  if (args.getArg(1).isUndefined()) {
    // 6. If fillString is undefined, let filler be a String consisting solely
    // of the code unit 0x0020 (SPACE).
    filler = runtime->getPredefinedString(Predefined::space);
  } else {
    // 7. Else, let filler be ? ToString(fillString).
    auto fillerRes = toString_RJS(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(fillerRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    filler = fillerRes->get();
  }

  // 8. If filler is the empty String, return S.
  if (filler->getStringLength() == 0) {
    return S.getHermesValue();
  }

  // 9. Let fillLen be intMaxLength - stringLength.
  const uint64_t fillLen = intMaxLength - stringLength;

  // Check for overflow and strings that are too long, so we can ensure uint32_t
  // is a large enough type to use for math below.
  if (fillLen > StringPrimitive::MAX_STRING_LENGTH) {
    return runtime->raiseRangeError("String pad result exceeds limit");
  }

  size.add((uint32_t)fillLen);

  if (size.isZero()) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }

  auto builderRes = StringBuilder::createStringBuilder(runtime, size);
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  uint32_t resultLen = size.get();

  // Repeatedly add filler to builder, taking up resultLen - stringLength
  // characters.
  auto addFiller = [&filler, resultLen, stringLength](StringBuilder &builder) {
    uint32_t remaining = resultLen - stringLength;
    const uint32_t fillerLen = filler->getStringLength();
    while (remaining != 0) {
      uint32_t length = std::min(remaining, fillerLen);
      builder.appendStringPrim(filler, length);
      remaining -= length;
    }
  };

  if (padStart) {
    // 10. Let truncatedStringFiller be a new String value consisting of
    // repeated concatenations of filler truncated to length fillLen.
    // 11. Return a new String value computed by the concatenation of
    // truncatedStringFiller and S.
    addFiller(*builderRes);
    builderRes->appendStringPrim(S);
  } else {
    // 10. Let truncatedStringFiller be a new String value consisting of
    // repeated concatenations of filler truncated to length fillLen.
    // 11. Return a new String value computed by the concatenation of S and
    // truncatedStringFiller.
    builderRes->appendStringPrim(S);
    addFiller(*builderRes);
  }

  return builderRes->getStringPrimitive().getHermesValue();
}

CallResult<HermesValue>
stringPrototypeRepeat(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. Let S be ToString(O).
  // 3. ReturnIfAbrupt(S).
  auto sRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(sRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*sRes));

  // 4. Let n be ToInteger(count).
  // 5. ReturnIfAbrupt(n).
  auto nRes = toInteger(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(nRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double n = nRes->getNumber();

  // 6. If n < 0, throw a RangeError exception.
  // 7. If n is +Infinity, throw a RangeError exception.
  if (n < 0 || n == std::numeric_limits<double>::infinity()) {
    return runtime->raiseRangeError(
        "String.prototype.repeat count must be finite and non-negative");
  }

  // 8. Let T be a String value that is made from n copies of S appended
  // together. If n is 0, T is the empty String.
  double strLen = S->getStringLength();

  if (n == 0 || strLen == 0) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }

  if (n > std::numeric_limits<uint32_t>::max() ||
      S->getStringLength() > (double)StringPrimitive::MAX_STRING_LENGTH / n) {
    // Check for overflow.
    return runtime->raiseRangeError(
        "String.prototype.repeat result exceeds limit");
  }

  // It's safe to multiply as the overflow check is done above.
  SafeUInt32 finalLen(strLen * n);

  auto builderRes = StringBuilder::createStringBuilder(runtime, finalLen);
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Using uint32_t for i is valid because we have bounds-checked n.
  for (uint32_t i = 0; i < n; ++i) {
    builderRes->appendStringPrim(S);
  }

  // 9. Return T.
  return builderRes->getStringPrimitive().getHermesValue();
}

CallResult<HermesValue>
stringPrototypeReplace(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  // 2. ReturnIfAbrupt(O).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 3. If searchValue is neither undefined nor null, then
  auto searchValue = args.getArgHandle(0);
  auto replaceValue = args.getArgHandle(1);
  if (!searchValue->isUndefined() && !searchValue->isNull()) {
    // a. Let replacer be GetMethod(searchValue, @@replace).
    auto methodRes = getMethod(
        runtime,
        searchValue,
        runtime->makeHandle(
            Predefined::getSymbolID(Predefined::SymbolReplace)));
    // b. ReturnIfAbrupt(replacer).
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If replacer is not undefined, then
    //   i. Return Call(replacer, searchValue, «‍O, replaceValue»).
    if (!methodRes->getHermesValue().isUndefined()) {
      // If methodRes is not Callable, step 3a would have thrown a TypeError.
      Handle<Callable> replacer =
          Handle<Callable>::vmcast(runtime, methodRes->getHermesValue());
      return Callable::executeCall2(
          replacer,
          runtime,
          searchValue,
          O.getHermesValue(),
          replaceValue.getHermesValue());
    }
  }
  // 4. Let string be ToString(O).
  // 5. ReturnIfAbrupt(string).
  auto stringRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(stringRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = toHandle(runtime, std::move(*stringRes));
  // 6. Let searchString be ToString(searchValue).
  auto searchStringRes = toString_RJS(runtime, searchValue);
  // 7. ReturnIfAbrupt(searchString).
  if (LLVM_UNLIKELY(searchStringRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchString = toHandle(runtime, std::move(*searchStringRes));
  // 8. Let functionalReplace be IsCallable(replaceValue).
  auto replaceFn = Handle<Callable>::dyn_vmcast(replaceValue);
  MutableHandle<StringPrimitive> replaceValueStr{runtime};
  bool functionalReplace = !!replaceFn;
  // 9. If functionalReplace is false, then
  if (!functionalReplace) {
    // a. Let replaceValue be ToString(replaceValue).
    // b. ReturnIfAbrupt(replaceValue).
    auto replaceValueStrRes = toString_RJS(runtime, replaceValue);
    if (LLVM_UNLIKELY(replaceValueStrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replaceValueStr = replaceValueStrRes->get();
  }
  // 10. Search string for the first occurrence of searchString and let pos be
  // the index within string of the first code unit of the matched substring and
  // let matched be searchString. If no occurrences of searchString were found,
  // return string.
  // Special case: if they're both empty then the match is at position 0.
  uint32_t pos = 0;
  auto strView = StringPrimitive::createStringView(runtime, string);
  if (!strView.empty()) {
    auto searchView = StringPrimitive::createStringView(runtime, searchString);
    auto searchResult = std::search(
        strView.begin(), strView.end(), searchView.begin(), searchView.end());

    if (searchResult != strView.end()) {
      pos = searchResult - strView.begin();
    } else {
      return string.getHermesValue();
    }
  } else if (searchString->getStringLength() != 0) {
    // If string is empty and search is not empty, there is no match.
    return string.getHermesValue();
  }
  MutableHandle<StringPrimitive> replStr{runtime};
  // 11. If functionalReplace is true, then
  if (functionalReplace) {
    // a. Let replValue be Call(replaceValue, undefined, «matched, pos, and
    // string»).
    auto callRes = Callable::executeCall3(
        replaceFn,
        runtime,
        Runtime::getUndefinedValue(),
        searchString.getHermesValue(),
        HermesValue::encodeNumberValue(pos),
        string.getHermesValue());
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // b. Let replStr be ToString(replValue).
    auto replStrRes =
        toString_RJS(runtime, runtime->makeHandle(callRes.getValue()));
    // c. ReturnIfAbrupt(replStr).
    if (LLVM_UNLIKELY(replStrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replStr = replStrRes->get();
  } else {
    // 12. Else,
    // a. Let captures be an empty List.
    auto nullHandle = Runtime::makeNullHandle<ArrayStorage>();
    // b. Let replStr be GetSubstitution(matched, string, pos, captures,
    // replaceValue).
    auto callRes = getSubstitution(
        runtime, searchString, string, pos, nullHandle, replaceValueStr);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replStr = vmcast<StringPrimitive>(callRes.getValue());
  }
  // 13. Let tailPos be pos + the number of code units in matched.
  uint32_t tailPos = pos + searchString->getStringLength();
  // 14. Let newString be the String formed by concatenating the first pos code
  // units of string, replStr, and the trailing substring of string starting at
  // index tailPos. If pos is 0, the first element of the concatenation will be
  // the empty String.
  SmallU16String<32> newString{};
  strView.slice(0, pos).copyUTF16String(newString);
  StringPrimitive::createStringView(runtime, replStr)
      .copyUTF16String(newString);
  strView.slice(tailPos).copyUTF16String(newString);
  // 15. Return newString.
  return StringPrimitive::create(runtime, newString);
}

CallResult<HermesValue>
stringPrototypeSearch(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  // 2. ReturnIfAbrupt(O).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 3. If regexp is neither undefined nor null, then
  auto regexp = args.getArgHandle(0);
  if (!regexp->isUndefined() && !regexp->isNull()) {
    // a. Let searcher be GetMethod(regexp, @@search).
    auto methodRes = getMethod(
        runtime,
        regexp,
        runtime->makeHandle(Predefined::getSymbolID(Predefined::SymbolSearch)));
    // b. ReturnIfAbrupt(searcher).
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If searcher is not undefined, then
    //   i. Return Call(searcher, regexp, «‍O»).
    if (!methodRes->getHermesValue().isUndefined()) {
      // If methodRes is not Callable, step 3a would have thrown a TypeError.
      Handle<Callable> searcher =
          Handle<Callable>::vmcast(runtime, methodRes->getHermesValue());
      return Callable::executeCall1(
          searcher, runtime, regexp, O.getHermesValue());
    }
  }
  // 4. Let string be ToString(O).
  // 5. ReturnIfAbrupt(string).
  auto strRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = toHandle(runtime, std::move(*strRes));

  // 6. Let rx be RegExpCreate(regexp, undefined) (see 21.2.3.2.3).
  // 7. ReturnIfAbrupt(rx).
  auto regRes = regExpCreate(runtime, regexp, Runtime::getUndefinedValue());
  if (regRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSRegExp> rx = *regRes;

  // 8. Return Invoke(rx, @@search, «string»).
  auto propRes = JSObject::getNamed_RJS(
      rx, runtime, Predefined::getSymbolID(Predefined::SymbolSearch));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!vmisa<Callable>(propRes.getValue()))) {
    return runtime->raiseTypeError(
        "RegExp.prototype[@@search] must be callable.");
  }
  auto func = Handle<Callable>::vmcast(runtime, *propRes);
  return Callable::executeCall1(func, runtime, rx, string.getHermesValue());
}

CallResult<HermesValue>
stringPrototypeEndsWith(void *, Runtime *runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 2. Let S be ToString(O).
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  // 4. Let isRegExp be IsRegExp(searchString).
  auto isRegExpRes = isRegExp(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 6. If isRegExp is true, throw a TypeError exception.
  if (LLVM_UNLIKELY(*isRegExpRes)) {
    return runtime->raiseTypeError(
        "First argument to endsWith must not be a RegExp");
  }

  // 7. Let searchStr be ToString(searchString).
  auto searchStrRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(searchStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = toHandle(runtime, std::move(*searchStrRes));

  // 9. Let len be the number of elements in S.
  double len = S->getStringLength();

  // 10. If endPosition is undefined, let pos be len, else let pos be
  // ToInteger(endPosition).
  double pos;
  if (args.getArg(1).isUndefined()) {
    pos = len;
  } else {
    auto posRes = toInteger(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(posRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    pos = posRes->getNumberAs<double>();
  }

  // 12. Let end be min(max(pos, 0), len).
  double end = std::min(std::max(pos, 0.0), len);

  // 13. Let searchLength be the number of elements in searchStr.
  double searchLength = searchStr->getStringLength();

  // 14. Let start be end - searchLength.
  double start = end - searchLength;

  // 15. If start is less than 0, return false.
  if (start < 0) {
    return HermesValue::encodeBoolValue(false);
  }
  // 16. If the sequence of elements of S starting at start of length
  // searchLength is the same as the full element sequence of searchStr, return
  // true.
  // 17. Otherwise, return false.
  return HermesValue::encodeBoolValue(
      S->sliceEquals(start, searchLength, *searchStr));
}

CallResult<HermesValue> stringPrototypeIncludesOrStartsWith(
    void *ctx,
    Runtime *runtime,
    NativeArgs args) {
  // If true, perform the startsWith operation.
  // Else, do the standard "includes" operation.
  bool startsWith = static_cast<bool>(ctx);

  // 1. Let O be RequireObjectCoercible(this value).
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 2. Let S be ToString(O).
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));

  // 4. Let isRegExp be IsRegExp(searchString).
  // 6. If isRegExp is true, throw a TypeError exception.
  auto isRegExpRes = isRegExp(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_UNLIKELY(*isRegExpRes)) {
    return runtime->raiseTypeError(
        "First argument to startsWith and includes must not be a RegExp");
  }

  // 7. Let searchStr be ToString(searchString).
  auto searchStrRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(searchStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = toHandle(runtime, std::move(*searchStrRes));

  // 9. Let pos be ToInteger(position).
  // (If position is undefined, this step produces the value 0).
  auto posRes = toInteger(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(posRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double pos = posRes->getNumber();

  // 11. Let len be the number of elements in S.
  double len = S->getStringLength();

  // 12. Let start be min(max(pos, 0), len).
  double start = std::min(std::max(pos, 0.0), len);

  // 13. Let searchLength be the number of elements in searchStr.
  double searchLength = searchStr->getStringLength();

  if (startsWith) {
    // Perform the startsWith operation instead of includes.
    // 14. If searchLength+start is greater than len, return false.
    if (searchLength + start > len) {
      return HermesValue::encodeBoolValue(false);
    }
    // 15. If the sequence of elements of S starting at start of length
    // searchLength is the same as the full element sequence of searchStr,
    // return true.
    // 16. Otherwise, return false.
    return HermesValue::encodeBoolValue(
        S->sliceEquals(start, searchLength, *searchStr));
  }

  // 14. If there exists any integer k not smaller than start such that k +
  // searchLen is not greater than len, and for all nonnegative integers j less
  // than searchLen, the code unit at index k+j of S is the same as the code
  // unit at index j of searchStr, return true; but if there is no such integer
  // k, return false.
  for (double k = start; k + searchLength <= len; ++k) {
    if (S->sliceEquals(k, searchLength, *searchStr)) {
      return HermesValue::encodeBoolValue(true);
    }
  }
  return HermesValue::encodeBoolValue(false);
}

CallResult<HermesValue>
stringPrototypeSymbolIterator(void *, Runtime *runtime, NativeArgs args) {
  auto thisValue = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = toHandle(runtime, std::move(*strRes));

  return JSStringIterator::create(runtime, string);
}

} // namespace vm
} // namespace hermes
