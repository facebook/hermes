/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

Handle<JSObject> createStringConstructor(Runtime &runtime) {
  auto stringPrototype = Handle<JSString>::vmcast(&runtime.stringPrototype);

  auto cons = defineSystemConstructor<JSString>(
      runtime,
      Predefined::getSymbolID(Predefined::String),
      stringConstructor,
      stringPrototype,
      1,
      CellKind::JSStringKind);

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
      Predefined::getSymbolID(Predefined::localeCompare),
      ctx,
      stringPrototypeLocaleCompare,
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
      Predefined::getSymbolID(Predefined::repeat),
      ctx,
      stringPrototypeRepeat,
      1);

  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();
  auto trimStartRes =
      runtime.makeHandle<Callable>(runtime.ignoreAllocationFailure(defineMethod(
          runtime,
          stringPrototype,
          Predefined::getSymbolID(Predefined::trimStart),
          ctx,
          stringPrototypeTrimStart,
          0,
          dpf)));
  auto trimEndRes =
      runtime.makeHandle<Callable>(runtime.ignoreAllocationFailure(defineMethod(
          runtime,
          stringPrototype,
          Predefined::getSymbolID(Predefined::trimEnd),
          ctx,
          stringPrototypeTrimEnd,
          0,
          dpf)));

  defineProperty(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::trimLeft),
      trimStartRes);
  defineProperty(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::trimRight),
      trimEndRes);

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
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::matchAll),
      ctx,
      stringPrototypeMatchAll,
      1);
  defineMethod(
      runtime,
      stringPrototype,
      Predefined::getSymbolID(Predefined::replaceAll),
      ctx,
      stringPrototypeReplaceAll,
      2);
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
      Predefined::getSymbolID(Predefined::charAt),
      ctx,
      stringPrototypeCharAt,
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
      Predefined::getSymbolID(Predefined::startsWith),
      (void *)true,
      stringPrototypeIncludesOrStartsWith,
      1);

  return cons;
}

