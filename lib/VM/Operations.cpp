/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Operations.h"

#include "hermes/Support/Conversions.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSGenerator.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/dtoa/dtoa.h"

#include "llvm/ADT/SmallString.h"

#include <cfloat>
#include <cmath>

namespace hermes {
namespace vm {

CallResult<Handle<SymbolID>> stringToSymbolID(
    Runtime *runtime,
    PseudoHandle<StringPrimitive> strPrim) {
  // Unique the string.
  return runtime->getIdentifierTable().getSymbolHandleFromPrimitive(
      runtime, std::move(strPrim));
}

CallResult<Handle<SymbolID>> valueToSymbolID(
    Runtime *runtime,
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

HermesValue typeOf(Runtime *runtime, Handle<> valueHandle) {
  switch (valueHandle->getTag()) {
    case UndefinedTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::undefined));
      break;
    case NullTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::object));
      break;
    case StrTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::string));
      break;
    case BoolTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::boolean));
      break;
    case SymbolTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::symbol));
      break;
    case ObjectTag:
      if (vmisa<Callable>(*valueHandle)) {
        return HermesValue::encodeStringValue(
            runtime->getPredefinedString(Predefined::function));
      } else {
        return HermesValue::encodeStringValue(
            runtime->getPredefinedString(Predefined::object));
      }
      break;
    default:
      assert(valueHandle->isNumber() && "Invalid type.");
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::number));
      break;
  }
}

