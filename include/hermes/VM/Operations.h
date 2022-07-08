/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_OPERATIONS_H
#define HERMES_VM_OPERATIONS_H

#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/InternalProperty.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace vm {

union DefinePropertyFlags;
class Runtime;

/// ES6.0 7.1.1
/// Type hint passed to toPrimitive_RJS() instead of string values.
enum class PreferredType {
  NONE,
  STRING,
  NUMBER,
};

/// ES6.0 7.1.1
CallResult<HermesValue>
toPrimitive_RJS(Runtime &runtime, Handle<> valueHandle, PreferredType hint);

/// ES6.0 7.1.1
/// The OrdinaryToPrimitive operation does not attempt to use the exotic
/// @@toPrimitive property on \p selfHandle.
CallResult<HermesValue> ordinaryToPrimitive(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    PreferredType preferredType);

/// ES5.1 9.2
bool toBoolean(HermesValue value);

/// ES5.1 9.3
CallResult<HermesValue> toNumber_RJS(Runtime &runtime, Handle<> valueHandle);

/// ES 2020 7.1.3 ToNumeric(value)
CallResult<HermesValue> toNumeric_RJS(Runtime &runtime, Handle<> valueHandle);

/// ES6 7.1.15
CallResult<HermesValue> toLength(Runtime &runtime, Handle<> valueHandle);

// a variant of toLength which returns a uint64_t
CallResult<uint64_t> toLengthU64(Runtime &runtime, Handle<> valueHandle);

/// ES 2018 7.1.17
CallResult<HermesValue> toIndex(Runtime &runtime, Handle<> valueHandle);

/// ES 2022 7.1.5
CallResult<HermesValue> toIntegerOrInfinity(
    Runtime &runtime,
    Handle<> valueHandle);

/// ES6 7.1.9
CallResult<HermesValue> toInt8(Runtime &runtime, Handle<> valueHandle);

/// ES6 7.1.7
CallResult<HermesValue> toInt16(Runtime &runtime, Handle<> valueHandle);

/// ES5.1 9.5
CallResult<HermesValue> toInt32_RJS(Runtime &runtime, Handle<> valueHandle);

/// ES6 7.1.10
CallResult<HermesValue> toUInt8(Runtime &runtime, Handle<> valueHandle);

/// ES6 7.1.11
uint8_t toUInt8Clamp(double number);
CallResult<HermesValue> toUInt8Clamp(Runtime &runtime, Handle<> valueHandle);

/// ES5.1 9.7
CallResult<HermesValue> toUInt16(Runtime &runtime, Handle<> valueHandle);

/// ES5.1 9.6
CallResult<HermesValue> toUInt32_RJS(Runtime &runtime, Handle<> valueHandle);

/// ES5.1 9.8
CallResult<PseudoHandle<StringPrimitive>> toString_RJS(
    Runtime &runtime,
    Handle<> valueHandle);

/// ES9 7.2.7
inline bool isPropertyKey(Handle<> valueHandle) {
  return valueHandle->isString() || valueHandle->isSymbol();
}