CallResult<HermesValue>
stringConstructor(void *, Runtime &runtime, NativeArgs args) {
  if (args.getArgCount() == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
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
  auto s = runtime.makeHandle(std::move(*sRes));

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
stringFromCharCode(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  uint32_t n = args.getArgCount();
  if (LLVM_LIKELY(n == 1)) {
    // Fast path for when only one argument is provided.
    auto res = toUInt16(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    char16_t ch = res->getNumber();
    return runtime.getCharacterString(ch).getHermesValue();
  }
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
stringFromCodePoint(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let codePoints be a List containing the arguments passed to this
  // function.
  // 2. Let length be the number of elements in codePoints.
  uint32_t length = args.getArgCount();
  // 3. Let elements be a new List.
  llvh::SmallVector<char16_t, 32> elements{};
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

    // 5d. If SameValue(nextCP, ToIntegerOrInfinity(nextCP)) is false, throw
    // a RangeError exception.
    auto nextCPInt = toIntegerOrInfinity(runtime, nextCP);
    if (LLVM_UNLIKELY(nextCPInt == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!isSameValue(*nextCP, *nextCPInt)) {
      return runtime.raiseRangeError(
          TwineChar16("Code point must be an integer: ") + nextCP->getNumber());
    }

    // 5e. If nextCP < 0 or nextCP > 0x10FFFF, throw a RangeError exception.
    if (nextCP->getNumber() < 0 || nextCP->getNumber() > 0x10FFFF) {
      return runtime.raiseRangeError(
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
CallResult<HermesValue> stringRaw(void *, Runtime &runtime, NativeArgs args) {
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
  auto cooked = runtime.makeHandle<JSObject>(*cookedRes);

  // 5. Let raw be ToObject(Get(cooked, "raw")).
  auto getRes = JSObject::getNamed_RJS(
      cooked, runtime, Predefined::getSymbolID(Predefined::raw));
  if (LLVM_UNLIKELY(getRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto rawRes = toObject(runtime, runtime.makeHandle(std::move(*getRes)));
  if (LLVM_UNLIKELY(rawRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto raw = runtime.makeHandle<JSObject>(*rawRes);

  // 7. Let literalSegments be ToLength(Get(raw, "length"))
  auto lengthRes = JSObject::getNamed_RJS(
      raw, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto literalSegmentsRes =
      toLength(runtime, runtime.makeHandle(std::move(*lengthRes)));
  if (LLVM_UNLIKELY(literalSegmentsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  int64_t literalSegments = literalSegmentsRes->getNumberAs<int64_t>();
  // 9. If literalSegments ≤ 0, return the empty string.
  if (literalSegments <= 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // 10. Let stringElements be a new List.
  llvh::SmallVector<char16_t, 32> stringElements{};

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
    tmpHandle = std::move(*nextSegPropRes);
    auto nextSegRes = toString_RJS(runtime, tmpHandle);
    if (LLVM_UNLIKELY(nextSegRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nextSeg = nextSegRes->get();

    // 12. d. Append in order the code unit elements of nextSeg to the end of
    // stringElements.
    nextSeg->appendUTF16String(stringElements);

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
      nextSub->appendUTF16String(stringElements);
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
stringPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  if (args.getThisArg().isString()) {
    return args.getThisArg();
  }

  // Not a String value, must be a string object.
  auto *strPtr = dyn_vmcast<JSString>(args.getThisArg());
  if (strPtr) {
    // Only return the string if called on a String object.
    return HermesValue::encodeStringValue(
        JSString::getPrimitiveString(strPtr, runtime));
  }
  return runtime.raiseTypeError(
      "String.prototype.toString() called on non-string object");
}

CallResult<HermesValue>
stringPrototypeCharCodeAt(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*strRes));
  auto intRes =
      toIntegerOrInfinity(runtime, runtime.makeHandle(args.getArg(0)));
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
stringPrototypeCodePointAt(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*strRes));

  // 4. Let position be ToIntegerOrInfinity(pos).
  auto positionRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
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
stringPrototypeConcat(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*strRes));
  // Track the total characters in the result.
  SafeUInt32 size(S->getStringLength());
  uint32_t argCount = args.getArgCount();

  // Store the results of toStrings and concat them at the end.
  auto arrRes = ArrayStorageSmall::create(runtime, argCount, argCount);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strings = runtime.makeHandle<ArrayStorageSmall>(*arrRes);

  // Run toString on the arguments to figure out the final size.
  auto marker = gcScope.createMarker();
  for (uint32_t i = 0; i < argCount; ++i) {
    auto strRes = toString_RJS(runtime, args.getArgHandle(i));
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // Allocations can't be performed here,
    // and we know we're in bounds because we preallocated.
    strings->set(
        i,
        SmallHermesValue::encodeStringValue(strRes->get(), runtime),
        runtime.getHeap());
    uint32_t strLength = strRes->get()->getStringLength();

    size.add(strLength);
    if (LLVM_UNLIKELY(size.isOverflowed())) {
      return runtime.raiseRangeError("resulting string length exceeds limit");
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
    element = strings->at(i).getString(runtime);
    builder->appendStringPrim(element);
  }
  return builder->getStringPrimitive().getHermesValue();
}

CallResult<HermesValue>
stringPrototypeSubstring(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));
  double len = S->getStringLength();

  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double intStart = intRes->getNumber();

  double intEnd;
  if (args.getArg(1).isUndefined()) {
    intEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1))) ==
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
    Runtime &runtime,
    Handle<StringPrimitive> S,
    const bool upperCase,
    const bool useCurrentLocale) {
  // Copying is unavoidable in this function, do it early on.
  SmallU16String<32> buff;
  // Must copy instead of just getting the reference, because later operations
  // may trigger GC and hence invalid pointers inside S.
  S->appendUTF16String(buff);
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
          return runtime.getCharacterString(c & ~(isLower << 5))
              .getHermesValue();
        } else {
          char16_t isUpper = 'A' <= c && c <= 'Z';
          return runtime.getCharacterString(c | (isUpper << 5))
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
stringPrototypeToLowerCase(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(
      runtime, runtime.makeHandle(std::move(*res)), false, false);
}

CallResult<HermesValue>
stringPrototypeToLocaleLowerCase(void *ctx, Runtime &runtime, NativeArgs args) {
#ifdef HERMES_ENABLE_INTL
  return intlStringPrototypeToLocaleLowerCase(/* unused */ ctx, runtime, args);
#else
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, runtime.makeHandle(std::move(*res)), false, true);
#endif
}

CallResult<HermesValue>
stringPrototypeToUpperCase(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, runtime.makeHandle(std::move(*res)), true, false);
}

CallResult<HermesValue>
stringPrototypeToLocaleUpperCase(void *ctx, Runtime &runtime, NativeArgs args) {
#ifdef HERMES_ENABLE_INTL
  return intlStringPrototypeToLocaleUpperCase(/* unused */ ctx, runtime, args);
#else
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return convertCase(runtime, runtime.makeHandle(std::move(*res)), true, true);
#endif
}

CallResult<HermesValue>
stringPrototypeSubstr(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));
  double stringLen = S->getStringLength();

  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double start = intRes->getNumber();

  double length;
  if (args.getArg(1).isUndefined()) {
    length = stringLen;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1))) ==
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
        runtime.getPredefinedString(Predefined::emptyString));
  } else {
    return StringPrimitive::slice(
        runtime,
        S,
        static_cast<size_t>(start),
        static_cast<size_t>(adjustedLength));
  }
}

/// \return the number of characters to trim from the begin iterator.
static size_t trimStart(
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
static size_t trimEnd(
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
stringPrototypeTrim(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t beginIdx = 0, endIdx = S->getStringLength();
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    beginIdx = trimStart(begin, end);
    begin += beginIdx;
    endIdx -= trimEnd(begin, end);
  }

  return StringPrimitive::slice(runtime, S, beginIdx, endIdx - beginIdx);
}

CallResult<HermesValue>
stringPrototypeTrimStart(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t beginIdx = 0;
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    beginIdx = trimStart(begin, end);
  }

  return StringPrimitive::slice(
      runtime, S, beginIdx, S->getStringLength() - beginIdx);
}

CallResult<HermesValue>
stringPrototypeTrimEnd(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto res = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*res));

  // Move begin and end to point to the first and last non-whitespace chars.
  size_t endIdx = S->getStringLength();
  {
    auto str = StringPrimitive::createStringView(runtime, S);
    auto begin = str.begin();
    auto end = str.end();
    endIdx -= trimEnd(begin, end);
  }

  return StringPrimitive::slice(runtime, S, 0, endIdx);
}

CallResult<HermesValue>
stringPrototypeLocaleCompare(void *ctx, Runtime &runtime, NativeArgs args) {
#ifdef HERMES_ENABLE_INTL
  return intlStringPrototypeLocaleCompare(/* unused */ ctx, runtime, args);
#else
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
  auto S = runtime.makeHandle(std::move(*sRes));

  auto tRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(tRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // "That" string.
  auto T = runtime.makeHandle(std::move(*tRes));

  llvh::SmallVector<char16_t, 32> left;
  llvh::SmallVector<char16_t, 32> right;

  StringPrimitive::createStringView(runtime, S).appendUTF16String(left);
  StringPrimitive::createStringView(runtime, T).appendUTF16String(right);
  int comparisonResult = platform_unicode::localeCompare(left, right);
  assert(comparisonResult >= -1 && comparisonResult <= 1);
  return HermesValue::encodeNumberValue(comparisonResult);
#endif
}

CallResult<HermesValue>
stringPrototypeNormalize(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*sRes));

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
    auto f = runtime.makeHandle(std::move(*fRes));

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
      return runtime.raiseRangeError(
          TwineChar16("Invalid normalization form: ") + *f);
    }
  }

  // 8. Let ns be the String value that is the result of normalizing S into the
  // normalization form named by f as specified in
  // http://www.unicode.org/reports/tr15/tr15-29.html.
  llvh::SmallVector<char16_t, 32> ns;
  S->appendUTF16String(ns);
  platform_unicode::normalize(ns, form);

  // 9. Return ns.
  return StringPrimitive::createEfficient(runtime, ns);
}