OptValue<uint32_t> toArrayIndex(
    Runtime *runtime,
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
  if (x.isString()) {
    // For strings, we compare each character in sequence.
    return x.getString()->equals(y.getString());
  }
  if (x.isNumber()) {
    // Numbers requires special care due to NaN and +/- 0s.
    auto xNum = x.getNumber();
    auto yNum = y.getNumber();
    if (std::isnan(xNum) && std::isnan(yNum)) {
      return true;
    }
    if (std::signbit(xNum) != std::signbit(yNum)) {
      return false;
    }
    return xNum == yNum;
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
  switch (val.getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case ObjectTag:
      return false;
    case StrTag:
    case BoolTag:
    case NullTag:
    case UndefinedTag:
    case SymbolTag:
    default:
      return true;
  }
}

CallResult<HermesValue> ordinaryToPrimitive(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
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
      if (auto funcHandle =
              Handle<Callable>::dyn_vmcast(runtime->makeHandle(*propRes))) {
        auto callRes =
            funcHandle->executeCall0(funcHandle, runtime, selfHandle);
        if (callRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        if (isPrimitive(*callRes))
          return *callRes;
      }

      // This method failed. Try the other one.
      preferredType = PreferredType::NUMBER;
    } else {
      auto propRes = JSObject::getNamed_RJS(
          selfHandle, runtime, Predefined::getSymbolID(Predefined::valueOf));
      if (propRes == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (auto funcHandle =
              Handle<Callable>::dyn_vmcast(runtime->makeHandle(*propRes))) {
        auto callRes =
            funcHandle->executeCall0(funcHandle, runtime, selfHandle);
        if (callRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        if (isPrimitive(*callRes))
          return *callRes;
      }

      // This method failed. Try the other one.
      preferredType = PreferredType::STRING;
    }
  }

  // Nothing succeeded, time to give up.
  return runtime->raiseTypeError("Cannot determine default value of object");
}

/// ES5.1 9.1
CallResult<HermesValue>
toPrimitive_RJS(Runtime *runtime, Handle<> valueHandle, PreferredType hint) {
  switch (valueHandle->getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case ObjectTag: {
      // 4. Let exoticToPrim be GetMethod(input, @@toPrimitive).
      auto exoticToPrim = getMethod(
          runtime,
          valueHandle,
          runtime->makeHandle(
              Predefined::getSymbolID(Predefined::SymbolToPrimitive)));
      if (LLVM_UNLIKELY(exoticToPrim == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // 6. If exoticToPrim is not undefined, then
      if (vmisa<Callable>(exoticToPrim->getHermesValue())) {
        auto callable = runtime->makeHandle<Callable>(
            dyn_vmcast<Callable>(exoticToPrim->getHermesValue()));
        CallResult<HermesValue> resultRes = Callable::executeCall1(
            callable,
            runtime,
            valueHandle,
            HermesValue::encodeStringValue(runtime->getPredefinedString(
                hint == PreferredType::NONE
                    ? Predefined::defaultStr
                    : hint == PreferredType::STRING ? Predefined::string
                                                    : Predefined::number)));
        if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        if (!resultRes->isObject()) {
          return *resultRes;
        }
        return runtime->raiseTypeError(
            "Symbol.toPrimitive function must return a primitive");
      }

      // 7. If hint is "default", let hint be "number".
      // 8. Return OrdinaryToPrimitive(input,hint).
      return ordinaryToPrimitive(
          Handle<JSObject>::vmcast(valueHandle),
          runtime,
          hint == PreferredType::NONE ? PreferredType::NUMBER : hint);
    }
    case StrTag:
    case BoolTag:
    case NullTag:
    case UndefinedTag:
    case SymbolTag:
    default:
      return *valueHandle;
  }
}

bool toBoolean(HermesValue value) {
  switch (value.getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case NullTag:
    case UndefinedTag:
      return false;
    case BoolTag:
      return value.getBool();
    case SymbolTag:
    case ObjectTag:
      return true;
    case StrTag:
      return value.getString()->getStringLength() != 0;
    default: {
      auto m = value.getNumber();
      return !(m == 0 || std::isnan(m));
    }
  }
}

/// ES5.1 9.8.1
static CallResult<PseudoHandle<StringPrimitive>> numberToString(
    Runtime *runtime,
    double m) LLVM_NO_SANITIZE("float-cast-overflow");

static CallResult<PseudoHandle<StringPrimitive>> numberToString(
    Runtime *runtime,
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

  auto getPredefined = [runtime](Predefined::Str predefinedID) {
    return createPseudoHandle(runtime->getPredefinedString(predefinedID));
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
    Runtime *runtime,
    Handle<> valueHandle) {
  HermesValue value = valueHandle.get();
  StringPrimitive *result;
  switch (value.getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case StrTag:
      result = vmcast<StringPrimitive>(value);
      break;
    case NullTag:
      result = runtime->getPredefinedString(Predefined::null);
      break;
    case UndefinedTag:
      result = runtime->getPredefinedString(Predefined::undefined);
      break;
    case BoolTag:
      result = value.getBool()
          ? runtime->getPredefinedString(Predefined::trueStr)
          : runtime->getPredefinedString(Predefined::falseStr);
      break;
    case ObjectTag: {
      auto res = toPrimitive_RJS(runtime, valueHandle, PreferredType::STRING);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return toString_RJS(runtime, runtime->makeHandle(res.getValue()));
    }
    case SymbolTag:
      return runtime->raiseTypeError("Cannot convert Symbol to string");
    default:
      return numberToString(runtime, value.getNumber());
  }

  return createPseudoHandle(result);
}

double parseIntWithRadix(const StringView str, int radix) {
  auto res = hermes::parseIntWithRadix(str, radix);
  return res ? res.getValue() : std::numeric_limits<double>::quiet_NaN();
}

/// ES5.1 9.3.1
static inline double stringToNumber(
    Runtime *runtime,
    Handle<StringPrimitive> strPrim) {
  auto &idTable = runtime->getIdentifierTable();

  // Fast check for special values (no extraneous whitespace).
  if (runtime->symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::Infinity), *strPrim)) {
    return std::numeric_limits<double>::infinity();
  }
  if (runtime->symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::PositiveInfinity), *strPrim)) {
    return std::numeric_limits<double>::infinity();
  }
  if (runtime->symbolEqualsToStringPrim(
          Predefined::getSymbolID(Predefined::NegativeInfinity), *strPrim)) {
  }
  if (runtime->symbolEqualsToStringPrim(
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
  }

  // Finally, copy 16 bit chars into 8 bit chars and call dtoa.
  llvm::SmallVector<char, 32> str8(len + 1);
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

CallResult<HermesValue> toNumber_RJS(Runtime *runtime, Handle<> valueHandle) {
  auto value = valueHandle.get();
  double result;
  switch (value.getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case ObjectTag: {
      auto res = toPrimitive_RJS(runtime, valueHandle, PreferredType::NUMBER);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return toNumber_RJS(runtime, runtime->makeHandle(res.getValue()));
    }
    case StrTag:
      result =
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(valueHandle));
      break;
    case NullTag:
      result = +0.0;
      break;
    case UndefinedTag:
      result = std::numeric_limits<double>::quiet_NaN();
      break;
    case BoolTag:
      result = value.getBool();
      break;
    case SymbolTag:
      return runtime->raiseTypeError("Cannot convert Symbol to number");
    default:
      // Already have a number, just return it.
      return value;
  }
  return HermesValue::encodeDoubleValue(result);
}

CallResult<HermesValue> toLength(Runtime *runtime, Handle<> valueHandle) {
  constexpr double highestIntegralDouble =
      ((uint64_t)1 << std::numeric_limits<double>::digits) - 1;
  auto res = toInteger(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = res->getNumber();
  if (len <= 0) {
    len = 0;
  } else if (len > highestIntegralDouble) {
    len = highestIntegralDouble;
  }
  return HermesValue::encodeDoubleValue(len);
}

CallResult<uint64_t> toLengthU64(Runtime *runtime, Handle<> valueHandle) {
  constexpr double highestIntegralDouble =
      ((uint64_t)1 << std::numeric_limits<double>::digits) - 1;
  auto res = toInteger(runtime, valueHandle);
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

CallResult<HermesValue> toIndex(Runtime *runtime, Handle<> valueHandle) {
  auto value = (valueHandle->isUndefined())
      ? runtime->makeHandle(HermesValue::encodeDoubleValue(0))
      : valueHandle;
  auto res = toInteger(runtime, value);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto integerIndex = res->getNumber();
  if (integerIndex < 0) {
    return runtime->raiseRangeError("A negative value cannot be an index");
  }
  auto integerIndexHandle =
      runtime->makeHandle(HermesValue::encodeDoubleValue(integerIndex));
  res = toLength(runtime, integerIndexHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto index = res.getValue();
  if (index.getNumber() != integerIndex) {
    return runtime->raiseRangeError(
        "The value given for the index must be between 0 and 2 ^ 53 - 1");
  }
  return res;
}

CallResult<HermesValue> toInteger(Runtime *runtime, Handle<> valueHandle) {
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  double num = res->getNumber();

  double result;
  if (std::isnan(num)) {
    result = 0;
  } else {
    result = oscompat::trunc(num);
  }

  return HermesValue::encodeDoubleValue(result);
}

/// Conversion of HermesValues to integers.
template <typename T>
static inline CallResult<HermesValue> toInt(
    Runtime *runtime,
    Handle<> valueHandle) {
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  double num = res->getNumber();
  T result = static_cast<T>(hermes::truncateToInt32(num));
  return HermesValue::encodeNumberValue(result);
}

CallResult<HermesValue> toInt8(Runtime *runtime, Handle<> valueHandle) {
  return toInt<int8_t>(runtime, valueHandle);
}

CallResult<HermesValue> toInt16(Runtime *runtime, Handle<> valueHandle) {
  return toInt<int16_t>(runtime, valueHandle);
}

CallResult<HermesValue> toInt32_RJS(Runtime *runtime, Handle<> valueHandle) {
  return toInt<int32_t>(runtime, valueHandle);
}

CallResult<HermesValue> toUInt8(Runtime *runtime, Handle<> valueHandle) {
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

CallResult<HermesValue> toUInt8Clamp(Runtime *runtime, Handle<> valueHandle) {
  // 1. Let number be toNumber_RJS(argument)
  auto res = toNumber_RJS(runtime, valueHandle);
  if (res == ExecutionStatus::EXCEPTION) {
    // 2. ReturnIfAbrupt(number)
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeNumberValue(toUInt8Clamp(res->getNumber()));
}

CallResult<HermesValue> toUInt16(Runtime *runtime, Handle<> valueHandle) {
  return toInt<uint16_t>(runtime, valueHandle);
}

CallResult<HermesValue> toUInt32_RJS(Runtime *runtime, Handle<> valueHandle) {
  return toInt<uint32_t>(runtime, valueHandle);
}

CallResult<Handle<JSObject>> getPrimitivePrototype(
    Runtime *runtime,
    Handle<> base) {
  switch (base->getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case ObjectTag:
      llvm_unreachable("object value");
    case NullTag:
      return runtime->raiseTypeError("Cannot convert null value to object");
    case UndefinedTag:
      return runtime->raiseTypeError(
          "Cannot convert undefined value to object");
    case StrTag:
      return Handle<JSObject>::vmcast(&runtime->stringPrototype);
    case BoolTag:
      return Handle<JSObject>::vmcast(&runtime->booleanPrototype);
    case SymbolTag:
      return Handle<JSObject>::vmcast(&runtime->symbolPrototype);
    default:
      assert(base->isNumber() && "Unknown tag in getPrimitivePrototype.");
      return Handle<JSObject>::vmcast(&runtime->numberPrototype);
  }
}

CallResult<HermesValue> toObject(Runtime *runtime, Handle<> valueHandle) {
  auto value = valueHandle.get();
  switch (value.getTag()) {
    case EmptyTag:
      llvm_unreachable("empty value");
    case NativeValueTag:
      llvm_unreachable("native value");
    case NullTag:
      return runtime->raiseTypeError("Cannot convert null value to object");
    case UndefinedTag:
      return runtime->raiseTypeError(
          "Cannot convert undefined value to object");
    case ObjectTag:
      return value;
    case BoolTag:
      return JSBoolean::create(
          runtime,
          value.getBool(),
          Handle<JSObject>::vmcast(&runtime->booleanPrototype));
    case StrTag:
      return JSString::create(
          runtime,
          runtime->makeHandle(value.getString()),
          Handle<JSObject>::vmcast(&runtime->stringPrototype));
    case SymbolTag:
      return JSSymbol::create(
          runtime,
          *Handle<SymbolID>::vmcast(valueHandle),
          Handle<JSObject>::vmcast(&runtime->symbolPrototype));
    default:
      assert(valueHandle->isNumber() && "Unknown tag in toObject.");
      return JSNumber::create(
          runtime,
          value.getNumber(),
          Handle<JSObject>::vmcast(&runtime->numberPrototype));
  }
}

ExecutionStatus amendPropAccessErrorMsgWithPropName(
    Runtime *runtime,
    Handle<> valueHandle,
    llvm::StringRef operationStr,
    SymbolID id) {
  if (!valueHandle->isNull() && !valueHandle->isUndefined()) {
    // If value is not null/undefined, fall back to the original exception.
    return ExecutionStatus::EXCEPTION;
  }
  assert(!runtime->getThrownValue().isEmpty() && "Error must have been thrown");
  // Clear the error first because we will re-throw.
  runtime->clearThrownValue();

  // Construct an error message that contains the property name.
  llvm::StringRef valueStr = valueHandle->isNull() ? "null" : "undefined";
  return runtime->raiseTypeError(
      TwineChar16("Cannot ") + operationStr + " property '" +
      runtime->getIdentifierTable().getStringView(runtime, id) + "' of " +
      valueStr);
}

/// Implement a comparison operator. First both operands a converted to
/// primitives. If they both end up being strings, a lexicographical comparison
/// is performed. Otherwise both operands are converted to numbers and the
/// values are compared.
/// \param oper is the comparison operator to use when comparing numbers.
#define IMPLEMENT_COMPARISON_OP(name, oper)                           \
  CallResult<bool> name(                                              \
      Runtime *runtime, Handle<> leftHandle, Handle<> rightHandle) {  \
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
    /* Convert both to a number and compare the numbers. */           \
    resLeft = toNumber_RJS(runtime, left);                            \
    if (resLeft == ExecutionStatus::EXCEPTION)                        \
      return ExecutionStatus::EXCEPTION;                              \
    left = resLeft.getValue();                                        \
    resRight = toNumber_RJS(runtime, right);                          \
    if (resRight == ExecutionStatus::EXCEPTION)                       \
      return ExecutionStatus::EXCEPTION;                              \
    right = resRight.getValue();                                      \
                                                                      \
    return left->getNumber() oper right->getNumber();                 \
  }

IMPLEMENT_COMPARISON_OP(lessOp_RJS, <);
IMPLEMENT_COMPARISON_OP(greaterOp_RJS, >);
IMPLEMENT_COMPARISON_OP(lessEqualOp_RJS, <=);
IMPLEMENT_COMPARISON_OP(greaterEqualOp_RJS, >=);
CallResult<HermesValue>
abstractEqualityTest_RJS(Runtime *runtime, Handle<> xHandle, Handle<> yHandle) {
  MutableHandle<> x{runtime, xHandle.get()};
  MutableHandle<> y{runtime, yHandle.get()};

abstractEqualityTailCall:
  // Same type comparison.
  if (x->getTag() == y->getTag() || (x->isNumber() && y->isNumber())) {
    bool result;
    switch (x->getTag()) {
      case EmptyTag:
        llvm_unreachable("can't compare empties");
      case NativeValueTag:
        llvm_unreachable("native value");
      case UndefinedTag:
      case NullTag:
        result = true;
        break;
      case StrTag:
        result = x->getString()->equals(y->getString());
        break;
      case ObjectTag:
        // Return true if x and y refer to the same object.
        result = x->getPointer() == y->getPointer();
        break;
      case BoolTag:
        result = x->getBool() == y->getBool();
        break;
      case SymbolTag:
        result = x->getSymbol() == y->getSymbol();
        break;
      default: {
        result = x->getNumber() == y->getNumber();
        break;
      }
    }
    return HermesValue::encodeBoolValue(result);
  }

  // If the types are different, combine tags for use in the switch statement.
  // Use NativeValueTag as a placeholder for numbers.
  assert(
      !x->isNativeValue() && !x->isEmpty() && "invalid value for comparison");
  assert(
      !y->isNativeValue() && !y->isEmpty() && "invalid value for comparison");

  constexpr TagKind NumberTag = NativeValueTag;

  // Tag numbers as numbers, and use default tag values for everything else.
  TagKind xType = x->isNumber() ? NumberTag : x->getTag();
  TagKind yType = y->isNumber() ? NumberTag : y->getTag();

  switch (HermesValue::combineTags(xType, yType)) {
    case HermesValue::combineTags(NullTag, UndefinedTag):
    case HermesValue::combineTags(UndefinedTag, NullTag):
      return HermesValue::encodeBoolValue(true);

    case HermesValue::combineTags(NumberTag, StrTag):
      return HermesValue::encodeBoolValue(
          x->getNumber() ==
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(y)));
    case HermesValue::combineTags(StrTag, NumberTag):
      return HermesValue::encodeBoolValue(
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(x)) ==
          y->getNumber());

    case HermesValue::combineTags(BoolTag, NumberTag):
      // Do both conversions and check numerical equality.
      return HermesValue::encodeBoolValue(x->getBool() == y->getNumber());
    case HermesValue::combineTags(BoolTag, StrTag):
      // Do string parsing and check double equality.
      return HermesValue::encodeBoolValue(
          x->getBool() ==
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(y)));
    case HermesValue::combineTags(BoolTag, ObjectTag):
      x = HermesValue::encodeDoubleValue(x->getBool());
      goto abstractEqualityTailCall;

    case HermesValue::combineTags(NumberTag, BoolTag):
      return HermesValue::encodeBoolValue(x->getNumber() == y->getBool());
    case HermesValue::combineTags(StrTag, BoolTag):
      return HermesValue::encodeBoolValue(
          stringToNumber(runtime, Handle<StringPrimitive>::vmcast(x)) ==
          y->getBool());
    case HermesValue::combineTags(ObjectTag, BoolTag):
      y = HermesValue::encodeDoubleValue(y->getBool());
      goto abstractEqualityTailCall;

    case HermesValue::combineTags(StrTag, ObjectTag):
    case HermesValue::combineTags(SymbolTag, ObjectTag):
    case HermesValue::combineTags(NumberTag, ObjectTag): {
      auto status = toPrimitive_RJS(runtime, y, PreferredType::NONE);
      if (status == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      y = status.getValue();
      goto abstractEqualityTailCall;
    }
    case HermesValue::combineTags(ObjectTag, StrTag):
    case HermesValue::combineTags(ObjectTag, SymbolTag):
    case HermesValue::combineTags(ObjectTag, NumberTag): {
      auto status = toPrimitive_RJS(runtime, x, PreferredType::NONE);
      if (status == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      x = status.getValue();
      goto abstractEqualityTailCall;
    }

    default:
      // Final case, return false.
      return HermesValue::encodeBoolValue(false);
  } // namespace vm
} // namespace hermes

bool strictEqualityTest(HermesValue x, HermesValue y) {
  if ((x.getTag() != y.getTag()) && !(x.isNumber() && y.isNumber())) {
    // x and y can be equal with different tags only if they're both numbers.
    // If tags are different and both x and y aren't numbers, we're done.
    return false;
  }
  switch (x.getTag()) {
    case UndefinedTag:
    case NullTag:
      return true;
    case StrTag:
      return x.getString()->equals(y.getString());
    case ObjectTag:
      // Return true if x and y refer to the same object.
      return x.getPointer() == y.getPointer();
    case BoolTag:
      return x.getBool() == y.getBool();
    case SymbolTag:
      return x.getSymbol() == y.getSymbol();
    case EmptyTag:
      llvm_unreachable("can't compare empties");
    case NativeValueTag:
      llvm_unreachable("can't compare native values");
    default:
      assert(x.isNumber() && y.isNumber());
      // Know they're both doubles here, so do standard comparison.
      return x.getNumber() == y.getNumber();
  }
  return false;
}

CallResult<HermesValue>
addOp_RJS(Runtime *runtime, Handle<> xHandle, Handle<> yHandle) {
  auto resX = toPrimitive_RJS(runtime, xHandle, PreferredType::NONE);
  if (resX == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto x = runtime->makeHandle(resX.getValue());

  auto resY = toPrimitive_RJS(runtime, yHandle, PreferredType::NONE);
  if (resY == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto y = runtime->makeHandle(resY.getValue());

  // If one of the values is a string, concatenate as strings.
  if (x->isString() || y->isString()) {
    auto resX = toString_RJS(runtime, x);
    if (resX == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto xStr = toHandle(runtime, std::move(*resX));

    auto resY = toString_RJS(runtime, y);
    if (resY == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto yStr = toHandle(runtime, std::move(*resY));

    return StringPrimitive::concat(runtime, xStr, yStr);
  }

  // Add the numbers since neither are strings.
  resX = toNumber_RJS(runtime, x);
  if (LLVM_UNLIKELY(resX == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto xNum = resX.getValue().getNumber();

  resY = toNumber_RJS(runtime, y);
  if (LLVM_UNLIKELY(resY == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto yNum = resY.getValue().getNumber();

  return HermesValue::encodeDoubleValue(xNum + yNum);
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
numberToStringWithRadix(Runtime *runtime, double number, unsigned radix) {
  (void)MIN_RADIX;
  (void)MAX_RADIX;
  assert(MIN_RADIX <= radix && radix <= MAX_RADIX && "Invalid radix");
  // Two parts of the final result: integer part and fractional part.
  llvm::SmallString<64> result{};

  // Used to store just the fractional part of the string (not including '.').
  llvm::SmallString<32> fStr{};

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
        oscompat::nextafter(number, std::numeric_limits<double>::infinity());
    double minDenorm =
        oscompat::nextafter(0.0, std::numeric_limits<double>::infinity());

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

  return runtime->makeHandle<StringPrimitive>(runtime->ignoreAllocationFailure(
      StringPrimitive::create(runtime, result)));
}

CallResult<PseudoHandle<>>
getMethod(Runtime *runtime, Handle<> O, Handle<> key) {
  GCScopeMarkerRAII gcScope{runtime};
  auto objRes = toObject(runtime, O);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto obj = runtime->makeHandle<JSObject>(*objRes);
  auto funcRes = JSObject::getComputed_RJS(obj, runtime, key);
  if (LLVM_UNLIKELY(funcRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (funcRes->isUndefined() || funcRes->isNull()) {
    return PseudoHandle<>::create(HermesValue::encodeUndefinedValue());
  }
  if (!vmisa<Callable>(*funcRes)) {
    return runtime->raiseTypeError("Could not get callable method from object");
  }
  return PseudoHandle<>::create(*funcRes);
}

CallResult<IteratorRecord> getIterator(
    Runtime *runtime,
    Handle<> obj,
    llvm::Optional<Handle<Callable>> methodOpt) {
  MutableHandle<Callable> method{runtime};
  if (LLVM_LIKELY(!methodOpt.hasValue())) {
    auto methodRes = getMethod(
        runtime,
        obj,
        runtime->makeHandle(
            Predefined::getSymbolID(Predefined::SymbolIterator)));
    if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!vmisa<Callable>(methodRes->getHermesValue())) {
      return runtime->raiseTypeError("Iterator method must be callable");
    }
    method = vmcast<Callable>(methodRes->getHermesValue());
  } else {
    method = **methodOpt;
  }
  auto iteratorRes = Callable::executeCall0(method, runtime, obj);
  if (LLVM_UNLIKELY(iteratorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!iteratorRes->isObject())) {
    return runtime->raiseTypeError("Iterators must be objects");
  }
  auto iterator = runtime->makeHandle<JSObject>(*iteratorRes);

  auto nextMethodRes = JSObject::getNamed_RJS(
      iterator, runtime, Predefined::getSymbolID(Predefined::next));
  if (LLVM_UNLIKELY(nextMethodRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // We perform this check prior to returning, because every function in the JS
  // library which gets an iterator immediately calls the 'next' function.
  if (!vmisa<Callable>(*nextMethodRes)) {
    return runtime->raiseTypeError(
        "'next' method on iterator must be callable");
  }

  auto nextMethod = runtime->makeHandle<Callable>(*nextMethodRes);

  return IteratorRecord{iterator, nextMethod};
}

CallResult<PseudoHandle<JSObject>> iteratorNext(
    Runtime *runtime,
    const IteratorRecord &iteratorRecord,
    llvm::Optional<Handle<>> value) {
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
  if (LLVM_UNLIKELY(!resultRes->isObject())) {
    return runtime->raiseTypeError(
        "Iterator .next() method must return an object");
  }
  return PseudoHandle<JSObject>::create(vmcast<JSObject>(*resultRes));
}

CallResult<Handle<JSObject>> iteratorStep(
    Runtime *runtime,
    const IteratorRecord &iteratorRecord) {
  auto resultRes = iteratorNext(runtime, iteratorRecord);
  if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> result = toHandle(runtime, std::move(*resultRes));
  auto completeRes = JSObject::getNamed_RJS(
      result, runtime, Predefined::getSymbolID(Predefined::done));
  if (LLVM_UNLIKELY(completeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (toBoolean(*completeRes)) {
    return Runtime::makeNullHandle<JSObject>();
  }
  return result;
}

ExecutionStatus iteratorClose(
    Runtime *runtime,
    Handle<JSObject> iterator,
    Handle<> completion) {
  ExecutionStatus completionStatus = completion->isEmpty()
      ? ExecutionStatus::RETURNED
      : ExecutionStatus::EXCEPTION;
  auto returnRes = getMethod(
      runtime,
      iterator,
      runtime->makeHandle(Predefined::getSymbolID(Predefined::returnStr)));
  if (LLVM_UNLIKELY(returnRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!vmisa<Callable>(returnRes->getHermesValue())) {
    runtime->setThrownValue(*completion);
    return completionStatus;
  }
  Handle<Callable> returnFn =
      runtime->makeHandle(vmcast<Callable>(returnRes->getHermesValue()));
  auto innerResultRes = Callable::executeCall0(returnFn, runtime, iterator);
  if (innerResultRes == ExecutionStatus::EXCEPTION &&
      isUncatchableError(runtime->getThrownValue())) {
    // If the call to return threw an uncatchable exception, that overrides
    // the completion, since the point of an uncatchable exception is to prevent
    // more JS from executing.
    return ExecutionStatus::EXCEPTION;
  }
  if (completionStatus == ExecutionStatus::EXCEPTION) {
    // Rethrow the error in the completion.
    runtime->setThrownValue(*completion);
    return ExecutionStatus::EXCEPTION;
  }
  if (innerResultRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!innerResultRes->isObject()) {
    return runtime->raiseTypeError("Iterator result must be an object");
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
createIterResultObject(Runtime *runtime, Handle<> value, bool done) {
  auto objHandle = toHandle(runtime, JSObject::create(runtime));
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
    Runtime *runtime,
    Handle<Callable> defaultConstructor) {
  // construct from the "constructor" property in self if that is defined, else
  // use the default one.
  auto res = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::constructor));
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto cons = *res;
  if (cons.isUndefined()) {
    return defaultConstructor;
  }
  if (!cons.isObject()) {
    return runtime->raiseTypeError(
        "Constructor must be an object if it is not undefined");
  }
  // There is no @@species (no Symbols yet), so we'll assume that there was
  // no other constructor specified.
  return defaultConstructor;
}

bool isConstructor(Runtime *runtime, HermesValue x) {
  // This is not a complete definition, since ES6 and later define member
  // functions of objects to not be constructors; however, Hermes does not have
  // ES6 classes implemented yet, so we cannot check for that case.
  Callable *c = dyn_vmcast<Callable>(x);
  if (!c) {
    return false;
  }

  // We traverse the BoundFunction target chain to find the eventual target.
  while (BoundFunction *b = dyn_vmcast<BoundFunction>(c)) {
    c = b->getTarget(runtime);
  }

  // If it is a bytecode function, check the flags.
  if (auto *func = dyn_cast<JSFunction>(c)) {
    auto *cb = func->getCodeBlock();
    // Even though it doesn't make sense logically, we need to compile the
    // function in order to access it flags.
    cb->lazyCompile(runtime);
    return !func->getCodeBlock()->getHeaderFlags().isCallProhibited(true);
  }

  // We check for NativeFunction since those are defined to not be
  // constructible, with the exception of NativeConstructor.
  return !vmisa<NativeFunction>(c) || vmisa<NativeConstructor>(c);
}

CallResult<bool>
ordinaryHasInstance(Runtime *runtime, Handle<> constructor, Handle<> object) {
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
      runtime->makeHandle(ctor),
      runtime,
      Predefined::getSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 6. If Type(P) is not Object, throw a TypeError exception.
  auto ctorPrototype = dyn_vmcast<JSObject>(*propRes);
  if (LLVM_UNLIKELY(!ctorPrototype)) {
    return runtime->raiseTypeError(
        "function's '.prototype' is not an object in 'instanceof'");
  }

  auto *obj = vmcast<JSObject>(object.get());

  // 7. Repeat
  // 7a. Let O be O.[[GetPrototypeOf]]().
  while ((obj = obj->getParent(runtime)) != nullptr) {
    if (obj == ctorPrototype) {
      // 7c. If SameValue(P, O) is true, return true.
      return true;
    }
  }

  // 7b. If O is null, return false.
  return false;
}

CallResult<bool> instanceOfOperator_RJS(
    Runtime *runtime,
    Handle<> object,
    Handle<> constructor) {
  // 1. If Type(C) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!constructor->isObject())) {
    return runtime->raiseTypeError(
        "right operand of 'instanceof' is not an object");
  }

  // Fast path: Function.prototype[Symbol.hasInstance] is non-configurable
  // and non-writable (ES6.0 19.2.3.6), so we directly run its behavior here.
  // Simply call through to ordinaryHasInstance.
  if (vmisa<JSFunction>(*constructor)) {
    return ordinaryHasInstance(runtime, constructor, object);
  }

  // 2. Let instOfHandler be GetMethod(C,@@hasInstance).
  CallResult<HermesValue> instOfHandlerRes = JSObject::getNamed_RJS(
      Handle<JSObject>::vmcast(constructor),
      runtime,
      Predefined::getSymbolID(Predefined::SymbolHasInstance));
  if (LLVM_UNLIKELY(instOfHandlerRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto instOfHandler = runtime->makeHandle(*instOfHandlerRes);

  // 4. If instOfHandler is not undefined, then
  if (!instOfHandler->isUndefined()) {
    // 5. Return ToBoolean(Call(instOfHandler, C, «O»)).
    if (!vmisa<Callable>(*instOfHandler)) {
      return runtime->raiseTypeError("instanceof handler must be callable");
    }
    auto callRes = Callable::executeCall1(
        Handle<Callable>::vmcast(instOfHandler), runtime, constructor, *object);
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return toBoolean(*callRes);
  }

  // 6. If IsCallable(C) is false, throw a TypeError exception.
  if (!vmisa<Callable>(*constructor)) {
    return runtime->raiseTypeError(
        "right operand of 'instanceof' is not callable");
  }

  // 7. Return OrdinaryHasInstance(C, O).
  return ordinaryHasInstance(runtime, constructor, object);
}

/// ES6.0 7.2.8
/// Returns true if the object is a JSRegExp or has a Symbol.match property that
/// evaluates to true.
CallResult<bool> isRegExp(Runtime *runtime, Handle<> arg) {
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
  if (!propRes->isUndefined()) {
    return toBoolean(propRes.getValue());
  }
  // 5. If argument has a [[RegExpMatcher]] internal slot, return true.
  // 6. Return false.
  return vmisa<JSRegExp>(arg.get());
}

CallResult<Handle<StringPrimitive>> symbolDescriptiveString(
    Runtime *runtime,
    Handle<SymbolID> sym) {
  auto desc = runtime->makeHandle<StringPrimitive>(
      runtime->getStringPrimFromSymbolID(*sym));
  SafeUInt32 descLen(desc->getStringLength());
  descLen.add(8);

  auto builder = StringBuilder::createStringBuilder(runtime, descLen);
  if (LLVM_UNLIKELY(builder == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  builder->appendASCIIRef({"Symbol(", 7});
  builder->appendStringPrim(desc);
  builder->appendCharacter(')');

  return builder->getStringPrimitive();
}

CallResult<bool> isConcatSpreadable(Runtime *runtime, Handle<> value) {
  auto O = Handle<JSObject>::dyn_vmcast(value);
  if (!O) {
    return false;
  }

  CallResult<HermesValue> spreadable = JSObject::getNamed_RJS(
      O,
      runtime,
      Predefined::getSymbolID(Predefined::SymbolIsConcatSpreadable));
  if (LLVM_UNLIKELY(spreadable == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (!spreadable->isUndefined()) {
    return toBoolean(*spreadable);
  }

  return vmisa<JSArray>(*O);
}

} // namespace vm
} // namespace hermes
