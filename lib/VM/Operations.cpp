/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Operations.h"

#include "hermes/Support/Conversions.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSCallableProxy.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

#include "dtoa/dtoa.h"

#include "llvh/ADT/SmallString.h"

#include <cfloat>
#include <cmath>

namespace hermes {
namespace vm {

CallResult<Handle<SymbolID>> stringToSymbolID(
    Runtime &runtime,
    PseudoHandle<StringPrimitive> strPrim) {
  // Unique the string.
  return runtime.getIdentifierTable().getSymbolHandleFromPrimitive(
      runtime, std::move(strPrim));
}

CallResult<Handle<SymbolID>> valueToSymbolID(
    Runtime &runtime,
    Handle<> nameValHnd) {
  if (nameValHnd->isSymbol()) {
    return Handle<SymbolID>::vmcast(nameValHnd);
  }
  // Convert the value to a string.
  auto res = toString_RJS(runtime, nameValHnd);
  if (res == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  // Unique the string.
  return stringToSymbolID(runtime, std::move(*res));
}

HermesValue typeOf(Runtime &runtime, Handle<> valueHandle) {
  switch (valueHandle->getETag()) {
    case HermesValue::ETag::Undefined:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::undefined));
    case HermesValue::ETag::Null:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::object));
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::string));
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::bigint));
    case HermesValue::ETag::Bool:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::boolean));
    case HermesValue::ETag::Symbol:
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::symbol));
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      if (vmisa<Callable>(*valueHandle))
        return HermesValue::encodeStringValue(
            runtime.getPredefinedString(Predefined::function));
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::object));
    default:
      assert(valueHandle->isNumber() && "Invalid type.");
      return HermesValue::encodeStringValue(
          runtime.getPredefinedString(Predefined::number));
  }
}

OptValue<uint32_t> toArrayIndex(
    Runtime &runtime,
    Handle<StringPrimitive> strPrim) {
  auto view = StringPrimitive::createStringView(runtime, strPrim);
  return toArrayIndex(view);
}

OptValue<uint32_t> toArrayIndex(StringView str) {
  auto len = str.length();
  if (str.isASCII()) {
    const char *ptr = str.castToCharPtr();
    return hermes::toArrayIndex(ptr, ptr + len);
  }
  const char16_t *ptr = str.castToChar16Ptr();
  return hermes::toArrayIndex(ptr, ptr + len);
}

bool isSameValue(HermesValue x, HermesValue y) {
  if (x.getTag() != y.getTag()) {
    // If the tags are different, they must be different.
    return false;
  }
  assert(
      !x.isEmpty() && !x.isNativeValue() &&
      "Empty and Native Value cannot be compared");

  // Strings require deep comparison.
  if (x.isString()) {
    // For strings, we compare each character in sequence.
    return x.getString()->equals(y.getString());
  }

  // Bigints also require deep comparison.
  if (x.isBigInt()) {
    // For bigints, perform the numerical comparison.
    return x.getBigInt()->compare(y.getBigInt()) == 0;
  }

  // Otherwise they are identical if the raw bits are the same.
  return x.getRaw() == y.getRaw();
}

bool isSameValueZero(HermesValue x, HermesValue y) {
  if (x.isNumber() && y.isNumber() && x.getNumber() == y.getNumber()) {
    // Takes care of +0 == -0.
    return true;
  }
  return isSameValue(x, y);
}

bool isPrimitive(HermesValue val) {
  assert(!val.isEmpty() && "empty value encountered");
  assert(!val.isNativeValue() && "native value encountered");
  return !val.isObject();
}