/// ES9 7.1.14
inline CallResult<Handle<>> toPropertyKey(
    Runtime &runtime,
    Handle<> valueHandle) {
  CallResult<HermesValue> primRes =
      toPrimitive_RJS(runtime, valueHandle, PreferredType::STRING);
  if (LLVM_UNLIKELY(primRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> prim = runtime.makeHandle(*primRes);
  if (prim->isSymbol()) {
    return prim;
  }
  CallResult<PseudoHandle<StringPrimitive>> strRes =
      toString_RJS(runtime, prim);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return Handle<>::vmcast(runtime.makeHandle(std::move(*strRes)));
}

/// This function is used to convert a property to property key if it's an
/// object. Check if \p nameValHandle is an object, if so, convert it to a
/// key which may have side-effects. Otherwise just return the original
/// handle.
/// ES6.0 7.1.14, but only for objects. Primitives are left unaltered.
/// Note: this can return a Symbol or a String if \p valueHandle is an object.
inline CallResult<Handle<>> toPropertyKeyIfObject(
    Runtime &runtime,
    Handle<> valueHandle) {
  if (LLVM_UNLIKELY(valueHandle->isObject())) {
    return toPropertyKey(runtime, valueHandle);
  }
  return valueHandle;
}

/// \return prototype for primitive \p base.
CallResult<Handle<JSObject>> getPrimitivePrototype(
    Runtime &runtime,
    Handle<> base);

/// ES5.1 9.9
CallResult<HermesValue> toObject(Runtime &runtime, Handle<> valueHandle);

/// Helper function. When accessing a property on an undefined/null object,
/// we can re-throw the exception with an improved error message that
/// contains the property name \id. \operationStr denotes the type of
/// operation (i.e. read, set and delete) on the object.
/// Example old error message: Cannot convert undefined value to object.
/// New error message would be:
/// Cannot \operationStr property [propName] of undefined.
ExecutionStatus amendPropAccessErrorMsgWithPropName(
    Runtime &runtime,
    Handle<> valueHandle,
    llvh::StringRef operationStr,
    SymbolID id);

/// ES5 9.10 CheckObjectCoercible
/// ES6+ 7.2.1 RequireObjectCoercible ( argument )
inline ExecutionStatus checkObjectCoercible(
    Runtime &runtime,
    Handle<> valueHandle) {
  if (LLVM_UNLIKELY(valueHandle->isUndefined() || valueHandle->isNull())) {
    return runtime.raiseTypeError("Value not coercible to object");
  }
  return ExecutionStatus::RETURNED;
}

/// ES6.0 7.2.9.
/// The SameValue(x, y) operation.
bool isSameValue(HermesValue x, HermesValue y);

/// ES6 7.2.10. The only difference from isSameValue is: +0 == -0.
bool isSameValueZero(HermesValue x, HermesValue y);

/// ES5.1 11.8.1.
CallResult<bool>
lessOp_RJS(Runtime &runtime, Handle<> leftHandle, Handle<> rightHandle);
/// ES5.1 11.8.2.
CallResult<bool>
greaterOp_RJS(Runtime &runtime, Handle<> leftHandle, Handle<> rightHandle);
/// ES5.1 11.8.3.
CallResult<bool>
lessEqualOp_RJS(Runtime &runtime, Handle<> leftHandle, Handle<> rightHandle);
/// ES5.1 11.8.4.
CallResult<bool>
greaterEqualOp_RJS(Runtime &runtime, Handle<> leftHandle, Handle<> rightHandle);

/// ES11 7.2.15 Abstract Equality Comparison
CallResult<bool>
abstractEqualityTest_RJS(Runtime &runtime, Handle<> xHandle, Handle<> yHandle);

/// ES5.1 11.9.6
bool strictEqualityTest(HermesValue x, HermesValue y);

/// Convert a string to a uniqued property name.
CallResult<Handle<SymbolID>> stringToSymbolID(
    Runtime &runtime,
    PseudoHandle<StringPrimitive> strPrim);

/// Convert a value to a uniqued property name.
CallResult<Handle<SymbolID>> valueToSymbolID(
    Runtime &runtime,
    Handle<> nameValHnd);

/// \return a string indicating the type of the operand corresponding to the
/// `typeof` operator.
HermesValue typeOf(Runtime &runtime, Handle<> valueHandle);

/// Convert a string to an array index following ES5.1 15.4.
/// A property name P (in the form of a String value) is an array index if and
/// only if ToString(ToUint32(P)) is equal to P and ToUint32(P) is not equal to
/// 2**32−1.
OptValue<uint32_t> toArrayIndex(
    Runtime &runtime,
    Handle<StringPrimitive> strPrim);

/// Fast path for toArrayIndex where we already have the view of the string.
OptValue<uint32_t> toArrayIndex(StringView str);

/// If it is possible to cheaply verify that \p value is an array index
/// according to the rules in ES5.1 15.4, do so and return the index. Note that
/// it this fails, the value may still be a valid index.
OptValue<uint32_t> toArrayIndexFastPath(HermesValue value)
    LLVM_NO_SANITIZE("float-cast-overflow");
inline OptValue<uint32_t> toArrayIndexFastPath(HermesValue value) {
  if (value.isNumber()) {
    return hermes::doubleToArrayIndex(value.getNumber());
  }
  return llvh::None;
}

/// \return true if the ToPrimitive function (ES5.1 9.1) performs no conversion.
/// Primitive types: Undefined, Null, Boolean, Number, and String.
bool isPrimitive(HermesValue val);

/// ES5.1 11.6.1
CallResult<HermesValue>
addOp_RJS(Runtime &runtime, Handle<> xHandle, Handle<> yHandle);

/// ES9.0 12.6.4
inline double expOp(double x, double y) {
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();

  // Handle special cases that std::pow handles differently.
  if (std::isnan(y)) {
    return nan;
  } else if (y == 0) {
    return 1;
  } else if (std::abs(x) == 1 && std::isinf(y)) {
    return nan;
  }

  // std::pow handles the other edge cases as the ES9.0 spec requires.
  return std::pow(x, y);
}

/// ES5.1 7.2
inline bool isWhiteSpaceChar(char16_t c) {
  return c == u'\u0009' || c == u'\u000B' || c == u'\u000C' || c == u'\u0020' ||
      c == u'\u00A0' || c == u'\uFEFF' || c == u'\u1680' ||
      (c >= u'\u2000' && c <= u'\u200A') || c == u'\u202F' || c == u'\u205F' ||
      c == u'\u3000';
}

/// ES5.1 7.3
inline bool isLineTerminatorChar(char16_t c) {
  return c == u'\u000A' || c == u'\u000D' || c == u'\u2028' || c == u'\u2029';
}

/// Takes a letter (a-z or A-Z) and makes it lowercase.
inline char16_t letterToLower(char16_t c) {
  return c | 32;
}

/// Takes a non-empty string (without the leading "0x" if hex) and parses it
/// as radix \p radix.
/// \returns the double that results, and NaN on failure.
double parseIntWithRadix(const StringView str, int radix);

/// Takes a finite double \p number and a base \p radix (between 2 and 36
/// inclusive), and returns the string that results from converting \p number
/// into a string in base \p radix.
/// Note that this is implementation defined behavior: we output a rounded
/// string such that we're as close as possible to the actual precision encoded
/// in the double value given by number.
/// \returns the string that results.
Handle<StringPrimitive>
numberToStringWithRadix(Runtime &runtime, double number, unsigned radix);

/// ES6.0 7.3.9
CallResult<PseudoHandle<>>
getMethod(Runtime &runtime, Handle<> O, Handle<> key);

/// ES9.0 Record type for iterator records.
/// Used for caching the "next" method to avoid repeated property lookups.
struct IteratorRecord {
  /// Actual iterator object.
  const Handle<JSObject> iterator;

  /// Cache for the "next" method to call to step the iterator.
  const Handle<Callable> nextMethod;

  IteratorRecord(Handle<JSObject> iterator, Handle<Callable> nextMethod)
      : iterator(iterator), nextMethod(nextMethod) {}
};

/// ES6.0 7.4.1
/// \param obj object to iterate over.
/// \param method an optional method to call instead of retrieving @@iterator.
/// \return the iterator object
CallResult<IteratorRecord> getIterator(
    Runtime &runtime,
    Handle<> obj,
    llvh::Optional<Handle<Callable>> method = llvh::None);

/// ES6.0 7.4.2
CallResult<PseudoHandle<JSObject>> iteratorNext(
    Runtime &runtime,
    const IteratorRecord &iteratorRecord,
    llvh::Optional<Handle<>> value = llvh::None);

/// ES6.0 7.4.5
/// \return a null pointer instead of the boolean false.
CallResult<Handle<JSObject>> iteratorStep(
    Runtime &runtime,
    const IteratorRecord &iteratorRecord);

/// ES sec-iteratorclose
/// \param completion the thrown value to complete this operation with, empty if
/// not thrown.
ExecutionStatus
iteratorClose(Runtime &runtime, Handle<JSObject> iterator, Handle<> completion);

/// Some types of errors are considered "uncatchable" by the VM.
/// If any native code wants to catch an error, it needs to check that the value
/// is catchable first.
/// \return true if the HermesValue is something that can be caught in a catch
/// block.
bool isUncatchableError(HermesValue value);

/// ES6.0 7.4.7
Handle<JSObject>
createIterResultObject(Runtime &runtime, Handle<> value, bool done);

/// ES7 7.3.20
CallResult<Handle<Callable>> speciesConstructor(
    Handle<JSObject> O,
    Runtime &runtime,
    Handle<Callable> defaultConstructor);

/// ES7 7.2.4
/// Returns true if the \c value is a constructor.  The value can be
/// Anything.
CallResult<bool> isConstructor(Runtime &runtime, HermesValue value);

/// ES7 7.2.4
/// Returns true if \c callable is a constructor.  Passing \c nullptr
/// is allowed, and returns false.
CallResult<bool> isConstructor(Runtime &runtime, Callable *callable);

/// ES6.0 7.2.8
/// Returns true if the object is a JSRegExp or has a Symbol.match property that
/// evaluates to true.
CallResult<bool> isRegExp(Runtime &runtime, Handle<> arg);

/// ES6.0 7.3.19
CallResult<bool>
ordinaryHasInstance(Runtime &runtime, Handle<> constructor, Handle<> object);

/// ES6.0 10.1.1
/// \param cp the codepoint to convert to UTF-16.
/// \param[out] output the vector into which to place the results.
inline void utf16Encoding(
    uint32_t cp,
    llvh::SmallVectorImpl<char16_t> &output) {
  // Assert: 0 ≤ cp ≤ 0x10FFFF.
  assert(cp <= 0x10FFFF && "Invalid input to UTF16Encoding");
  // If cp ≤ 65535, return cp.
  if (cp <= 65535) {
    output.push_back((char16_t)cp);
    return;
  }
  // Let cu1 be floor((cp – 65536) / 1024) + 0xD800.
  // The floor is handled by using integer arithmetic.
  char16_t cu1 = (char16_t)(((cp - 65536) / 1024) + 0xD800);
  // Let cu2 be ((cp – 65536) modulo 1024) + 0xDC00.
  char16_t cu2 = (char16_t)(((cp - 65536) % 1024) + 0xDC00);
  // Return the code unit sequence consisting of cu1 followed by cu2.
  output.push_back(cu1);
  output.push_back(cu2);
}

/// ES6.0 10.1.2
/// Two code units, lead and trail, that form a UTF-16 surrogate pair are
/// converted to a code point.
inline uint32_t utf16Decode(char16_t lead, char16_t trail) {
  // Assert: 0xD800 ≤ lead ≤ 0xDBFF and 0xDC00 ≤ trail ≤ 0xDFFF.
  assert(
      0xD800 <= lead && lead <= 0xDBFF && 0xDC00 <= trail && trail <= 0xDFFF &&
      "Invalid input to UTF16Decode");
  // Let cp be (lead – 0xD800) × 1024 + (trail – 0xDC00) + 0x10000.
  // Return the code point cp.
  uint32_t c1 = lead - 0xD800;
  uint32_t c2 = trail - 0xDC00;
  return c1 * 1024 + c2 + 0x10000;
}

/// ES6.0 12.9.4
CallResult<bool>
instanceOfOperator_RJS(Runtime &runtime, Handle<> object, Handle<> constructor);

/// ES6.0 19.4.3.2.1 Runtime Semantics: SymbolDescriptiveString ( sym )
/// Returns "Symbol([description])" given a symbol.
CallResult<Handle<StringPrimitive>> symbolDescriptiveString(
    Runtime &runtime,
    Handle<SymbolID> sym);

/// ES9 7.2.2
CallResult<bool> isArray(Runtime &runtime, JSObject *obj);

/// ES6.0 22.1.3.1.1
CallResult<bool> isConcatSpreadable(Runtime &runtime, Handle<> value);

/// \return true if and only if \p id is a primitive SymbolID backing a JS
/// Symbol instance.
constexpr bool isSymbolPrimitive(SymbolID id) {
  return id.isNotUniqued() && !InternalProperty::isInternal(id);
}

/// \return true if and only if \p id is a primitive SymbolID backing a JS
/// Property Name.
constexpr bool isPropertyNamePrimitive(SymbolID id) {
  return id.isUniqued();
}

/// ES5.1 8.10.5. toPropertyDescriptor(O). The result is written into
/// \p flags and \p valueOrAccessor together to represent a descriptor.
ExecutionStatus toPropertyDescriptor(
    Handle<> obj,
    Runtime &runtime,
    DefinePropertyFlags &flags,
    MutableHandle<> &valueOrAccessor);

/// ES9 6.2.5.4 FromPropertyDescriptor
CallResult<HermesValue> objectFromPropertyDescriptor(
    Runtime &runtime,
    ComputedPropertyDescriptor desc,
    Handle<> valueOrAccessor);

/// ES2022 21.2.1.1.1 NumberToBigInt(number)
CallResult<HermesValue> numberToBigInt(Runtime &runtime, double number);

// ES2022 7.2.6 IsIntegralNumber(argument)
bool isIntegralNumber(double number);

// ES2022 7.1.13 ToBigInt(argument)
CallResult<HermesValue> toBigInt_RJS(Runtime &runtime, Handle<> value);

// ES2022 7.1.14 StringToBigInt
CallResult<HermesValue> stringToBigInt_RJS(Runtime &runtime, Handle<> value);

// ES2022 21.2.3 Properties of the BigInt Prototype Object - thisBigIntValue
CallResult<HermesValue> thisBigIntValue(Runtime &runtime, Handle<> value);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OPERATIONS_H