CallResult<HermesValue>
stringPrototypeRepeat(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*sRes));

  // 4. Let n be ToIntegerOrInfinity(count).
  // 5. ReturnIfAbrupt(n).
  auto nRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(nRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double n = nRes->getNumber();

  // 6. If n < 0, throw a RangeError exception.
  // 7. If n is +Infinity, throw a RangeError exception.
  if (n < 0 || n == std::numeric_limits<double>::infinity()) {
    return runtime.raiseRangeError(
        "String.prototype.repeat count must be finite and non-negative");
  }

  // 8. Let T be a String value that is made from n copies of S appended
  // together. If n is 0, T is the empty String.
  double strLen = S->getStringLength();

  if (n == 0 || strLen == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  if (n > std::numeric_limits<uint32_t>::max() ||
      S->getStringLength() > (double)StringPrimitive::MAX_STRING_LENGTH / n) {
    // Check for overflow.
    return runtime.raiseRangeError(
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

/// ES6.0 21.1.3.27 String.prototype [ @@iterator ]( )
CallResult<HermesValue>
stringPrototypeSymbolIterator(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let O be RequireObjectCoercible(this value).
  auto thisValue = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, thisValue) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. Let S be ToString(O).
  // 3. ReturnIfAbrupt(S).
  auto strRes = toString_RJS(runtime, thisValue);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = runtime.makeHandle(std::move(*strRes));

  // 4. Return CreateStringIterator(S).
  return JSStringIterator::create(runtime, string).getHermesValue();
}

CallResult<HermesValue>
stringPrototypeMatchAll(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let O be ? RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 2. If regexp is neither undefined nor null, then
  auto regexp = args.getArgHandle(0);
  if (!regexp->isUndefined() && !regexp->isNull()) {
    // a. Let isRegExp be ? IsRegExp(regexp).
    auto isRegExpRes = isRegExp(runtime, regexp);
    if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // b. If isRegExp is true, then
    if (*isRegExpRes) {
      // Passing undefined and null checks imply regexp is an ObjectCoercible.
      Handle<JSObject> regexpObj = Handle<JSObject>::vmcast(regexp);
      bool isGlobal = false;
      // i. Let flags be ? Get(regexp, "flags").
      auto flagsPropRes = JSObject::getNamed_RJS(
          regexpObj, runtime, Predefined::getSymbolID(Predefined::flags));
      if (LLVM_UNLIKELY(flagsPropRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto flags = runtime.makeHandle(std::move(*flagsPropRes));
      // ii. Perform ? RequireObjectCoercible(flags).
      if (LLVM_UNLIKELY(
              checkObjectCoercible(runtime, flags) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // iii. If ? ToString(flags) does not contain "g", throw a TypeError
      // exception.
      auto strRes = toString_RJS(runtime, flags);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto strView = StringPrimitive::createStringView(
          runtime, runtime.makeHandle(std::move(*strRes)));
      for (char16_t c : strView)
        if (c == u'g')
          isGlobal = true;
      if (!isGlobal)
        return runtime.raiseTypeError(
            "String.prototype.matchAll called with a non-global RegExp argument");
    }
    // c. Let matcher be ? GetMethod(regexp, @@matchAll).
    auto matcherRes = getMethod(
        runtime,
        regexp,
        runtime.makeHandle(
            Predefined::getSymbolID(Predefined::SymbolMatchAll)));
    if (LLVM_UNLIKELY(matcherRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // d. If matcher is not undefined, then
    if (!matcherRes->getHermesValue().isUndefined()) {
      auto matcher = runtime.makeHandle<Callable>(std::move(*matcherRes));
      // i. Return ? Call(matcher, regexp, «O»).
      return Callable::executeCall1(
                 matcher, runtime, regexp, O.getHermesValue())
          .toCallResultHermesValue();
    }
  }

  // 3. Let S be ? ToString(O).
  auto strRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));

  // 4. Let rx be ? RegExpCreate(regexp, "g").
  auto regRes = regExpCreate(runtime, regexp, runtime.getCharacterString('g'));
  if (regRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSRegExp> rx = regRes.getValue();

  // 5. Return ? Invoke(rx, @@matchAll, «S»).
  auto propRes = JSObject::getNamed_RJS(
      rx, runtime, Predefined::getSymbolID(Predefined::SymbolMatchAll));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto func =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(!func)) {
    return runtime.raiseTypeError(
        "RegExp.prototype[@@matchAll] must be callable.");
  }
  return Callable::executeCall1(func, runtime, rx, S.getHermesValue())
      .toCallResultHermesValue();
}

/// This provides a shared implementation of three operations in ES2021:
/// 6.1.4.1 Runtime Semantics: StringIndexOf ( string, searchValue, fromIndex )
///   when clampPostion=false,
/// 21.1.3.8 String.prototype.indexOf ( searchString [ , position ] )
///   when reverse=false, and
/// 21.1.3.9 String.prototype.lastIndexOf ( searchString [ , position ] )
///   when reverse=true.
///
/// Given a haystack ('string'), needle ('searchString'), and position, return
/// the index of the first (reverse=false) or last (reverse=true) substring
/// match of needle within haystack that is not smaller (normal) or larger
/// (reverse) than position.
///
/// \param runtime  the runtime to use for argument coercions
/// \param string  represent the string searching from, i.e. "this" of
///   indexOf / lastIndexOf or "string" of StringIndexOf.
/// \param searchString  represent the substring searching for, i.e.
///   "searchString" of indexOf / lastIndexOf or "searchValue" of StringIndexOf.
/// \param position  represent the starting index of the search, i.e.
///   "position" of indexOf / lastIndexOf or "fromIndex" of StringIndexOf.
/// \param reverse  whether we are running lastIndexOf (true) or indexOf (false)
/// \param clampPosition  whether the "position" is clamped to [0, length)
///   (true if running indexOf / lastIndexOf) or not (running StringIndexOf).
/// \returns Hermes-encoded index of the substring match, or -1 on failure
static CallResult<HermesValue> stringDirectedIndexOf(
    Runtime &runtime,
    Handle<> string,
    Handle<> searchString,
    Handle<> position,
    bool reverse,
    bool clampPosition = true) {
  // 1. Let O be ? RequireObjectCoercible(this value).
  // Call a function that may throw, let the runtime record it.
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, string) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. Let S be ? ToString(O).
  auto strRes = toString_RJS(runtime, string);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));

  // 3. Let searchStr be ? ToString(searchString).
  auto searchStrRes = toString_RJS(runtime, searchString);
  if (searchStrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = runtime.makeHandle(std::move(*searchStrRes));

  double pos;
  if (reverse) {
    // lastIndexOf
    // 4. Let numPos be ? ToNumber(position).
    auto intRes = toNumber_RJS(runtime, position);
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    Handle<> numPos = runtime.makeHandle(intRes.getValue());
    // 6. If numPos is NaN, let pos be +∞; otherwise, let pos be !
    // ToIntegerOrInfinity(numPos).
    if (std::isnan(numPos->getNumber())) {
      pos = std::numeric_limits<double>::infinity();
    } else {
      if (LLVM_UNLIKELY(
              (intRes = toIntegerOrInfinity(runtime, numPos)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      pos = intRes->getNumber();
    }
  } else {
    // indexOf
    // 4. Let pos be ? ToIntegerOrInfinity(position).
    auto intRes = toIntegerOrInfinity(runtime, position);
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    pos = intRes->getNumber();
  }

  // Let len be the length of S.
  double len = S->getStringLength();

  // When pos > len and "searchString" is an empty string, StringIndexOf behaves
  // differently than String.prototype.indexOf in terms of the "clampPosition":
  //   'aa'.indexOf('', 3) => 2               3 is clamped to 2.
  //    StringIndexOf('aa', '', 3) => -1      3 is not clamped thus -1.
  // Also, when pos > len and "searchString" is non-empty, they both fail.
  // Therefore, it's safe to early return -1 for the case of StringIndexOf
  // (i.e. clampPosition=false) as soon as pos > len is observed.
  if (!clampPosition && pos > len) {
    return HermesValue::encodeNumberValue(-1);
  }

  // Let start be min(max(pos, 0), len).
  uint32_t start = static_cast<uint32_t>(std::min(std::max(pos, 0.), len));

  // TODO: good candidate for Boyer-Moore on large needles/haystacks
  // TODO: good candidate for memchr on length-1 needles
  auto SView = StringPrimitive::createStringView(runtime, S);
  auto searchStrView = StringPrimitive::createStringView(runtime, searchStr);
  double ret = -1;
  if (reverse) {
    // lastIndexOf
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
    // indexOf
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

/// ES12 6.1.4.1 Runtime Semantics: StringIndexOf ( string, searchValue,
/// fromIndex )
/// This is currently implemented as a wrapper of stringDirectedIndexOf.
/// Ideally, this can be implemented with less runtime checks and provide a fast
/// path than stringDirectedIndexOf. TODO(T74338730)
static CallResult<HermesValue> stringIndexOf(
    Runtime &runtime,
    Handle<StringPrimitive> string,
    Handle<StringPrimitive> searchValue,
    uint32_t fromIndex) {
  // 1. Assert: Type(string) is String.
  // 2. Assert: Type(searchValue) is String.
  // 3. Assert: ! IsNonNegativeInteger(fromIndex) is true.
  return stringDirectedIndexOf(
      runtime,
      string,
      searchValue,
      runtime.makeHandle(HermesValue::encodeDoubleValue(fromIndex)),
      false,
      false);
}

/// ES12 21.1.3.18 String.prototype.replaceAll ( searchValue, replaceValue )
CallResult<HermesValue>
stringPrototypeReplaceAll(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto searchValue = args.getArgHandle(0);
  auto replaceValue = args.getArgHandle(1);
  // 2. If searchValue is neither undefined nor null, then
  if (!searchValue->isUndefined() && !searchValue->isNull()) {
    // a. Let isRegExp be ? IsRegExp(searchValue).
    auto isRegExpRes = isRegExp(runtime, searchValue);
    if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // b. If isRegExp is true, then
    if (*isRegExpRes) {
      Handle<JSObject> regexpObj = Handle<JSObject>::vmcast(searchValue);
      // i. Let flags be ? Get(searchValue, "flags").
      auto flagsPropRes = JSObject::getNamed_RJS(
          regexpObj, runtime, Predefined::getSymbolID(Predefined::flags));
      if (LLVM_UNLIKELY(flagsPropRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto flags = runtime.makeHandle(std::move(*flagsPropRes));
      // ii. Perform ? RequireObjectCoercible(flags).
      if (LLVM_UNLIKELY(
              checkObjectCoercible(runtime, flags) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // iii. If ? ToString(flags) does not contain "g", throw a TypeError
      // exception.
      auto strRes = toString_RJS(runtime, flags);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto strView = StringPrimitive::createStringView(
          runtime, runtime.makeHandle(std::move(*strRes)));
      bool isGlobal = false;
      for (char16_t c : strView)
        if (c == u'g')
          isGlobal = true;
      if (!isGlobal)
        return runtime.raiseTypeError(
            "String.prototype.replaceAll called with a non-global RegExp argument");
    }
    // c. Let replacer be ? GetMethod(searchValue, @@replace).
    auto replacerRes = getMethod(
        runtime,
        searchValue,
        runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolReplace)));
    if (LLVM_UNLIKELY(replacerRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // d. If replacer is not undefined, then
    if (!replacerRes->getHermesValue().isUndefined()) {
      auto replacer = runtime.makeHandle<Callable>(std::move(*replacerRes));
      // i. Return ? Call(replacer, searchValue, « O, replaceValue »)
      return Callable::executeCall2(
                 replacer,
                 runtime,
                 searchValue,
                 O.getHermesValue(),
                 replaceValue.getHermesValue())
          .toCallResultHermesValue();
    }
  }

  // 3. Let string be ? ToString(O).
  auto strRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = runtime.makeHandle(std::move(*strRes));

  // 4. Let searchString be ? ToString(searchValue).
  auto searchStrRes = toString_RJS(runtime, searchValue);
  if (LLVM_UNLIKELY(searchStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchString = runtime.makeHandle(std::move(*searchStrRes));

  // 5. Let functionalReplace be IsCallable(replaceValue).
  auto replaceFn = Handle<Callable>::dyn_vmcast(replaceValue);
  bool functionalReplace = !!replaceFn;

  // It need to mutable since it's written here but read below.
  MutableHandle<StringPrimitive> replaceValueStr{runtime};
  // 6. If functionalReplace is false, then
  if (!functionalReplace) {
    // a. Set replaceValue to ? ToString(replaceValue).
    auto replaceValueStrRes = toString_RJS(runtime, replaceValue);
    if (LLVM_UNLIKELY(replaceValueStrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replaceValueStr = std::move(*replaceValueStrRes);
  }

  // 7. Let searchLength be the length of searchString.
  uint32_t searchLength = searchString->getStringLength();
  // 8. Let advanceBy be max(1, searchLength).
  uint32_t advanceBy = std::max(1u, searchLength);

  // 9. Let matchPositions be a new empty List.
  llvh::SmallVector<int32_t, 8> matchPositions{};

  // 10. Let position be ! StringIndexOf(string, searchString, 0).
  auto positionRes = stringIndexOf(runtime, string, searchString, 0);
  int32_t position = positionRes->getNumberAs<int32_t>();

  // 11. Repeat, while position is not -1,
  while (position != -1) {
    GCScopeMarkerRAII marker{runtime};
    // a. Append position to the end of matchPositions.
    matchPositions.push_back(position);
    // b. Set position to ! StringIndexOf(string, searchString, position +
    // advanceBy).
    positionRes =
        stringIndexOf(runtime, string, searchString, position + advanceBy);
    assert(
        positionRes == ExecutionStatus::RETURNED &&
        "StringIndexOf cannot fail");
    position = positionRes->getNumberAs<int32_t>();
  }

  // 12. Let endOfLastMatch be 0.
  uint32_t endOfLastMatch = 0;
  // 13. Let result be the empty String value.
  SmallU16String<32> result{};

  // 14. For each position in matchPositions, do
  auto stringView = StringPrimitive::createStringView(runtime, string);
  MutableHandle<StringPrimitive> replacement{runtime};
  MutableHandle<> replacementCallRes{runtime};
  for (uint32_t i = 0, size = matchPositions.size(); i < size; ++i) {
    GCScopeMarkerRAII marker{runtime};
    uint32_t position = matchPositions[i];
    // a. Let preserved be the substring of string from endOfLastMatch to
    // position.
    // Noted that "substring" is from inclusiveStart to exclusiveEnd.
    auto preserved =
        stringView.slice(endOfLastMatch, position - endOfLastMatch);
    // b. If functionalReplace is true, then
    if (functionalReplace) {
      // i. Let replacement be ? ToString(?
      //   Call(replaceValue, undefined, « searchString, position, string »)).
      auto callRes = Callable::executeCall3(
                         replaceFn,
                         runtime,
                         Runtime::getUndefinedValue(),
                         searchString.getHermesValue(),
                         HermesValue::encodeNumberValue(position),
                         string.getHermesValue())
                         .toCallResultHermesValue();
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      replacementCallRes = *callRes;
      auto replacementStrRes = toString_RJS(runtime, replacementCallRes);
      if (LLVM_UNLIKELY(replacementStrRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      replacement = std::move(*replacementStrRes);
    } else {
      // c. Else,
      // i. Assert: Type(replaceValue) is String.
      // ii. Let captures be a new empty List.
      auto captures = Runtime::makeNullHandle<ArrayStorageSmall>();
      // iii. Let replacement be ! GetSubstitution(searchString, string,
      // position, captures, undefined, replaceValue).
      auto callRes = getSubstitution(
          runtime,
          searchString,
          string,
          (double)position,
          captures,
          replaceValueStr);
      replacement = vmcast<StringPrimitive>(*callRes);
    }

    // d. Set result to the string-concatenation of result, preserved, and
    // replacement.
    preserved.appendUTF16String(result);
    StringPrimitive::createStringView(runtime, replacement)
        .appendUTF16String(result);
    // e. Set endOfLastMatch to position + searchLength.
    endOfLastMatch = position + searchLength;
  }

  // 15. If endOfLastMatch < the length of string, then
  if (endOfLastMatch < string->getStringLength()) {
    // a. Set result to the string-concatenation of result and the substring of
    // string from endOfLastMatch.
    stringView.slice(endOfLastMatch).appendUTF16String(result);
  }
  // 16. Return result.
  return StringPrimitive::create(runtime, result);
}

CallResult<HermesValue>
stringPrototypeMatch(void *, Runtime &runtime, NativeArgs args) {
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
        runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolMatch)));
    // b. ReturnIfAbrupt(matcher).
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If matcher is not undefined, then
    //   i. Return Call(matcher, regexp, «‍O»).
    if (!methodRes->getHermesValue().isUndefined()) {
      auto matcher = runtime.makeHandle<Callable>(std::move(*methodRes));
      return Callable::executeCall1(
                 matcher, runtime, regexp, O.getHermesValue())
          .toCallResultHermesValue();
    }
  }
  // 4. Let S be ToString(O).
  auto strRes = toString_RJS(runtime, O);
  // 5. ReturnIfAbrupt(S).
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));

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
  auto func =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(!func)) {
    return runtime.raiseTypeError(
        "RegExp.prototype[@@match] must be callable.");
  }
  return Callable::executeCall1(func, runtime, rx, S.getHermesValue())
      .toCallResultHermesValue();
}

CallResult<HermesValue>
stringPrototypePad(void *ctx, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*sRes));

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
    filler = runtime.getPredefinedString(Predefined::space);
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
    return runtime.raiseRangeError("String pad result exceeds limit");
  }

  size.add((uint32_t)fillLen);

  if (size.isZero()) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
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
stringPrototypeReplace(void *, Runtime &runtime, NativeArgs args) {
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
        runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolReplace)));
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
                 replaceValue.getHermesValue())
          .toCallResultHermesValue();
    }
  }
  // 4. Let string be ToString(O).
  // 5. ReturnIfAbrupt(string).
  auto stringRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(stringRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = runtime.makeHandle(std::move(*stringRes));
  // 6. Let searchString be ToString(searchValue).
  auto searchStringRes = toString_RJS(runtime, searchValue);
  // 7. ReturnIfAbrupt(searchString).
  if (LLVM_UNLIKELY(searchStringRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchString = runtime.makeHandle(std::move(*searchStringRes));
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
    replaceValueStr = std::move(*replaceValueStrRes);
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
        toString_RJS(runtime, runtime.makeHandle(std::move(*callRes)));
    // c. ReturnIfAbrupt(replStr).
    if (LLVM_UNLIKELY(replStrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replStr = replStrRes->get();
  } else {
    // 12. Else,
    // a. Let captures be an empty List.
    auto nullHandle = Runtime::makeNullHandle<ArrayStorageSmall>();
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
  strView.slice(0, pos).appendUTF16String(newString);
  StringPrimitive::createStringView(runtime, replStr)
      .appendUTF16String(newString);
  strView.slice(tailPos).appendUTF16String(newString);
  // 15. Return newString.
  return StringPrimitive::create(runtime, newString);
}

CallResult<HermesValue>
stringPrototypeSearch(void *, Runtime &runtime, NativeArgs args) {
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
        runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolSearch)));
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
                 searcher, runtime, regexp, O.getHermesValue())
          .toCallResultHermesValue();
    }
  }
  // 4. Let string be ToString(O).
  // 5. ReturnIfAbrupt(string).
  auto strRes = toString_RJS(runtime, O);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto string = runtime.makeHandle(std::move(*strRes));

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
  if (LLVM_UNLIKELY(!vmisa<Callable>(propRes->get()))) {
    return runtime.raiseTypeError(
        "RegExp.prototype[@@search] must be callable.");
  }
  auto func = Handle<Callable>::vmcast(runtime.makeHandle(std::move(*propRes)));
  return Callable::executeCall1(func, runtime, rx, string.getHermesValue())
      .toCallResultHermesValue();
}

CallResult<HermesValue>
stringPrototypeCharAt(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*strRes));

  auto intRes =
      toIntegerOrInfinity(runtime, runtime.makeHandle(args.getArg(0)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto position = intRes->getNumber();
  auto size = S->getStringLength();
  if (position < 0 || position >= size) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }
  auto result = runtime.getCharacterString(
      StringPrimitive::createStringView(runtime, S)[position]);
  return HermesValue::encodeStringValue(result.get());
}

CallResult<HermesValue>
stringPrototypeSlice(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, args.getThisHandle()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));
  double len = S->getStringLength();

  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double intStart = intRes->getNumber();

  double intEnd;
  if (args.getArg(1).isUndefined()) {
    intEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1))) ==
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