CallResult<HermesValue> ordinaryToPrimitive(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    PreferredType preferredType) {
  GCScope gcScope{runtime};
  assert(
      preferredType != PreferredType::NONE &&
      "OrdinaryToPrimitive requires a type hint");

  for (int i = 0; i < 2; ++i) {
    if (preferredType == PreferredType::STRING) {
      auto propRes = JSObject::getNamed_RJS(
          selfHandle, runtime, Predefined::getSymbolID(Predefined::toString));
      if (propRes == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (auto funcHandle = Handle<Callable>::dyn_vmcast(
              runtime.makeHandle(std::move(*propRes)))) {
        auto callRes =
            funcHandle->executeCall0(funcHandle, runtime, selfHandle);
        if (callRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        if (isPrimitive(callRes->get()))
          return callRes.toCallResultHermesValue();
      }

      // This method failed. Try the other one.
      preferredType = PreferredType::NUMBER;
    } else {
      auto propRes = JSObject::getNamed_RJS(
          selfHandle, runtime, Predefined::getSymbolID(Predefined::valueOf));
      if (propRes == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (auto funcHandle = Handle<Callable>::dyn_vmcast(
              runtime.makeHandle(std::move(*propRes)))) {
        auto callRes =
            funcHandle->executeCall0(funcHandle, runtime, selfHandle);
        if (callRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        if (isPrimitive(callRes->get()))
          return callRes.toCallResultHermesValue();
      }

      // This method failed. Try the other one.
      preferredType = PreferredType::STRING;
    }
  }

  // Nothing succeeded, time to give up.
  return runtime.raiseTypeError("Cannot determine default value of object");
}

/// ES5.1 9.1
CallResult<HermesValue>
toPrimitive_RJS(Runtime &runtime, Handle<> valueHandle, PreferredType hint) {
  assert(!valueHandle->isEmpty() && "empty value is not allowed");
  assert(!valueHandle->isNativeValue() && "native value is not allowed");

  if (!valueHandle->isObject())
    return *valueHandle;

  // 4. Let exoticToPrim be GetMethod(input, @@toPrimitive).
  auto exoticToPrim = getMethod(
      runtime,
      valueHandle,
      runtime.makeHandle(
          Predefined::getSymbolID(Predefined::SymbolToPrimitive)));
  if (LLVM_UNLIKELY(exoticToPrim == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 6. If exoticToPrim is not undefined, then
  if (vmisa<Callable>(exoticToPrim->getHermesValue())) {
    auto callable = runtime.makeHandle<Callable>(
        dyn_vmcast<Callable>(exoticToPrim->getHermesValue()));
    CallResult<PseudoHandle<>> resultRes = Callable::executeCall1(
        callable,
        runtime,
        valueHandle,
        HermesValue::encodeStringValue(runtime.getPredefinedString(
            hint == PreferredType::NONE         ? Predefined::defaultStr
                : hint == PreferredType::STRING ? Predefined::string
                                                : Predefined::number)));
    if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    PseudoHandle<> result = std::move(*resultRes);
    if (!result->isObject()) {
      return result.getHermesValue();
    }
    return runtime.raiseTypeError(
        "Symbol.toPrimitive function must return a primitive");
  }

  // 7. If hint is "default", let hint be "number".
  // 8. Return OrdinaryToPrimitive(input,hint).
  return ordinaryToPrimitive(
      Handle<JSObject>::vmcast(valueHandle),
      runtime,
      hint == PreferredType::NONE ? PreferredType::NUMBER : hint);
}

bool toBoolean(HermesValue value) {
  switch (value.getETag()) {
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      llvm_unreachable("invalid value");
#endif // HERMES_SLOW_DEBUG
    case HermesValue::ETag::Empty:
      llvm_unreachable("empty value");
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      llvm_unreachable("native value");
    case HermesValue::ETag::Undefined:
    case HermesValue::ETag::Null:
      return false;
    case HermesValue::ETag::Bool:
      return value.getBool();
    case HermesValue::ETag::Symbol:
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      return true;
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return value.getBigInt()->compare(0) != 0;
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      return value.getString()->getStringLength() != 0;
    default: {
      auto m = value.getNumber();
      return !(m == 0 || std::isnan(m));
    }
  }
}

/// ES5.1 9.8.1
static CallResult<PseudoHandle<StringPrimitive>> numberToString(
    Runtime &runtime,
    double m) LLVM_NO_SANITIZE("float-cast-overflow");

static CallResult<PseudoHandle<StringPrimitive>> numberToString(
    Runtime &runtime,
    double m) {
  char buf8[hermes::NUMBER_TO_STRING_BUF_SIZE];

  // Optimization: Fast-case for positive integers < 2^31
  int32_t n = static_cast<int32_t>(m);
  if (m == static_cast<double>(n) && n > 0) {
    // Write base 10 digits in reverse from end of buf8.
    char *p = buf8 + sizeof(buf8);
    do {
      *--p = '0' + (n % 10);
      n /= 10;
    } while (n);
    size_t len = buf8 + sizeof(buf8) - p;
    // Temporarily stop the propagation of removing.
    auto result = StringPrimitive::create(runtime, ASCIIRef(p, len));
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return createPseudoHandle(vmcast<StringPrimitive>(*result));
  }

  auto getPredefined = [&runtime](Predefined::Str predefinedID) {
    return createPseudoHandle(runtime.getPredefinedString(predefinedID));
  };

  if (std::isnan(m))
    return getPredefined(Predefined::NaN);
  if (m == 0)
    return getPredefined(Predefined::zero);
  if (m == std::numeric_limits<double>::infinity())
    return getPredefined(Predefined::Infinity);
  if (m == -std::numeric_limits<double>::infinity())
    return getPredefined(Predefined::NegativeInfinity);

  // After special cases, run the generic routine to convert.
  size_t len = hermes::numberToString(m, buf8, sizeof(buf8));

  auto result = StringPrimitive::create(runtime, ASCIIRef(buf8, len));
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return createPseudoHandle(vmcast<StringPrimitive>(*result));
}

CallResult<PseudoHandle<StringPrimitive>> toString_RJS(
    Runtime &runtime,
    Handle<> valueHandle) {
  HermesValue value = valueHandle.get();
  StringPrimitive *result;
  switch (value.getETag()) {
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      llvm_unreachable("invalid value");
#endif // HERMES_SLOW_DEBUG
    case HermesValue::ETag::Empty:
      llvm_unreachable("empty value");
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      llvm_unreachable("native value");
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2: {
      const uint8_t kDefaultRadix = 10;
      auto res =
          vmcast<BigIntPrimitive>(value)->toString(runtime, kDefaultRadix);
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      result = res->getString();
      break;
    }
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      result = vmcast<StringPrimitive>(value);
      break;
    case HermesValue::ETag::Undefined:
      result = runtime.getPredefinedString(Predefined::undefined);
      break;
    case HermesValue::ETag::Null:
      result = runtime.getPredefinedString(Predefined::null);
      break;
    case HermesValue::ETag::Bool:
      result = value.getBool()
          ? runtime.getPredefinedString(Predefined::trueStr)
          : runtime.getPredefinedString(Predefined::falseStr);
      break;
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2: {
      auto res = toPrimitive_RJS(runtime, valueHandle, PreferredType::STRING);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return toString_RJS(runtime, runtime.makeHandle(res.getValue()));
    }
    case HermesValue::ETag::Symbol:
      return runtime.raiseTypeError("Cannot convert Symbol to string");
    default:
      return numberToString(runtime, value.getNumber());
  }

  return createPseudoHandle(result);
}

double parseIntWithRadix(const StringView str, int radix) {
  auto res =
      hermes::parseIntWithRadix</* AllowNumericSeparator */ false>(str, radix);
  return res ? res.getValue() : std::numeric_limits<double>::quiet_NaN();
}

/// ES5.1 9.3.1
static inline double stringToNumber(
    Runtime &runtime,
    Handle<StringPrimitive> strPrim) {
  auto &idTable = runtime.getIdentifierTable();

  // Fast check for special values (no extraneous whitespace).
  if (runtime.symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::Infinity), *strPrim)) {
    return std::numeric_limits<double>::infinity();
  }
  if (runtime.symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::PositiveInfinity), *strPrim)) {
    return std::numeric_limits<double>::infinity();
  }
  if (runtime.symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::NegativeInfinity), *strPrim)) {
  }
  if (runtime.symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::NaN), *strPrim)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Trim string to the interval [begin, end).
  auto orig = StringPrimitive::createStringView(runtime, strPrim);
  auto begin = orig.begin();
  auto end = orig.end();

  // Move begin and end to ignore whitespace.
  while (begin != end &&
         (isWhiteSpaceChar(*begin) || isLineTerminatorChar(*begin))) {
    ++begin;
  }
  while (begin != end &&
         (isWhiteSpaceChar(*(end - 1)) || isLineTerminatorChar(*(end - 1)))) {
    --end;
  }
  // Early return for empty strings (strings only containing whitespace).
  if (begin == end) {
    return 0;
  }

  // Trim the string.
  StringView str16 = orig.slice(begin, end);

  // Slow check for special values.
  // This should only run if user created a string with extra whitespace,
  // since normal uses would get caught by the initial check.
  if (LLVM_UNLIKELY(str16.equals(idTable.getStringView(
          runtime, Predefined::getSymbolID(Predefined::Infinity))))) {
    return std::numeric_limits<double>::infinity();
  }
  if (LLVM_UNLIKELY(str16.equals(idTable.getStringView(
          runtime, Predefined::getSymbolID(Predefined::PositiveInfinity))))) {
    return std::numeric_limits<double>::infinity();
  }
  if (LLVM_UNLIKELY(str16.equals(idTable.getStringView(
          runtime, Predefined::getSymbolID(Predefined::NegativeInfinity))))) {
    return -std::numeric_limits<double>::infinity();
  }
  if (LLVM_UNLIKELY(str16.equals(idTable.getStringView(
          runtime, Predefined::getSymbolID(Predefined::NaN))))) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto len = str16.length();

  // Parse hex codes, since dtoa doesn't do it.
  // FIXME: May be inaccurate for some hex values.
  // We need to check other sources first.
  if (len > 2) {
    if (str16[0] == u'0' && letterToLower(str16[1]) == u'x') {
      return parseIntWithRadix(str16.slice(2), 16);
    }
    if (str16[0] == u'0' && letterToLower(str16[1]) == u'o') {
      return parseIntWithRadix(str16.slice(2), 8);
    }
    if (str16[0] == u'0' && letterToLower(str16[1]) == u'b') {
      return parseIntWithRadix(str16.slice(2), 2);
    }
  }

  // Finally, copy 16 bit chars into 8 bit chars and call dtoa.
  llvh::SmallVector<char, 32> str8(len + 1);
  uint32_t i = 0;
  for (auto c16 : str16) {
    // Check to ensure we only have valid number characters now.
    if ((u'0' <= c16 && c16 <= u'9') || c16 == u'.' ||
        letterToLower(c16) == u'e' || c16 == u'+' || c16 == u'-') {
      str8[i] = static_cast<char>(c16);
    } else {
      return std::numeric_limits<double>::quiet_NaN();
    }
    ++i;
  }
  str8[len] = '\0';
  char *endPtr;
  double result = ::hermes_g_strtod(str8.data(), &endPtr);
  if (endPtr == str8.data() + len) {
    return result;
  }

  // If everything failed, return NaN.
  return std::numeric_limits<double>::quiet_NaN();
}

CallResult<HermesValue> toNumber_RJS(Runtime &runtime, Handle<> valueHandle) {
  auto value = valueHandle.get();
  double result;
  switch (value.getETag()) {
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      llvm_unreachable("invalid value");
#endif // HERMES_SLOW_DEBUG
    case HermesValue::ETag::Empty:
      llvm_unreachable("empty value");
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      llvm_unreachable("native value");
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2: {
      auto res = toPrimitive_RJS(runtime, valueHandle, PreferredType::NUMBER);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return toNumber_RJS(runtime, runtime.makeHandle(res.getValue()));
    }
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      result =
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(valueHandle));
      break;
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return runtime.raiseTypeError("Cannot convert BigInt to number");
    case HermesValue::ETag::Undefined:
      result = std::numeric_limits<double>::quiet_NaN();
      break;
    case HermesValue::ETag::Null:
      result = +0.0;
      break;
    case HermesValue::ETag::Bool:
      result = value.getBool();
      break;
    case HermesValue::ETag::Symbol:
      return runtime.raiseTypeError("Cannot convert Symbol to number");
    default:
      // Already have a number, just return it.
      return value;
  }
  return HermesValue::encodeDoubleValue(result);
}