CallResult<HermesValue>
stringPrototypeEndsWith(void *, Runtime &runtime, NativeArgs args) {
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
  auto S = runtime.makeHandle(std::move(*strRes));

  // 4. Let isRegExp be IsRegExp(searchString).
  auto isRegExpRes = isRegExp(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 6. If isRegExp is true, throw a TypeError exception.
  if (LLVM_UNLIKELY(*isRegExpRes)) {
    return runtime.raiseTypeError(
        "First argument to endsWith must not be a RegExp");
  }

  // 7. Let searchStr be ToString(searchString).
  auto searchStrRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(searchStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = runtime.makeHandle(std::move(*searchStrRes));

  // 9. Let len be the number of elements in S.
  double len = S->getStringLength();

  // 10. If endPosition is undefined, let pos be len, else let pos be
  // ToIntegerOrInfinity(endPosition).
  double pos;
  if (args.getArg(1).isUndefined()) {
    pos = len;
  } else {
    auto posRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
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

/// ES11 21.1.3.20.1
/// Works slightly differently from the given implementation in the spec.
/// Given a string \p S and a starting point \p q, finds the first match of
/// \p R such that it starts on or after index \p q in \p S.
/// \param q starting point in S. Requires: q <= S->getStringLength().
/// \param R a String.
/// \return Either None or an integer representing the start of the match.
static OptValue<uint32_t> splitMatch(
    Runtime &runtime,
    Handle<StringPrimitive> S,
    uint32_t q,
    Handle<StringPrimitive> R) {
  // 2. Let r be the number of code units in R.
  auto r = R->getStringLength();
  // 3. Let s be the number of code units in S.
  auto s = S->getStringLength();
  // 4. If q+r > s, return false.
  if (q + r > s) {
    return llvh::None;
  }

  // Handle the case where the search starts at the end of the string and R is
  // the empty string. This path should only be triggered when S is itself the
  // empty string.
  if (q == s) {
    return q;
  }

  auto SStr = StringPrimitive::createStringView(runtime, S);
  auto RStr = StringPrimitive::createStringView(runtime, R);

  auto sliced = SStr.slice(q);
  auto searchResult =
      std::search(sliced.begin(), sliced.end(), RStr.begin(), RStr.end());

  if (searchResult != sliced.end()) {
    return q + (searchResult - sliced.begin()) + r;
  }
  return llvh::None;
}

// ES11 21.1.3.20 String.prototype.split ( separator, limit )
CallResult<HermesValue>
stringPrototypeSplit(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? RequireObjectCoercible(this value).
  auto O = args.getThisHandle();
  if (LLVM_UNLIKELY(
          checkObjectCoercible(runtime, O) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. If separator is neither undefined nor null, then
  auto separator = args.getArgHandle(0);
  if (!separator->isUndefined() && !separator->isNull()) {
    // a. Let splitter be ? GetMethod(separator, @@split).
    auto methodRes = getMethod(
        runtime,
        separator,
        runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolSplit)));
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // b. If splitter is not undefined, then
    // i. Return ? Call(splitter, separator, «‍O, limit»).
    if (!methodRes->getHermesValue().isUndefined()) {
      Handle<Callable> splitter =
          Handle<Callable>::vmcast(runtime, methodRes->getHermesValue());
      return Callable::executeCall2(
                 splitter,
                 runtime,
                 separator,
                 O.getHermesValue(),
                 args.getArg(1))
          .toCallResultHermesValue();
    }
  }

  // 3. Let S be ? ToString(O).
  auto strRes = toString_RJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = runtime.makeHandle(std::move(*strRes));

  // 4. Let A be ! ArrayCreate(0).
  auto arrRes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto A = *arrRes;
  // 5. Let lengthA be 0.
  uint32_t lengthA = 0;

  // 6. If limit is undefined, let lim be 232 - 1; else let lim be ?
  // ToUint32(limit).
  auto limit = args.getArgHandle(1);
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

  // 7. Let s be the length of S.
  uint32_t s = S->getStringLength();
  // 8. Let p be 0.
  // End of the last match.
  uint32_t p = 0;

  // 9. Let R be ? ToString(separator).
  // The pattern which we want to separate on.
  auto sepRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(sepRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> R = runtime.makeHandle(std::move(sepRes.getValue()));

  // 10. If lim = 0, return A.
  if (lim == 0) {
    // Don't want any elements, so we're done.
    return A.getHermesValue();
  }

  // 11. If separator is undefined, then
  if (LLVM_UNLIKELY(separator->isUndefined())) {
    // a. Perform ! CreateDataPropertyOrThrow(A, "0", S).
    (void)JSArray::setElementAt(A, runtime, 0, S);
    if (LLVM_UNLIKELY(
            JSArray::setLengthProperty(A, runtime, 1) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    // b. Return A.
    return A.getHermesValue();
  }

  // 12. If s = 0, then
  if (s == 0) {
    // S is the empty string.
    // a. Let z be SplitMatch(S, 0, R).
    auto matchResult = splitMatch(runtime, S, 0, R);
    // b. If z is not false, return A.
    if (matchResult) {
      return A.getHermesValue();
    }
    // c. Perform ! CreateDataPropertyOrThrow(A, "0", S).
    (void)JSArray::setElementAt(A, runtime, 0, S);
    if (LLVM_UNLIKELY(
            JSArray::setLengthProperty(A, runtime, 1) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    // e. Return A.
    return A.getHermesValue();
  }

  // 17. Let q be p.
  // Place to attempt the start of the next match.
  uint32_t q = p;

  MutableHandle<> tmpHandle{runtime};
  auto marker = gcScope.createMarker();

  // 18. Repeat, while q ≠ s
  // Main loop: continue while we have space to find another match.
  while (q != s) {
    gcScope.flushToMarker(marker);

    // a. Let e be SplitMatch(S, q, R).
    // b. If e is false, let q = q+1.

    // Find the next valid match. We know that q < s.
    // ES6's SplitMatch only finds matches at q, but we find matches at or
    // after q, so if it fails, we know we're done. In effect, we are performing
    // steps (a) and (b) over and over again.
    auto matchResult = splitMatch(runtime, S, q, R);
    // If we did find a match, fast-forward q to the start of that match.
    if (matchResult) {
      q = *matchResult - R->getStringLength();
    }
    if (!matchResult || q >= s) {
      // There's no matches between index q and the end of the string, so we're
      // done searching. Note: This behavior differs from the spec
      // implementation, because we check for matches at or after q. However, in
      // line with the spec, we only count matches that start before the end of
      // the string.
      break;
    }
    // c. Else,
    // Found a match, so go ahead and update e, such that the match is the range
    // [q,e).
    uint32_t e = *matchResult;
    // i. If e = p, set q to q + 1.
    if (e == p) {
      // The end of this match is the same as the end of the last match,
      // so we matched with the empty string.
      // We don't want to match the empty string at this location again,
      // so increment q in order to start the next search at the next position.
      q++;
    }
    // ii. Else,
    else {
      // Found a non-empty string match.
      // Add everything from the last match to the current one to A.
      // This has length q-p because q is the start of the current match,
      // and p was the end (exclusive) of the last match.

      // 1. Let T be the String value equal to the substring of S consisting of
      // the code units at indices p (inclusive) through q (exclusive).
      auto strRes = StringPrimitive::slice(runtime, S, p, q - p);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      tmpHandle = *strRes;
      // 2. Perform ! CreateDataPropertyOrThrow(A, ! ToString(lengthA), T)
      JSArray::setElementAt(A, runtime, lengthA, tmpHandle);
      // 3. Set lengthA to lengthA + 1.
      ++lengthA;

      // 4. If lengthA = lim, return A.
      if (lengthA == lim) {
        // Reached the limit, return early.
        if (LLVM_UNLIKELY(
                JSArray::setLengthProperty(A, runtime, lengthA) ==
                ExecutionStatus::EXCEPTION))
          return ExecutionStatus::EXCEPTION;
        return A.getHermesValue();
      }
      // 5. Set p to e.
      // Update p to point to the end of this match, maintaining the
      // invariant that it points to the end of the last match encountered.
      p = e;
      // 6. Set q to p.
      // Start position of the next search is updated to the end of this match.
      q = p;
    }
  }

  // 15. Let T be the String value equal to the substring of S consisting of the
  // code units at indices p (inclusive) through s (exclusive).
  // Add the rest of the string (after the last match) to A.
  auto elementStrRes = StringPrimitive::slice(runtime, S, p, s - p);
  if (LLVM_UNLIKELY(elementStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = *elementStrRes;
  // 16. Perform ! CreateDataPropertyOrThrow(A, ! ToString(lengthA), T).
  JSArray::setElementAt(A, runtime, lengthA, tmpHandle);
  ++lengthA;

  if (LLVM_UNLIKELY(
          JSArray::setLengthProperty(A, runtime, lengthA) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  // 27. Return A.
  return A.getHermesValue();
}

CallResult<HermesValue> stringPrototypeIncludesOrStartsWith(
    void *ctx,
    Runtime &runtime,
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
  auto S = runtime.makeHandle(std::move(*strRes));

  // 4. Let isRegExp be IsRegExp(searchString).
  // 6. If isRegExp is true, throw a TypeError exception.
  auto isRegExpRes = isRegExp(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(isRegExpRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_UNLIKELY(*isRegExpRes)) {
    return runtime.raiseTypeError(
        "First argument to startsWith and includes must not be a RegExp");
  }

  // 7. Let searchStr be ToString(searchString).
  auto searchStrRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(searchStrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto searchStr = runtime.makeHandle(std::move(*searchStrRes));

  // 9. Let pos be ToIntegerOrInfinity(position).
  // (If position is undefined, this step produces the value 0).
  auto posRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
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
  auto SView = StringPrimitive::createStringView(runtime, S);
  auto searchStrView = StringPrimitive::createStringView(runtime, searchStr);
  auto foundIter = std::search(
      SView.begin() + start,
      SView.end(),
      searchStrView.begin(),
      searchStrView.end());
  // Note: searchStrView.empty check is needed in the special case that S is
  // empty, searchStr is empty, and start = 0
  return HermesValue::encodeBoolValue(
      foundIter != SView.end() || searchStrView.empty());
}

CallResult<HermesValue>
stringPrototypeIndexOf(void *, Runtime &runtime, NativeArgs args) {
  auto searchString = args.getArgHandle(0);
  auto position = args.getArgHandle(1);
  return stringDirectedIndexOf(
      runtime, args.getThisHandle(), searchString, position, false);
}

CallResult<HermesValue>
stringPrototypeLastIndexOf(void *, Runtime &runtime, NativeArgs args) {
  auto searchString = args.getArgHandle(0);
  auto position = args.getArgHandle(1);
  return stringDirectedIndexOf(
      runtime, args.getThisHandle(), searchString, position, true);
}

} // namespace vm
} // namespace hermes