CallResult<HermesValue> toNumeric_RJS(Runtime &runtime, Handle<> valueHandle) {
  GCScopeMarkerRAII marker{runtime};
  auto primValue = toPrimitive_RJS(runtime, valueHandle, PreferredType::NUMBER);

  if (LLVM_UNLIKELY(primValue == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (primValue->isBigInt()) {
    return *primValue;
  }

  return toNumber_RJS(runtime, runtime.makeHandle(*primValue));
}

CallResult<HermesValue> toLength(Runtime &runtime, Handle<> valueHandle) {
  constexpr double maxLength = 9007199254740991.0; // 2**53 - 1
  auto res = toIntegerOrInfinity(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = res->getNumber();
  if (len <= 0) {
    len = 0;
  } else if (len > maxLength) {
    len = maxLength;
  }
  return HermesValue::encodeDoubleValue(len);
}

CallResult<uint64_t> toLengthU64(Runtime &runtime, Handle<> valueHandle) {
  constexpr double highestIntegralDouble =
      ((uint64_t)1 << std::numeric_limits<double>::digits) - 1;
  auto res = toIntegerOrInfinity(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = res->getNumber();
  if (len <= 0) {
    len = 0;
  } else if (len > highestIntegralDouble) {
    len = highestIntegralDouble;
  }
  return len;
}

CallResult<HermesValue> toIndex(Runtime &runtime, Handle<> valueHandle) {
  auto value = (valueHandle->isUndefined())
      ? runtime.makeHandle(HermesValue::encodeDoubleValue(0))
      : valueHandle;
  auto res = toIntegerOrInfinity(runtime, value);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto integerIndex = res->getNumber();
  if (integerIndex < 0) {
    return runtime.raiseRangeError("A negative value cannot be an index");
  }
  auto integerIndexHandle =
      runtime.makeHandle(HermesValue::encodeDoubleValue(integerIndex));
  res = toLength(runtime, integerIndexHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto index = res.getValue();
  if (index.getNumber() != integerIndex) {
    return runtime.raiseRangeError(
        "The value given for the index must be between 0 and 2 ^ 53 - 1");
  }
  return res;
}

CallResult<HermesValue> toIntegerOrInfinity(
    Runtime &runtime,
    Handle<> valueHandle) {
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  double num = res->getNumber();

  double result;
  if (std::isnan(num)) {
    result = 0;
  } else {
    result = std::trunc(num);
  }

  return HermesValue::encodeDoubleValue(result);
}

/// Conversion of HermesValues to integers.
template <typename T>
static inline CallResult<HermesValue> toInt(
    Runtime &runtime,
    Handle<> valueHandle) {
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  double num = res->getNumber();
  T result = static_cast<T>(hermes::truncateToInt32(num));
  return HermesValue::encodeNumberValue(result);
}

CallResult<HermesValue> toInt8(Runtime &runtime, Handle<> valueHandle) {
  return toInt<int8_t>(runtime, valueHandle);
}

CallResult<HermesValue> toInt16(Runtime &runtime, Handle<> valueHandle) {
  return toInt<int16_t>(runtime, valueHandle);
}

CallResult<HermesValue> toInt32_RJS(Runtime &runtime, Handle<> valueHandle) {
  return toInt<int32_t>(runtime, valueHandle);
}

CallResult<HermesValue> toUInt8(Runtime &runtime, Handle<> valueHandle) {
  return toInt<uint8_t>(runtime, valueHandle);
}

uint8_t toUInt8Clamp(double number) {
  // 3. If number is NaN, return +0.
  // 4. If number <= 0, return +0.
  // Not < so that NaN coerces to 0.
  // NOTE: this check correctly rounds numbers less than 0.5
  if (!(number >= 0.5)) {
    return 0;
  }

  // 5. If number >= 255, return 255.
  if (number > 255) {
    return 255;
  }

  // The next steps are the equivalent of the spec's round-to-even requirement.
  // Round up and then do the even/odd check.
  double toTruncate = number + 0.5;
  uint8_t x = static_cast<uint8_t>(toTruncate);

  // If it was a tie (i.e. it ended in 0.5) then
  if (x == toTruncate) {
    // number ended in 0.5 and was rounded up, reduce by 1 if odd,
    // else leave the same.
    // That is the same as unsetting the least significant bit.
    return (x & ~1);
  } else {
    // number did not end in 0.5, don't need to check the parity.
    return x;
  }
}

CallResult<HermesValue> toUInt8Clamp(Runtime &runtime, Handle<> valueHandle) {
  // 1. Let number be toNumber_RJS(argument)
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    // 2. ReturnIfAbrupt(number)
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeNumberValue(toUInt8Clamp(res->getNumber()));
}

CallResult<HermesValue> toUInt16(Runtime &runtime, Handle<> valueHandle) {
  return toInt<uint16_t>(runtime, valueHandle);
}

CallResult<HermesValue> toUInt32_RJS(Runtime &runtime, Handle<> valueHandle) {
  return toInt<uint32_t>(runtime, valueHandle);
}

CallResult<Handle<JSObject>> getPrimitivePrototype(
    Runtime &runtime,
    Handle<> base) {
  switch (base->getETag()) {
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      llvm_unreachable("invalid value");
#endif // HERMES_SLOW_DEBUG
    case HermesValue::ETag::Empty:
      llvm_unreachable("empty value");
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      llvm_unreachable("native value");
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      llvm_unreachable("object value");
    case HermesValue::ETag::Undefined:
      return runtime.raiseTypeError("Cannot convert undefined value to object");
    case HermesValue::ETag::Null:
      return runtime.raiseTypeError("Cannot convert null value to object");
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      return Handle<JSObject>::vmcast(&runtime.stringPrototype);
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return Handle<JSObject>::vmcast(&runtime.bigintPrototype);
    case HermesValue::ETag::Bool:
      return Handle<JSObject>::vmcast(&runtime.booleanPrototype);
    case HermesValue::ETag::Symbol:
      return Handle<JSObject>::vmcast(&runtime.symbolPrototype);
    default:
      assert(base->isNumber() && "Unknown tag in getPrimitivePrototype.");
      return Handle<JSObject>::vmcast(&runtime.numberPrototype);
  }
}

CallResult<HermesValue> toObject(Runtime &runtime, Handle<> valueHandle) {
  auto value = valueHandle.get();
  switch (value.getETag()) {
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      llvm_unreachable("invalid value");
#endif // HERMES_SLOW_DEBUG
    case HermesValue::ETag::Empty:
      llvm_unreachable("empty value");
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      llvm_unreachable("native value");
    case HermesValue::ETag::Undefined:
      return runtime.raiseTypeError("Cannot convert undefined value to object");
    case HermesValue::ETag::Null:
      return runtime.raiseTypeError("Cannot convert null value to object");
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      return value;
    case HermesValue::ETag::Bool:
      return JSBoolean::create(
                 runtime,
                 value.getBool(),
                 Handle<JSObject>::vmcast(&runtime.booleanPrototype))
          .getHermesValue();
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2: {
      auto res = JSBigInt::create(
          runtime,
          Handle<BigIntPrimitive>::vmcast(valueHandle),
          Handle<JSObject>::vmcast(&runtime.bigintPrototype));
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return res->getHermesValue();
    }
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2: {
      auto res = JSString::create(
          runtime,
          Handle<StringPrimitive>::vmcast(valueHandle),
          Handle<JSObject>::vmcast(&runtime.stringPrototype));
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return res->getHermesValue();
    }
    case HermesValue::ETag::Symbol:
      return JSSymbol::create(
                 runtime,
                 *Handle<SymbolID>::vmcast(valueHandle),
                 Handle<JSObject>::vmcast(&runtime.symbolPrototype))
          .getHermesValue();
    default:
      assert(valueHandle->isNumber() && "Unknown tag in toObject.");
      return JSNumber::create(
                 runtime,
                 value.getNumber(),
                 Handle<JSObject>::vmcast(&runtime.numberPrototype))
          .getHermesValue();
  }
}

ExecutionStatus amendPropAccessErrorMsgWithPropName(
    Runtime &runtime,
    Handle<> valueHandle,
    llvh::StringRef operationStr,
    SymbolID id) {
  if (!valueHandle->isNull() && !valueHandle->isUndefined()) {
    // If value is not null/undefined, fall back to the original exception.
    return ExecutionStatus::EXCEPTION;
  }
  assert(!runtime.getThrownValue().isEmpty() && "Error must have been thrown");
  // Clear the error first because we will re-throw.
  runtime.clearThrownValue();

  // Construct an error message that contains the property name.
  llvh::StringRef valueStr = valueHandle->isNull() ? "null" : "undefined";
  return runtime.raiseTypeError(
      TwineChar16("Cannot ") + operationStr + " property '" +
      runtime.getIdentifierTable().getStringView(runtime, id) + "' of " +
      valueStr);
}

/// Implement a BigInt vs. String comparison operation using a user-provided
/// \p comparator. Note that \p leftHandle is a Handle<BigIntPrimitive> to
/// ensure the caller is putting the BigInt in the lhs (and adjusting \p
/// comparator appropriately).
/// \return false if StringToBigInt( \p rightHandle ) is undefined, otherwise
/// returns \p comparator ( \p leftHandle <=> \p righHandle ).
static CallResult<bool> compareBigIntAndString(
    Runtime &runtime,
    Handle<BigIntPrimitive> leftHandle,
    Handle<> rightHandle,
    bool (*comparator)(int)) {
  assert(rightHandle->isString() && "rhs should be string");

  auto bigintRight = stringToBigInt_RJS(runtime, rightHandle);
  if (LLVM_UNLIKELY(bigintRight == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (bigintRight->isUndefined()) { // Non-compliance: should be undefined.
    return false;
  }
  assert(bigintRight->isBigInt() && "stringToBigInt resulted in non-bigint");
  return comparator(leftHandle->compare(bigintRight->getBigInt()));
}

/// Implement a BigInt vs. Number comparison operation using a user-provided
/// \p comparator. Note that \p leftHandle is a Handle<BigIntPrimitive> to
/// ensure the caller is putting the BigInt in the lhs (and adjusting \p
/// comparator appropriately).
/// \return false if \p right is NaN, otherwise returns
/// \p comparator ( \p leftHandle <=> \p righHandle ).
static CallResult<bool> compareBigIntAndNumber(
    Runtime &runtime,
    Handle<BigIntPrimitive> leftHandle,
    double right,
    bool (*comparator)(int)) {
  switch (std::fpclassify(right)) {
    case FP_NAN:
      // BigInt comparison to NaN is always false.
      return false;
    case FP_INFINITE:
      // If rhs is +infinite, it is greater than lhs; otherwise, it is less than
      // rhs.
      return comparator(right > 0 ? -1 : 1);
    default:
      break;
  }

  // Split the rhs into integral and fractional parts.
  double integralPart;
  const double fractionalPart = std::modf(right, &integralPart);

  // Now use the rhs' integral part to create a new BigInt, which is compared to
  // lhs.
  auto rightHandle = BigIntPrimitive::fromDouble(runtime, integralPart);
  if (LLVM_UNLIKELY(rightHandle == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // If rhs' integral part is different than lhs, then use the integral parts'
  // comparison to decide the result.
  if (int comparisonResult = leftHandle->compare(rightHandle->getBigInt())) {
    return comparator(comparisonResult);
  }

  // Lhs' and rhs' integral parts are equal, thus resort the rhs' fractional
  // part.
  if (fractionalPart != 0) {
    //  If rhs is negative, then it is smaller than lhs; otherwise, it is
    //  greater.
    return comparator(right < 0 ? 1 : -1);
  }

  // Lhs' and rhs' integral parts are equal, and rhs does not have a fractional
  // part (it is zero), thus they are equal.
  return comparator(0);
}

/// Implement a comparison operator. First both operands a converted to
/// primitives. If they both end up being strings, a lexicographical comparison
/// is performed. Otherwise both operands are converted to numbers and the
/// values are compared.
/// \param oper is the comparison operator to use when comparing numbers.
#define IMPLEMENT_COMPARISON_OP(name, oper)                           \
  CallResult<bool> name(                                              \
      Runtime &runtime, Handle<> leftHandle, Handle<> rightHandle) {  \
    auto resLeft =                                                    \
        toPrimitive_RJS(runtime, leftHandle, PreferredType::NUMBER);  \
    if (resLeft == ExecutionStatus::EXCEPTION)                        \
      return ExecutionStatus::EXCEPTION;                              \
    MutableHandle<> left(runtime, resLeft.getValue());                \
                                                                      \
    auto resRight =                                                   \
        toPrimitive_RJS(runtime, rightHandle, PreferredType::NUMBER); \
    if (resRight == ExecutionStatus::EXCEPTION)                       \
      return ExecutionStatus::EXCEPTION;                              \
    MutableHandle<> right(runtime, resRight.getValue());              \
                                                                      \
    /* If both are strings, we must do a string comparison.*/         \
    if (left->isString() && right->isString()) {                      \
      return left->getString()->compare(right->getString()) oper 0;   \
    }                                                                 \
                                                                      \
    if (left->isBigInt() && right->isString()) {                      \
      return compareBigIntAndString(                                  \
          runtime,                                                    \
          Handle<BigIntPrimitive>::vmcast(left),                      \
          right,                                                      \
          [](int result) { return result oper 0; });                  \
    }                                                                 \
                                                                      \
    if (left->isString() && right->isBigInt()) {                      \
      return compareBigIntAndString(                                  \
          runtime,                                                    \
          Handle<BigIntPrimitive>::vmcast(right),                     \
          left,                                                       \
          [](int result) { return -result oper 0; });                 \
    }                                                                 \
                                                                      \
    /* Convert both to a number and compare the numbers. */           \
    resLeft = toNumeric_RJS(runtime, left);                           \
    if (resLeft == ExecutionStatus::EXCEPTION)                        \
      return ExecutionStatus::EXCEPTION;                              \
    left = resLeft.getValue();                                        \
    resRight = toNumeric_RJS(runtime, right);                         \
    if (resRight == ExecutionStatus::EXCEPTION)                       \
      return ExecutionStatus::EXCEPTION;                              \
    right = resRight.getValue();                                      \
                                                                      \
    if (left->isNumber() && right->isNumber()) {                      \
      return left->getNumber() oper right->getNumber();               \
    } else if (left->isBigInt() && right->isBigInt()) {               \
      return left->getBigInt()->compare(right->getBigInt()) oper 0;   \
    }                                                                 \
                                                                      \
    if (left->isBigInt() && right->isNumber()) {                      \
      return compareBigIntAndNumber(                                  \
          runtime,                                                    \
          Handle<BigIntPrimitive>::vmcast(left),                      \
          right->getNumber(),                                         \
          [](int result) { return result oper 0; });                  \
    }                                                                 \
    assert(                                                           \
        left->isNumber() && right->isBigInt() &&                      \
        "expecting one number and one bigint");                       \
    return compareBigIntAndNumber(                                    \
        runtime,                                                      \
        Handle<BigIntPrimitive>::vmcast(right),                       \
        left->getNumber(),                                            \
        [](int result) { return -result oper 0; });                   \
  }

IMPLEMENT_COMPARISON_OP(lessOp_RJS, <);
IMPLEMENT_COMPARISON_OP(greaterOp_RJS, >);
IMPLEMENT_COMPARISON_OP(lessEqualOp_RJS, <=);
IMPLEMENT_COMPARISON_OP(greaterEqualOp_RJS, >=);

/// ES11 7.2.15 Abstract Equality Comparison
CallResult<bool>
abstractEqualityTest_RJS(Runtime &runtime, Handle<> xHandle, Handle<> yHandle) {
  MutableHandle<> x{runtime, xHandle.get()};
  MutableHandle<> y{runtime, yHandle.get()};

  while (true) {
    // Combine tags for use in the switch statement. Use NativeValueTag as a
    // placeholder for numbers.
    assert(
        !x->isNativeValue() && !x->isEmpty() && "invalid value for comparison");
    assert(
        !y->isNativeValue() && !y->isEmpty() && "invalid value for comparison");

    // The following macros are used to generate the switch cases using
    // HermesValue::combineETags; an S in the name means it is a single ETag
    // (e.g., ETag::Bool), while M means it is a multi ETag (e.g., ETag::Object1
    // and ETag::Object2).
#define CASE_S_S(typeA, typeB)    \
  case HermesValue::combineETags( \
      HermesValue::ETag::typeA, HermesValue::ETag::typeB):

#define CASE_S_M(typeA, typeB) \
  CASE_S_S(typeA, typeB##1)    \
  CASE_S_S(typeA, typeB##2)

#define CASE_M_S(typeA, typeB) \
  CASE_S_S(typeA##1, typeB)    \
  CASE_S_S(typeA##2, typeB)

#define CASE_M_M(typeA, typeB) \
  CASE_M_S(typeA, typeB##1)    \
  CASE_M_S(typeA, typeB##2)

// NUMBER_TAG is a "virtual" ETag member that is used to tag numbers (which
// don't have a tag assigned to them). It reuses ETag::Native1 there will
// never be any native values in this part of the code.
#define NUMBER_TAG Native1

    // Tag numbers as with the "virtual" ETag member NUMBER_TAG, and use default
    // tag values for everything else.
    HermesValue::ETag xType =
        x->isNumber() ? HermesValue::ETag::NUMBER_TAG : x->getETag();
    HermesValue::ETag yType =
        y->isNumber() ? HermesValue::ETag::NUMBER_TAG : y->getETag();

    switch (HermesValue::combineETags(xType, yType)) {
      // 1. If Type(x) is the same as Type(y), then
      // a. Return the result of performing Strict Equality Comparison x === y.
      CASE_S_S(Undefined, Undefined)
      CASE_S_S(Null, Null) {
        return true;
      }
      CASE_S_S(NUMBER_TAG, NUMBER_TAG) {
        return x->getNumber() == y->getNumber();
      }
      CASE_M_M(Str, Str) {
        return x->getString()->equals(y->getString());
      }
      CASE_M_M(BigInt, BigInt) {
        return x->getBigInt()->compare(y->getBigInt()) == 0;
      }
      CASE_S_S(Bool, Bool)
      CASE_S_S(Symbol, Symbol)
      CASE_M_M(Object, Object) {
        return x->getRaw() == y->getRaw();
      }
      // 2. If x is null and y is undefined, return true.
      // 3. If x is undefined and y is null, return true.
      CASE_S_S(Undefined, Null)
      CASE_S_S(Null, Undefined) {
        return true;
      }
      // 4. If Type(x) is Number and Type(y) is String, return the result of the
      // comparison x == ! ToNumber(y).
      CASE_S_M(NUMBER_TAG, Str) {
        return x->getNumber() ==
            stringToNumber(runtime, Handle<StringPrimitive>::vmcast(y));
      }
      // 5. If Type(x) is String and Type(y) is Number, return the result of the
      // comparison ! ToNumber(x) == y.
      CASE_M_S(Str, NUMBER_TAG) {
        return stringToNumber(runtime, Handle<StringPrimitive>::vmcast(x)) ==
            y->getNumber();
      }
      // 6. If Type(x) is BigInt and Type(y) is String, then
      CASE_M_M(BigInt, Str) {
        // a. Let n be ! StringToBigInt(y).
        auto n = stringToBigInt_RJS(runtime, y);
        if (LLVM_UNLIKELY(n == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        // b. If n is NaN, return false.
        // N.B.: this has been amended in ES2023 to read
        //       If n is undefined, return false.
        if (n->isUndefined()) {
          return false;
        }
        // c. Return the result of the comparison x == n.
        y = n.getValue();
        break;
      }
      // 7. If Type(x) is String and Type(y) is BigInt, return the result of the
      // comparison y == x.
      CASE_M_M(Str, BigInt) {
        std::swap(x, y);
        break;
      }
      // 8. If Type(x) is Boolean, return the result of the comparison !
      // ToNumber(x) == y.
      CASE_S_S(Bool, NUMBER_TAG) {
        // Do both conversions and check numerical equality.
        return x->getBool() == y->getNumber();
      }
      CASE_S_M(Bool, Str) {
        // Do string parsing and check double equality.
        return x->getBool() ==
            stringToNumber(runtime, Handle<StringPrimitive>::vmcast(y));
      }
      CASE_S_M(Bool, BigInt) {
        return y->getBigInt()->compare(static_cast<int32_t>(x->getBool())) == 0;
      }
      CASE_S_M(Bool, Object) {
        x = HermesValue::encodeDoubleValue(x->getBool());
        break;
      }
      // 9. If Type(y) is Boolean, return the result of the comparison x == !
      // ToNumber(y).
      CASE_S_S(NUMBER_TAG, Bool) {
        return x->getNumber() == y->getBool();
      }
      CASE_M_S(Str, Bool) {
        return stringToNumber(runtime, Handle<StringPrimitive>::vmcast(x)) ==
            y->getBool();
      }
      CASE_M_S(BigInt, Bool) {
        return x->getBigInt()->compare(static_cast<int32_t>(y->getBool())) == 0;
      }
      CASE_M_S(Object, Bool) {
        y = HermesValue::encodeDoubleValue(y->getBool());
        break;
      }
      // 10. If Type(x) is either String, Number, BigInt, or Symbol and Type(y)
      // is Object, return the result of the comparison x == ToPrimitive(y).
      CASE_M_M(Str, Object)
      CASE_M_M(BigInt, Object)
      CASE_S_M(Symbol, Object)
      CASE_S_M(NUMBER_TAG, Object) {
        auto status = toPrimitive_RJS(runtime, y, PreferredType::NONE);
        if (status == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        y = status.getValue();
        break;
      }
      // 11. If Type(x) is Object and Type(y) is either String, Number, BigInt,
      // or Symbol, return the result of the comparison ToPrimitive(x) == y.
      CASE_M_M(Object, Str)
      CASE_M_M(Object, BigInt)
      CASE_M_S(Object, Symbol)
      CASE_M_S(Object, NUMBER_TAG) {
        auto status = toPrimitive_RJS(runtime, x, PreferredType::NONE);
        if (status == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        x = status.getValue();
        break;
      }
      // 12. If Type(x) is BigInt and Type(y) is Number, or if Type(x) is
      // Number and Type(y) is BigInt, then a. If x or y are any of NaN, +∞,
      // or -∞, return false. b. If the mathematical value of x is equal to
      // the mathematical value of y, return true; otherwise return false.
      CASE_M_S(BigInt, NUMBER_TAG) {
        std::swap(x, y);
        LLVM_FALLTHROUGH;
      }
      CASE_S_M(NUMBER_TAG, BigInt) {
        if (!isIntegralNumber(x->getNumber())) {
          return false;
        }

        auto xAsBigInt = BigIntPrimitive::fromDouble(runtime, x->getNumber());
        if (LLVM_UNLIKELY(xAsBigInt == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        return xAsBigInt->getBigInt()->compare(y->getBigInt()) == 0;
      }

      // 13. Return false.
      default:
        return false;
    }

#undef CASE_S_S
#undef CASE_S_M
#undef CASE_M_S
#undef CASE_M_M
#undef NUMBER_TAG
  }
}

bool strictEqualityTest(HermesValue x, HermesValue y) {
  // Numbers are special because they can have different tags and they don't
  // obey bit-exact equality (because of NaN).
  if (x.isNumber())
    return y.isNumber() && x.getNumber() == y.getNumber();
  // If they are not numbers and are bit exact, they must be the same.
  if (x.getRaw() == y.getRaw())
    return true;
  // All the rest of the cases need to have the same tags.
  if (x.getTag() != y.getTag())
    return false;
  // Strings need deep comparison.
  if (x.isString())
    return x.getString()->equals(y.getString());

  // The only remaining case is bigint, which also needs a deep comparison.
  return x.isBigInt() && x.getBigInt()->compare(y.getBigInt()) == 0;
}

CallResult<HermesValue>
addOp_RJS(Runtime &runtime, Handle<> xHandle, Handle<> yHandle) {
  auto resX = toPrimitive_RJS(runtime, xHandle, PreferredType::NONE);
  if (resX == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto xPrim = runtime.makeHandle(resX.getValue());

  auto resY = toPrimitive_RJS(runtime, yHandle, PreferredType::NONE);
  if (resY == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto yPrim = runtime.makeHandle(resY.getValue());

  // If one of the values is a string, concatenate as strings.
  if (xPrim->isString() || yPrim->isString()) {
    auto resX = toString_RJS(runtime, xPrim);
    if (resX == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto xStr = runtime.makeHandle(std::move(*resX));

    auto resY = toString_RJS(runtime, yPrim);
    if (resY == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto yStr = runtime.makeHandle(std::move(*resY));

    return StringPrimitive::concat(runtime, xStr, yStr);
  }

  // xPrim and yPrim are primitives; hence, they are already bigints, or they
  // will never be bigints.
  if (LLVM_LIKELY(!xPrim->isBigInt())) {
    // xPrim is not a bigint; thus this is Number + Number.
    auto res = toNumber_RJS(runtime, xPrim);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    const double xNum = res->getNumber();
    // N.B.: toNumber(yPrim) will raise an TypeError if yPrim is bigint, which
    // is the correct exception to be raised when trying to perform
    // Number + BigInt. This avoids the need to check if yPrim is a bigint.
    res = toNumber_RJS(runtime, yPrim);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    const double yNum = res->getNumber();
    return HermesValue::encodeDoubleValue(xNum + yNum);
  }

  // yPrim is a primitive; therefore it is already a BigInt, or it will never be
  // one.
  if (!yPrim->isBigInt()) {
    return runtime.raiseTypeErrorForValue(
        "Cannot convert ", yHandle, " to BigInt");
  }

  return BigIntPrimitive::add(
      runtime,
      runtime.makeHandle(resX->getBigInt()),
      runtime.makeHandle(resY->getBigInt()));
}

static const size_t MIN_RADIX = 2;
static const size_t MAX_RADIX = 36;

static inline char toRadixChar(unsigned x, unsigned radix) {
  const char chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  static_assert(sizeof(chars) - 1 == MAX_RADIX, "Invalid chars array");
  assert(
      x < radix && x < std::strlen(chars) &&
      "invalid number to radix conversion");
  return chars[x];
}

/// \return the exponent component of the double \p x.
static inline int doubleExponent(double x) {
  int e;
  std::frexp(x, &e);
  return e;
}

Handle<StringPrimitive>
numberToStringWithRadix(Runtime &runtime, double number, unsigned radix) {
  (void)MIN_RADIX;
  (void)MAX_RADIX;
  assert(MIN_RADIX <= radix && radix <= MAX_RADIX && "Invalid radix");
  // Two parts of the final result: integer part and fractional part.
  llvh::SmallString<64> result{};

  // Used to store just the fractional part of the string (not including '.').
  llvh::SmallString<32> fStr{};

  // If negative, treat as if positive and add a '-' later.
  bool negative = false;
  if (number < 0) {
    negative = true;
    number = -number;
  }

  // Split number into integer and fractional parts.
  double iPart;
  double fPart = std::modf(number, &iPart);

  // If there's a fractional part, convert it and store in fStr.
  if (fPart != 0) {
    // Distance to the next double value.
    double next =
        std::nextafter(number, std::numeric_limits<double>::infinity());
    double minDenorm =
        std::nextafter(0.0, std::numeric_limits<double>::infinity());

    // Precision of the input (half the distance to the next double).
    // We only compute digits up to that precision.
    // Ensure that delta > 0 by clamping it by the min denormalized positive
    // double number.
    double delta = std::max(0.5 * (next - number), minDenorm);

    while (fPart > delta) {
      // Multiply by radix to find the next digit.
      fPart *= radix;
      delta *= radix;
      // Write the next digit.
      unsigned digit = static_cast<unsigned>(fPart);
      fStr.push_back(toRadixChar(digit, radix));
      // Remove current digit from fPart to prepare for next iteration.
      fPart -= digit;
      // Round-to-even.
      if (fPart > 0.5 || (fPart == 0.5 && (digit & 1))) {
        // Must round up, necessitating changing written digits.
        if (fPart + delta > 1) {
          // Round because printing the next closest double would not give
          // closer results than rounding. The distance between the next
          // double and this one is large enough that at this point, we're
          // doing worse than rounding up if we were to print out the next
          // double precisely.
          while (true) {
            // Rounding requires backtracking to fix everything up.
            if (fStr.size() == 0) {
              // Rounding failed to stop in the fractional part,
              // so carry over to the integral part.
              ++iPart;
              break;
            }
            // Iterator to the last digit of the string.
            char &c = fStr.back();
            unsigned digitForC = c <= '9' ? c - '0' : c - 'a' + 10;
            if (digitForC + 1 < radix) {
              // Can increment this digit, and we're done.
              c = toRadixChar(digitForC + 1, radix);
              break;
            }
            // We weren't able to increment, so this will be a trailing 0,
            // which we don't want to keep around anyway. So, pop the last
            // digit off the string and continue on.
            fStr.pop_back();
          }
          // Rounded off the number, done writing the fractional string.
          break;
        }
      }
    }
  }

  // Now, create the integer part.
  if (iPart == 0) {
    result.push_back('0');
  } else {
    // Write the number backwards, then reverse it. This simplifies the code.

    // Physical mantissa size.
    // Hidden bit is not included because it's not after the decimal point in
    // the binary scientific notation representation of a double.
    // We use this to calculate whether we have the precision required to know
    // what the next digit is going to be.
    constexpr const int MANTISSA_SIZE = DBL_MANT_DIG - 1;

    // Handle trailing zeros.
    while (doubleExponent(iPart / radix) > MANTISSA_SIZE) {
      // (iPart / radix) doesn't have enough precision to be useful here,
      // because its exponent is larger than the number of bits that the
      // mantissa can encode. So, just put a trailing zero, divide by radix,
      // and move on to the next digit.
      result.push_back('0');
      iPart /= radix;
    }

    // Print the rest of the string when we know we have enough precision to
    // do so.
    while (iPart > 0) {
      // Cast digit to int because we know 2 <= digit <= 36.
      int digit = static_cast<int>(std::fmod(iPart, radix));
      result.push_back(toRadixChar(digit, radix));
      iPart = (iPart - digit) / radix;
    }

    // Int string was generated in reverse, so flip it.
    std::reverse(result.begin(), result.end());
  }

  // Concatenate the fractional string on if it exists.
  if (!fStr.empty()) {
    result += '.';
    result += fStr;
  }

  // Account for negative numbers.
  if (negative) {
    result.insert(result.begin(), '-');
  }

  return runtime.makeHandle<StringPrimitive>(runtime.ignoreAllocationFailure(
      StringPrimitive::create(runtime, result)));
}

CallResult<PseudoHandle<>>
getMethod(Runtime &runtime, Handle<> O, Handle<> key) {
  GCScopeMarkerRAII gcScope{runtime};
  auto objRes = toObject(runtime, O);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto obj = runtime.makeHandle<JSObject>(*objRes);
  auto funcRes = JSObject::getComputed_RJS(obj, runtime, key);
  if (LLVM_UNLIKELY(funcRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if ((*funcRes)->isUndefined() || (*funcRes)->isNull()) {
    return PseudoHandle<>::create(HermesValue::encodeUndefinedValue());
  }
  if (!vmisa<Callable>(funcRes->get())) {
    return runtime.raiseTypeError("Could not get callable method from object");
  }
  return funcRes;
}

CallResult<IteratorRecord> getIterator(
    Runtime &runtime,
    Handle<> obj,
    llvh::Optional<Handle<Callable>> methodOpt) {
  MutableHandle<Callable> method{runtime};
  if (LLVM_LIKELY(!methodOpt.hasValue())) {
    auto methodRes = getMethod(
        runtime,
        obj,
        runtime.makeHandle(
            Predefined::getSymbolID(Predefined::SymbolIterator)));
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!vmisa<Callable>(methodRes->getHermesValue())) {
      return runtime.raiseTypeError("iterator method is not callable");
    }
    method = vmcast<Callable>(methodRes->getHermesValue());
  } else {
    method = **methodOpt;
  }
  auto iteratorRes = Callable::executeCall0(method, runtime, obj);
  if (LLVM_UNLIKELY(iteratorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!(*iteratorRes)->isObject())) {
    return runtime.raiseTypeError("iterator is not an object");
  }
  auto iterator = runtime.makeHandle<JSObject>(std::move(*iteratorRes));

  CallResult<PseudoHandle<>> nextMethodRes = JSObject::getNamed_RJS(
      iterator, runtime, Predefined::getSymbolID(Predefined::next));
  if (LLVM_UNLIKELY(nextMethodRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // We perform this check prior to returning, because every function in the JS
  // library which gets an iterator immediately calls the 'next' function.
  if (!vmisa<Callable>(nextMethodRes->get())) {
    return runtime.raiseTypeError("'next' method on iterator must be callable");
  }

  auto nextMethod =
      Handle<Callable>::vmcast(runtime.makeHandle(std::move(*nextMethodRes)));

  return IteratorRecord{iterator, nextMethod};
}

CallResult<PseudoHandle<JSObject>> iteratorNext(
    Runtime &runtime,
    const IteratorRecord &iteratorRecord,
    llvh::Optional<Handle<>> value) {
  GCScopeMarkerRAII marker{runtime};
  auto resultRes = value
      ? Callable::executeCall1(
            iteratorRecord.nextMethod,
            runtime,
            iteratorRecord.iterator,
            value->getHermesValue())
      : Callable::executeCall0(
            iteratorRecord.nextMethod, runtime, iteratorRecord.iterator);
  if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!(*resultRes)->isObject())) {
    return runtime.raiseTypeError("iterator.next() did not return an object");
  }
  return PseudoHandle<JSObject>::vmcast(std::move(*resultRes));
}

CallResult<Handle<JSObject>> iteratorStep(
    Runtime &runtime,
    const IteratorRecord &iteratorRecord) {
  auto resultRes = iteratorNext(runtime, iteratorRecord);
  if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> result = runtime.makeHandle(std::move(*resultRes));
  auto completeRes = JSObject::getNamed_RJS(
      result, runtime, Predefined::getSymbolID(Predefined::done));
  if (LLVM_UNLIKELY(completeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (toBoolean(completeRes->get())) {
    return Runtime::makeNullHandle<JSObject>();
  }
  return result;
}

ExecutionStatus iteratorClose(
    Runtime &runtime,
    Handle<JSObject> iterator,
    Handle<> completion) {
  ExecutionStatus completionStatus = completion->isEmpty()
      ? ExecutionStatus::RETURNED
      : ExecutionStatus::EXCEPTION;

  // 4. Let innerResult be GetMethod(iterator, "return").
  // Do this lazily: innerResult is only actually used if GetMethod returns
  // a callable which, when called, doesn't throw. Defer storing to innerResult
  // until that point.
  auto returnRes = getMethod(
      runtime,
      iterator,
      runtime.makeHandle(Predefined::getSymbolID(Predefined::returnStr)));

  MutableHandle<> innerResult{runtime};
  if (LLVM_LIKELY(returnRes != ExecutionStatus::EXCEPTION)) {
    if (!vmisa<Callable>(returnRes->getHermesValue())) {
      runtime.setThrownValue(*completion);
      return completionStatus;
    }
    Handle<Callable> returnFn =
        runtime.makeHandle(vmcast<Callable>(returnRes->getHermesValue()));
    auto innerResultRes = Callable::executeCall0(returnFn, runtime, iterator);
    if (LLVM_UNLIKELY(innerResultRes == ExecutionStatus::EXCEPTION)) {
      if (isUncatchableError(runtime.getThrownValue())) {
        // If the call to return threw an uncatchable exception, that overrides
        // the completion, since the point of an uncatchable exception is to
        // prevent more JS from executing.
        return ExecutionStatus::EXCEPTION;
      }
      // If the error is catchable, suppress it temporarily below in lieu
      // of the returnRes exception by writing to innerResultException.
      // Spec text overwrites the value in `innerResult`.
    } else {
      innerResult = std::move(*innerResultRes);
    }
  }
  // Runtime::thrownValue now contains the innerResult's exception if it
  // was thrown.
  // GetMethod error here is deliberately suppressed (no "?" in the spec).
  if (completionStatus == ExecutionStatus::EXCEPTION) {
    // 6. If completion.[[Type]] is throw, return Completion(completion).
    // Note: Overrides the innerResult exception.
    runtime.setThrownValue(*completion);
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!runtime.getThrownValue().isEmpty())) {
    // 7. If innerResult.[[Type]] is throw, return Completion(innerResult).
    // Note: innerResult exception is still in Runtime::thrownValue,
    // so there is no need to set it again.
    return ExecutionStatus::EXCEPTION;
  }
  if (!innerResult->isObject()) {
    // 8. If Type(innerResult.[[Value]]) is not Object,
    //    throw a TypeError exception.
    return runtime.raiseTypeError("iterator.return() did not return an object");
  }
  return ExecutionStatus::RETURNED;
}

bool isUncatchableError(HermesValue value) {
  if (auto *jsError = dyn_vmcast<JSError>(value)) {
    return !jsError->catchable();
  }
  return false;
}

Handle<JSObject>
createIterResultObject(Runtime &runtime, Handle<> value, bool done) {
  auto objHandle = runtime.makeHandle(JSObject::create(runtime));
  auto status = JSObject::defineOwnProperty(
      objHandle,
      runtime,
      Predefined::getSymbolID(Predefined::value),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      value);
  (void)status;
  assert(
      status != ExecutionStatus::EXCEPTION && *status &&
      "put own value property cannot fail");
  status = JSObject::defineOwnProperty(
      objHandle,
      runtime,
      Predefined::getSymbolID(Predefined::done),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      Runtime::getBoolValue(done));
  assert(
      status != ExecutionStatus::EXCEPTION && *status &&
      "put own value property cannot fail");
  return objHandle;
}

CallResult<Handle<Callable>> speciesConstructor(
    Handle<JSObject> O,
    Runtime &runtime,
    Handle<Callable> defaultConstructor) {
  // construct from the "constructor" property in self if that is defined, else
  // use the default one.
  auto res = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::constructor));
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<> cons = std::move(*res);
  if (cons->isUndefined()) {
    return defaultConstructor;
  }
  if (!cons->isObject()) {
    return runtime.raiseTypeError(
        "Constructor must be an object if it is not undefined");
  }
  // There is no @@species (no Symbols yet), so we'll assume that there was
  // no other constructor specified.
  return defaultConstructor;
}

CallResult<bool> isConstructor(Runtime &runtime, HermesValue value) {
  return isConstructor(runtime, dyn_vmcast<Callable>(value));
}

CallResult<bool> isConstructor(Runtime &runtime, Callable *callable) {
  // This is not a complete definition, since ES6 and later define member
  // functions of objects to not be constructors; however, Hermes does not have
  // ES6 classes implemented yet, so we cannot check for that case.
  if (!callable) {
    return false;
  }

  // We traverse the BoundFunction target chain to find the eventual target.
  while (BoundFunction *b = dyn_vmcast<BoundFunction>(callable)) {
    callable = b->getTarget(runtime);
  }

  // If it is a bytecode function, check the flags.
  if (auto *func = dyn_vmcast<JSFunction>(callable)) {
    auto *cb = func->getCodeBlock(runtime);
    // Even though it doesn't make sense logically, we need to compile the
    // function in order to access it flags.
    cb->lazyCompile(runtime);
    return !func->getCodeBlock(runtime)->getHeaderFlags().isCallProhibited(
        true);
  }

  // We check for NativeFunction since those are defined to not be
  // constructible, with the exception of NativeConstructor.
  if (!vmisa<NativeFunction>(callable) || vmisa<NativeConstructor>(callable)) {
    return true;
  }

  // JSCallableProxy is a NativeFunction, but may or may not be a
  // constructor, so we ask it.
  if (auto *cproxy = dyn_vmcast<JSCallableProxy>(callable)) {
    return cproxy->isConstructor(runtime);
  }

  return false;
}

CallResult<bool>
ordinaryHasInstance(Runtime &runtime, Handle<> constructor, Handle<> object) {
  // 1. If IsCallable(C) is false, return false.
  if (!vmisa<Callable>(*constructor)) {
    return false;
  }

  Callable *ctor = vmcast<Callable>(*constructor);

  BoundFunction *bound;
  // 2. If C has a [[BoundTargetFunction]] internal slot, then
  while (LLVM_UNLIKELY(bound = dyn_vmcast<BoundFunction>(ctor))) {
    // 2a. Let BC be the value of C’s [[BoundTargetFunction]] internal slot.
    // 2b. Return InstanceofOperator(O,BC) (see 12.9.4).
    // Note that we can do this with the loop instead,
    // because bound->getTarget() must be a Callable, and Callables cannot
    // redefine @@hasInstance (non-configurable).
    // Callables call this function directly from their @@hasInstance
    // function.
    ctor = bound->getTarget(runtime);
  }

  // At this point 'ctor' is the actual function with a prototype.
  assert(ctor != nullptr && "ctor must not be null");

  // 3. If Type(O) is not Object, return false.
  if (LLVM_UNLIKELY(!object->isObject())) {
    return false;
  }

  // 4. Let P be Get(C, "prototype").
  auto propRes = JSObject::getNamed_RJS(
      runtime.makeHandle(ctor),
      runtime,
      Predefined::getSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 5. If Type(P) is not Object, throw a TypeError exception.
  Handle<JSObject> ctorPrototype = runtime.makeHandle(
      PseudoHandle<JSObject>::dyn_vmcast(std::move(*propRes)));
  if (LLVM_UNLIKELY(!ctorPrototype)) {
    return runtime.raiseTypeError(
        "function's '.prototype' is not an object in 'instanceof'");
  }

  // 6.1.7.3 Invariants of the Essential Internal Methods notes that
  // detection of infinite prototype chains is not enforceable as an
  // invariant if exotic objects exist in the chain.  Most of the
  // time, ScopedNativeDepthTracker will detect this. Here, we need to
  // check that we're not repeating forever.  Since ordinary object
  // chains are verified at the time the parent is set, we count Proxy
  // objects.  Thus, any length chain of ordinary objects is ok.
  constexpr unsigned int kMaxProxyCount = 1024;
  unsigned int proxyCount = 0;
  MutableHandle<JSObject> head{runtime, vmcast<JSObject>(object.get())};
  GCScopeMarkerRAII gcScope{runtime};
  // 6. Repeat
  while (true) {
    // 6a. Let O be O.[[GetPrototypeOf]]().
    CallResult<PseudoHandle<JSObject>> parentRes =
        JSObject::getPrototypeOf(head, runtime);
    if (LLVM_UNLIKELY(parentRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // 6b. If O is null, return false.
    if (!*parentRes) {
      return false;
    }
    // 6c. If SameValue(P, O) is true, return true.
    if (parentRes->get() == ctorPrototype.get()) {
      return true;
    }
    if (head->isProxyObject()) {
      ++proxyCount;
      if (proxyCount > kMaxProxyCount) {
        return runtime.raiseRangeError(
            "Maximum prototype chain length exceeded");
      }
    }
    head = parentRes->get();
    gcScope.flush();
  }
}

CallResult<bool> instanceOfOperator_RJS(
    Runtime &runtime,
    Handle<> object,
    Handle<> constructor) {
  // 1. If Type(C) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!constructor->isObject())) {
    return runtime.raiseTypeError(
        "right operand of 'instanceof' is not an object");
  }

  // Fast path: Function.prototype[Symbol.hasInstance] is non-configurable
  // and non-writable (ES6.0 19.2.3.6), so we directly run its behavior here.
  // Simply call through to ordinaryHasInstance.
  if (vmisa<JSFunction>(*constructor)) {
    return ordinaryHasInstance(runtime, constructor, object);
  }

  // 2. Let instOfHandler be GetMethod(C,@@hasInstance).
  CallResult<PseudoHandle<>> instOfHandlerRes = JSObject::getNamed_RJS(
      Handle<JSObject>::vmcast(constructor),
      runtime,
      Predefined::getSymbolID(Predefined::SymbolHasInstance));
  if (LLVM_UNLIKELY(instOfHandlerRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto instOfHandler = runtime.makeHandle(std::move(*instOfHandlerRes));

  // 4. If instOfHandler is not undefined, then
  if (!instOfHandler->isUndefined()) {
    // 5. Return ToBoolean(Call(instOfHandler, C, «O»)).
    if (!vmisa<Callable>(*instOfHandler)) {
      return runtime.raiseTypeError("instanceof handler must be callable");
    }
    auto callRes = Callable::executeCall1(
        Handle<Callable>::vmcast(instOfHandler), runtime, constructor, *object);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return toBoolean(callRes->get());
  }

  // 6. If IsCallable(C) is false, throw a TypeError exception.
  if (!vmisa<Callable>(*constructor)) {
    return runtime.raiseTypeError(
        "right operand of 'instanceof' is not callable");
  }

  // 7. Return OrdinaryHasInstance(C, O).
  return ordinaryHasInstance(runtime, constructor, object);
}

/// ES6.0 7.2.8
/// Returns true if the object is a JSRegExp or has a Symbol.match property that
/// evaluates to true.
CallResult<bool> isRegExp(Runtime &runtime, Handle<> arg) {
  // 1. If Type(argument) is not Object, return false.
  if (!arg->isObject()) {
    return false;
  }
  Handle<JSObject> obj = Handle<JSObject>::vmcast(arg);
  // 2. Let isRegExp be Get(argument, @@match).
  auto propRes = JSObject::getNamed_RJS(
      obj, runtime, Predefined::getSymbolID(Predefined::SymbolMatch));
  // 3. ReturnIfAbrupt(isRegExp).
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 4. If isRegExp is not undefined, return ToBoolean(isRegExp).
  if (!(*propRes)->isUndefined()) {
    return toBoolean(propRes->get());
  }
  // 5. If argument has a [[RegExpMatcher]] internal slot, return true.
  // 6. Return false.
  return vmisa<JSRegExp>(arg.get());
}

CallResult<Handle<StringPrimitive>> symbolDescriptiveString(
    Runtime &runtime,
    Handle<SymbolID> sym) {
  // 1. Assert: Type(sym) is Symbol.
  // 2. Let desc be sym's [[Description]] value.
  // 3. If desc is undefined, set desc to the empty string.
  // 4. Assert: Type(desc) is String.
  auto desc = runtime.makeHandle<StringPrimitive>(
      runtime.getStringPrimFromSymbolID(*sym));
  SafeUInt32 descLen(desc->getStringLength());
  descLen.add(8);

  // 5. Return the string-concatenation of "Symbol(", desc, and ")".
  auto builder = StringBuilder::createStringBuilder(runtime, descLen);
  if (LLVM_UNLIKELY(builder == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  builder->appendASCIIRef({"Symbol(", 7});
  builder->appendStringPrim(desc);
  builder->appendCharacter(')');

  return builder->getStringPrimitive();
}

CallResult<bool> isArray(Runtime &runtime, JSObject *obj) {
  if (!obj) {
    return false;
  }
  while (true) {
    if (vmisa<JSArray>(obj)) {
      return true;
    }
    if (LLVM_LIKELY(!obj->isProxyObject())) {
      return false;
    }
    if (JSProxy::isRevoked(obj, runtime)) {
      return runtime.raiseTypeError("Proxy has been revoked");
    }
    obj = JSProxy::getTarget(obj, runtime).get();
    assert(obj && "target of non-revoked Proxy is null");
  }
}

CallResult<bool> isConcatSpreadable(Runtime &runtime, Handle<> value) {
  auto O = Handle<JSObject>::dyn_vmcast(value);
  if (!O) {
    return false;
  }

  CallResult<PseudoHandle<>> spreadable = JSObject::getNamed_RJS(
      O,
      runtime,
      Predefined::getSymbolID(Predefined::SymbolIsConcatSpreadable));
  if (LLVM_UNLIKELY(spreadable == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (!(*spreadable)->isUndefined()) {
    return toBoolean(spreadable->get());
  }

  return isArray(runtime, *O);
}

ExecutionStatus toPropertyDescriptor(
    Handle<> obj,
    Runtime &runtime,
    DefinePropertyFlags &flags,
    MutableHandle<> &valueOrAccessor) {
  GCScopeMarkerRAII gcMarker{runtime};

  // Verify that the attributes argument is also an object.
  auto attributes = Handle<JSObject>::dyn_vmcast(obj);
  if (!attributes) {
    return runtime.raiseTypeError(
        "Object.defineProperty() Attributes argument is not an object");
  }

  NamedPropertyDescriptor desc;

  // Get enumerable property of the attributes.
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::enumerable, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::enumerable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.enumerable = toBoolean(propRes->get());
    flags.setEnumerable = true;
  }

  // Get configurable property of the attributes.
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::configurable, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::configurable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.configurable = toBoolean(propRes->get());
    flags.setConfigurable = true;
  }

  // Get value property of the attributes.
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::value, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::value),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    valueOrAccessor = std::move(*propRes);
    flags.setValue = true;
  }

  // Get writable property of the attributes.
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::writable, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::writable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.writable = toBoolean(propRes->get());
    flags.setWritable = true;
  }

  // Get getter property of the attributes.
  MutableHandle<Callable> getterPtr{runtime};
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::get, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::get),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.setGetter = true;
    PseudoHandle<> getter = std::move(*propRes);
    if (LLVM_LIKELY(!getter->isUndefined())) {
      getterPtr = dyn_vmcast<Callable>(getter.get());
      if (LLVM_UNLIKELY(!getterPtr)) {
        return runtime.raiseTypeError(
            "Invalid property descriptor. Getter must be a function.");
      }
    }
  }

  // Get setter property of the attributes.
  MutableHandle<Callable> setterPtr{runtime};
  if (JSObject::getNamedDescriptorPredefined(
          attributes, runtime, Predefined::set, desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::set),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.setSetter = true;
    PseudoHandle<> setter = std::move(*propRes);
    if (LLVM_LIKELY(!setter->isUndefined())) {
      setterPtr = PseudoHandle<Callable>::dyn_vmcast(std::move(setter));
      if (LLVM_UNLIKELY(!setterPtr)) {
        return runtime.raiseTypeError(
            "Invalid property descriptor. Setter must be a function.");
      }
    }
  }

  // Construct property accessor if getter/setter is set.
  if (flags.setSetter || flags.setGetter) {
    if (flags.setValue) {
      return runtime.raiseTypeError(
          "Invalid property descriptor. Can't set both accessor and value.");
    }
    if (flags.setWritable) {
      return runtime.raiseTypeError(
          "Invalid property descriptor. Can't set both accessor and writable.");
    }
    auto crtRes = PropertyAccessor::create(runtime, getterPtr, setterPtr);
    if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    valueOrAccessor = *crtRes;
  }

  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> objectFromPropertyDescriptor(
    Runtime &runtime,
    ComputedPropertyDescriptor desc,
    Handle<> valueOrAccessor) {
  Handle<JSObject> obj = runtime.makeHandle(JSObject::create(runtime));

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();

  if (!desc.flags.accessor) {
    // Data Descriptor
    auto result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::value),
        dpf,
        valueOrAccessor,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::writable),
        dpf,
        Runtime::getBoolValue(desc.flags.writable),
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    // Accessor
    auto *accessor = vmcast<PropertyAccessor>(valueOrAccessor.get());

    auto getter = runtime.makeHandle(
        accessor->getter ? HermesValue::encodeObjectValue(
                               accessor->getter.getNonNull(runtime))
                         : HermesValue::encodeUndefinedValue());

    auto setter = runtime.makeHandle(
        accessor->setter ? HermesValue::encodeObjectValue(
                               accessor->setter.getNonNull(runtime))
                         : HermesValue::encodeUndefinedValue());

    auto result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::get),
        dpf,
        getter,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::set),
        dpf,
        setter,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  auto result = JSObject::defineOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::enumerable),
      dpf,
      Runtime::getBoolValue(desc.flags.enumerable),
      PropOpFlags().plusThrowOnError());
  assert(
      result != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  result = JSObject::defineOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::configurable),
      dpf,
      Runtime::getBoolValue(desc.flags.configurable),
      PropOpFlags().plusThrowOnError());
  assert(
      result != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return obj.getHermesValue();
}

CallResult<HermesValue> numberToBigInt(Runtime &runtime, double number) {
  if (!isIntegralNumber(number)) {
    return runtime.raiseRangeError("number is not integral");
  }

  return BigIntPrimitive::fromDouble(runtime, number);
}

bool isIntegralNumber(double number) {
  // 1. if Type(argument) is not Number, return false
  // it is a number

  // 2. if argument is NaN, +inf, -inf, return false
  if (std::isnan(number) || number == std::numeric_limits<double>::infinity() ||
      number == -std::numeric_limits<double>::infinity()) {
    return false;
  }

  // 3. if floor(abs(R(argument))) != abs(R(argument)) return false
  if (std::floor(std::abs(number)) != std::abs(number)) {
    return false;
  }

  // 4. return true
  return true;
}

CallResult<HermesValue> toBigInt_RJS(Runtime &runtime, Handle<> value) {
  auto prim = toPrimitive_RJS(runtime, value, PreferredType::NUMBER);
  if (LLVM_UNLIKELY(prim == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  switch (prim->getETag()) {
    default:
      break;
    case HermesValue::ETag::Undefined:
      return runtime.raiseTypeError("invalid argument to BigInt()");
    case HermesValue::ETag::Null:
      return runtime.raiseTypeError("invalid argument to BigInt()");
    case HermesValue::ETag::Bool:
      return BigIntPrimitive::fromSigned(runtime, value->getBool() ? 1 : 0);
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return *prim;
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2: {
      auto n = stringToBigInt_RJS(runtime, runtime.makeHandle(*prim));
      if (LLVM_UNLIKELY(n == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (n->isUndefined()) {
        return runtime.raiseSyntaxError("can't convert string to bigint");
      }
      return *n;
    }
    case HermesValue::ETag::Symbol:
      return runtime.raiseTypeError("invalid argument to BigInt()");
  }

  return runtime.raiseTypeError("invalid argument to BigInt()");
}

CallResult<HermesValue> stringToBigInt_RJS(Runtime &runtime, Handle<> value) {
  if (value->isString()) {
    auto str = value->getString();

    std::string outError;
    auto parsedBigInt = str->isASCII()
        ? bigint::ParsedBigInt::parsedBigIntFromStringIntegerLiteral(
              str->getStringRef<char>(), &outError)
        : bigint::ParsedBigInt::parsedBigIntFromStringIntegerLiteral(
              str->getStringRef<char16_t>(), &outError);
    if (!parsedBigInt) {
      return HermesValue::encodeUndefinedValue();
    }

    return BigIntPrimitive::fromBytes(runtime, parsedBigInt->getBytes());
  }

  return runtime.raiseTypeError("Invalid argument to stringToBigInt");
}

CallResult<HermesValue> thisBigIntValue(Runtime &runtime, Handle<> value) {
  switch (value->getTag()) {
    default:
      break;

    case HermesValue::Tag::BigInt:
      return *value;

    case HermesValue::Tag::Object:
      if (auto jsBigInt = Handle<JSBigInt>::dyn_vmcast(value)) {
        if (value->getRaw() != runtime.bigintPrototype.getRaw()) {
          BigIntPrimitive *bigint =
              JSBigInt::getPrimitiveBigInt(jsBigInt.get(), runtime);
          assert(bigint && "boxed bigint is missing its primitive");
          return HermesValue::encodeBigIntValue(bigint);
        }
      }
      break;
  }

  return runtime.raiseTypeError("value is not a bigint");
}

} // namespace vm
} // namespace hermes
