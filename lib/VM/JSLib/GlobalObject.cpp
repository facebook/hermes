/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Initialize the global object ES5.1 15.1
//===----------------------------------------------------------------------===//
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/VM/FastArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/StringView.h"

#include "dtoa/dtoa.h"

#include "JSLibInternal.h"

namespace hermes {
namespace vm {

/// ES5.1 15.1.2.4
CallResult<HermesValue> isNaN(void *, Runtime &runtime, NativeArgs args) {
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(std::isnan(res->getNumber()));
}

/// ES5.1 15.1.2.5
CallResult<HermesValue> isFinite(void *, Runtime &runtime, NativeArgs args) {
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto value = res->getDouble();
  return HermesValue::encodeBoolValue(std::isfinite(value));
}

/// Needed to construct Function.prototype.
CallResult<HermesValue> emptyFunction(void *, Runtime &, NativeArgs) {
  return HermesValue::encodeUndefinedValue();
}

/// Given a character \p c in radix \p radix, checks if it's valid.
static bool isValidRadixChar(char16_t c, int radix) {
  // c is 0..9.
  if (c >= '0' && c <= '9') {
    return (radix >= 10 || c < '0' + radix);
  }
  c = letterToLower(c);
  return (radix > 10 && c >= 'a' && c < 'a' + radix - 10);
}

/// ES5.1 15.1.2.2 parseInt(string, radix)
CallResult<HermesValue> parseInt(void *, Runtime &runtime, NativeArgs args) {
  // toString(arg0).
  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto str = runtime.makeHandle(std::move(*strRes));

  int radix = 10;
  bool stripPrefix = true;
  // If radix (arg1) is present and not undefined, toInt32_RJS(arg1).
  if (args.getArgCount() > 1 && !args.getArg(1).isUndefined()) {
    auto intRes = toInt32_RJS(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    radix = static_cast<int>(intRes->getNumber());
    if (radix == 0) {
      radix = 10;
    } else if (radix < 2 || radix > 36) {
      return HermesValue::encodeNaNValue();
    } else if (radix != 16) {
      stripPrefix = false;
    }
  }

  auto strView = StringPrimitive::createStringView(runtime, str);
  auto begin = strView.begin();
  auto end = strView.end();

  // Remove leading whitespaces.
  while (begin != end &&
         (isWhiteSpaceChar(*begin) || isLineTerminatorChar(*begin))) {
    ++begin;
  }

  // Process sign.
  int sign = 1;
  if (begin != end && (*begin == u'+' || *begin == u'-')) {
    if (*begin == u'-') {
      sign = -1;
    }
    ++begin;
  }

  // Strip 0x or 0X for 16-radix number.
  if (stripPrefix && begin != end) {
    if (*begin == u'0') {
      ++begin;
      if (begin != end && letterToLower(*begin) == u'x') {
        ++begin;
        radix = 16;
      } else {
        --begin;
      }
    }
  }

  // Find the longest prefix that's still a valid int.
  auto realEnd = begin;
  for (; realEnd != end && isValidRadixChar(*realEnd, radix); ++realEnd) {
  }
  if (realEnd == begin) {
    // Return NaN if string has no digits.
    return HermesValue::encodeNaNValue();
  }

  return HermesValue::encodeTrustedNumberValue(
      sign * parseIntWithRadix(strView.slice(begin, realEnd), radix));
}

// Check if str1 is a prefix of str2.
static bool isPrefix(StringView str1, StringView str2) {
  if (str1.length() > str2.length()) {
    return false;
  }
  for (auto first1 = str1.begin(), last1 = str1.end(), first2 = str2.begin();
       first1 != last1;
       ++first1, ++first2) {
    if (*first1 != *first2) {
      return false;
    }
  }
  return true;
}

/// ES5.1 15.1.2.3 parseFloat(string)
CallResult<HermesValue> parseFloat(void *, Runtime &runtime, NativeArgs args) {
  // toString(arg0).
  auto res = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto strHandle = runtime.makeHandle(std::move(*res));
  auto origStr = StringPrimitive::createStringView(runtime, strHandle);

  auto &idTable = runtime.getIdentifierTable();

  // Trim leading whitespaces.
  auto begin = origStr.begin();
  auto end = origStr.end();

  while (begin != end &&
         (isWhiteSpaceChar(*begin) || isLineTerminatorChar(*begin))) {
    ++begin;
  }
  StringView str16 = origStr.slice(begin, end);

  // Check for special values.
  // parseFloat allows for partial match, hence we have to check for
  // substring.
  if (LLVM_UNLIKELY(isPrefix(
          idTable.getStringView(
              runtime, Predefined::getSymbolID(Predefined::Infinity)),
          str16))) {
    return HermesValue::encodeTrustedNumberValue(
        std::numeric_limits<double>::infinity());
  }
  if (LLVM_UNLIKELY(isPrefix(
          idTable.getStringView(
              runtime, Predefined::getSymbolID(Predefined::PositiveInfinity)),
          str16))) {
    return HermesValue::encodeTrustedNumberValue(
        std::numeric_limits<double>::infinity());
  }
  if (LLVM_UNLIKELY(isPrefix(
          idTable.getStringView(
              runtime, Predefined::getSymbolID(Predefined::NegativeInfinity)),
          str16))) {
    return HermesValue::encodeTrustedNumberValue(
        -std::numeric_limits<double>::infinity());
  }
  if (LLVM_UNLIKELY(isPrefix(
          idTable.getStringView(
              runtime, Predefined::getSymbolID(Predefined::NaN)),
          str16))) {
    return HermesValue::encodeNaNValue();
  }

  // Copy 16 bit chars into 8 bit chars as long as the character is
  // still a valid decimal number character.
  auto len = str16.length();
  llvh::SmallVector<char, 32> str8(len + 1);
  uint32_t i = 0;
  for (auto c : str16) {
    if ((c >= u'0' && c <= u'9') || c == '.' || letterToLower(c) == 'e' ||
        c == '+' || c == '-') {
      str8[i] = static_cast<char>(c);
    } else {
      break;
    }
    ++i;
  }
  if (i == 0) {
    // Empty string.
    return HermesValue::encodeNaNValue();
  }
  // Use hermes_g_strtod to figure out the longest prefix that's still valid.
  // hermes_g_strtod will try to convert the string to int for as long as it
  // can, and set endPtr to the last location where the prefix so far is still
  // a valid integer.
  len = i;
  str8[len] = '\0';
  char *endPtr;
  ::hermes_g_strtod(str8.data(), &endPtr);
  if (endPtr == str8.data()) {
    // Empty string.
    return HermesValue::encodeNaNValue();
  }
  // Now we know the prefix untill endPtr is a valid int.
  *endPtr = '\0';
  return HermesValue::encodeTrustedNumberValue(
      ::hermes_g_strtod(str8.data(), &endPtr));
}

/// Customized global function. gc() forces a GC collect.
CallResult<HermesValue> gc(void *, Runtime &runtime, NativeArgs) {
  runtime.collect("forced");
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
throwTypeError(void *ctx, Runtime &runtime, NativeArgs) {
  static const char *TypeErrorMessage[] = {
      "Restricted property cannot be accessed",
      "Dynamic requires are not allowed after static resolution",
  };

  uint64_t kind = (uint64_t)ctx;
  assert(
      kind < (uint64_t)TypeErrorKind::NumKinds &&
      "[[ThrowTypeError]] wrong error kind passed as context");
  return runtime.raiseTypeError(TypeErrorMessage[kind]);
}

// NOTE: when declaring more global symbols, don't forget to update
// "Libhermes.h".
void initGlobalObject(Runtime &runtime, const JSLibFlags &jsLibFlags) {
  GCScope gcScope{runtime, "initGlobalObject", 350};

  // Not enumerable, not writable, not configurable.
  DefinePropertyFlags constantDPF =
      DefinePropertyFlags::getDefaultNewPropertyFlags();
  constantDPF.enumerable = 0;
  constantDPF.writable = 0;
  constantDPF.configurable = 0;

  // Not enumerable, but writable and configurable.
  DefinePropertyFlags normalDPF =
      DefinePropertyFlags::getNewNonEnumerableFlags();

  // Not enumerable, not writable but configurable.
  DefinePropertyFlags configurableOnlyPDF =
      DefinePropertyFlags::getDefaultNewPropertyFlags();
  configurableOnlyPDF.enumerable = 0;
  configurableOnlyPDF.writable = 0;

  /// Clear the configurable flag.
  DefinePropertyFlags clearConfigurableDPF{};
  clearConfigurableDPF.setConfigurable = 1;
  clearConfigurableDPF.configurable = 0;

  // Define a function on the global object with name \p name.
  // Allocates a NativeObject and puts it in the global object.
  auto defineGlobalFunc =
      [&](SymbolID name, NativeFunctionPtr functionPtr, unsigned paramCount) {
        gcScope.clearAllHandles();

        auto func = NativeFunction::createWithoutPrototype(
            runtime, nullptr, functionPtr, name, paramCount);
        runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
            runtime.getGlobal(), runtime, name, normalDPF, func));
        return func;
      };

  // 15.1.1.1 NaN.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::NaN),
      constantDPF,
      runtime.makeHandle(HermesValue::encodeNaNValue())));

  // 15.1.1.2 Infinity.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::Infinity),
      constantDPF,
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(
          std::numeric_limits<double>::infinity()))));

  // 15.1.1.2 undefined.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::undefined),
      constantDPF,
      runtime.makeHandle(HermesValue::encodeUndefinedValue())));

  // "Forward declaration" of Object.prototype. Its properties will be populated
  // later.

  runtime.objectPrototype =
      JSObject::create(runtime, Runtime::makeNullHandle<JSObject>());
  runtime.objectPrototypeRawPtr = *runtime.objectPrototype;

  // "Forward declaration" of Error.prototype. Its properties will be populated
  // later.
  runtime.ErrorPrototype = JSObject::create(runtime);

// "Forward declaration" of the prototype for native error types. Their
// properties will be populated later.
#define NATIVE_ERROR_TYPE(name) \
  runtime.name##Prototype = JSObject::create(runtime, runtime.ErrorPrototype);
#define AGGREGATE_ERROR_TYPE(name) NATIVE_ERROR_TYPE(name)
#include "hermes/VM/NativeErrorTypes.def"

  // "Forward declaration" of the internal CallSite prototype. Its properties
  // will be populated later.
  runtime.callSitePrototype =
      JSObject::create(runtime, runtime.objectPrototype);

  // "Forward declaration" of Function.prototype. Its properties will be
  // populated later.
  runtime.functionPrototype = NativeFunction::create(
      runtime,
      runtime.objectPrototype,
      nullptr,
      emptyFunction,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      Runtime::makeNullHandle<JSObject>());
  runtime.functionPrototypeRawPtr = *runtime.functionPrototype;
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.functionPrototype,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      configurableOnlyPDF,
      Runtime::getZeroValue()));

  // [[ThrowTypeError]].
  auto throwTypeErrorFunction = NativeFunction::create(
      runtime,
      runtime.functionPrototype,
      (void *)TypeErrorKind::RestrictedProperty,
      throwTypeError,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      Runtime::makeNullHandle<JSObject>());
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      throwTypeErrorFunction,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      clearConfigurableDPF,
      Runtime::getUndefinedValue()));
  runtime.throwTypeErrorAccessor = PropertyAccessor::create(
      runtime, throwTypeErrorFunction, throwTypeErrorFunction);

  // Define the 'parseInt' function.
  runtime.parseIntFunction = defineGlobalFunc(
      Predefined::getSymbolID(Predefined::parseInt), parseInt, 2);

  // Define the 'parseFloat' function.
  runtime.parseFloatFunction = defineGlobalFunc(
      Predefined::getSymbolID(Predefined::parseFloat), parseFloat, 1);

  // "Forward declaration" of String.prototype. Its properties will be
  // populated later.
  runtime.stringPrototype = runtime.ignoreAllocationFailure(JSString::create(
      runtime,
      runtime.getPredefinedStringHandle(Predefined::emptyString),
      runtime.objectPrototype));

  // "Forward declaration" of BigInt.prototype. Its properties will be
  // populated later.
  runtime.bigintPrototype = JSObject::create(runtime);

  // "Forward declaration" of Number.prototype. Its properties will be
  // populated later.
  runtime.numberPrototype =
      JSNumber::create(runtime, +0.0, runtime.objectPrototype);

  // "Forward declaration" of Boolean.prototype. Its properties will be
  // populated later.
  runtime.booleanPrototype =
      JSBoolean::create(runtime, false, runtime.objectPrototype);

  // "Forward declaration" of Symbol.prototype. Its properties will be
  // populated later.
  runtime.symbolPrototype = JSObject::create(runtime);

  // "Forward declaration" of Date.prototype. Its properties will be
  // populated later.
  runtime.datePrototype = JSObject::create(runtime, runtime.objectPrototype);

  // "Forward declaration" of %IteratorPrototype%.
  runtime.iteratorPrototype = JSObject::create(runtime);

  // "Forward declaration" of Array.prototype. Its properties will be
  // populated later.
  runtime.arrayPrototype =
      runtime.ignoreAllocationFailure(JSArray::createNoAllocPropStorage(
          runtime,
          runtime.objectPrototype,
          JSArray::createClass(runtime, runtime.objectPrototype),
          0,
          0));

  // Declare the array class.
  runtime.arrayClass = JSArray::createClass(runtime, runtime.arrayPrototype);

  // TODO: Give FastArray its own prototype so methods like push will work.
  runtime.fastArrayPrototype = *runtime.objectPrototype;

  // Declare the fast array class.
  runtime.fastArrayClass =
      FastArray::createClass(runtime, runtime.fastArrayPrototype);

  // Declare the regexp match object class.
  runtime.regExpMatchClass =
      JSRegExp::createMatchClass(runtime, runtime.arrayClass);

  // "Forward declaration" of ArrayBuffer.prototype. Its properties will be
  // populated later.
  runtime.arrayBufferPrototype =
      JSObject::create(runtime, runtime.objectPrototype);

  // "Forward declaration" of DataView.prototype. Its properties will be
  // populated later.
  runtime.dataViewPrototype =
      JSObject::create(runtime, runtime.objectPrototype);

  // "Forward declaration" of TypedArrayBase.prototype. Its properties will be
  // populated later.
  runtime.typedArrayBasePrototype = JSObject::create(runtime);

// Typed arrays
// NOTE: a TypedArray's prototype is a normal object, not a TypedArray.
#define TYPED_ARRAY(name, type)  \
  runtime.name##ArrayPrototype = \
      JSObject::create(runtime, runtime.typedArrayBasePrototype);
#include "hermes/VM/TypedArrays.def"

  // "Forward declaration" of Set.prototype. Its properties will be
  // populated later.
  runtime.setPrototype = JSObject::create(runtime);

  runtime.setIteratorPrototype = createSetIteratorPrototype(runtime);

  // "Forward declaration" of Map.prototype. Its properties will be
  // populated later.
  runtime.mapPrototype = JSObject::create(runtime);

  runtime.mapIteratorPrototype = createMapIteratorPrototype(runtime);

  // "Forward declaration" of RegExp.prototype.
  // ES6: 21.2.5 "The RegExp prototype object is an ordinary object. It is not a
  // RegExp instance..."
  runtime.regExpPrototype = JSObject::create(runtime, runtime.objectPrototype);

  // "Forward declaration" of WeakMap.prototype.
  runtime.weakMapPrototype = JSObject::create(runtime);

  // "Forward declaration" of WeakSet.prototype.
  runtime.weakSetPrototype = JSObject::create(runtime);

  // Only define WeakRef if microtasks are being used.
  if (LLVM_UNLIKELY(runtime.hasMicrotaskQueue())) {
    // "Forward declaration" of WeakRef.prototype.
    runtime.weakRefPrototype = JSObject::create(runtime);
  }

  // "Forward declaration" of %ArrayIteratorPrototype%.
  runtime.arrayIteratorPrototype =
      JSObject::create(runtime, runtime.iteratorPrototype);

  // "Forward declaration" of %StringIteratorPrototype%.
  runtime.stringIteratorPrototype =
      JSObject::create(runtime, runtime.iteratorPrototype);

  // "Forward declaration" of %RegExpStringIteratorPrototype%.
  runtime.regExpStringIteratorPrototype =
      JSObject::create(runtime, runtime.iteratorPrototype);

  // "Forward declaration" of "Generator prototype object"
  runtime.generatorPrototype =
      JSObject::create(runtime, runtime.iteratorPrototype);

  // "Forward declaration" of %GeneratorFunction.prototype%
  runtime.generatorFunctionPrototype =
      JSObject::create(runtime, runtime.functionPrototype);

  // "Forward declaration" of %AsyncFunction.prototype%
  runtime.asyncFunctionPrototype =
      JSObject::create(runtime, runtime.functionPrototype);

  // "Forward declaration" of TextEncoder.prototype.
  runtime.textEncoderPrototype = JSObject::create(runtime);

  // Object constructor.
  runtime.objectConstructor = createObjectConstructor(runtime);

// All Error constructors.
#define ALL_ERROR_TYPE(name)                                      \
  runtime.name##Constructor = create##name##Constructor(runtime); \
  gcScope.clearAllHandles();
#include "hermes/VM/NativeErrorTypes.def"

  // Populate the internal CallSite prototype.
  populateCallSitePrototype(runtime);

  // String constructor.
  runtime.stringConstructor = createStringConstructor(runtime);

  // BigInt constructor.
  createBigIntConstructor(runtime);

  // Function constructor.
  runtime.functionConstructor = createFunctionConstructor(runtime);

  // Number constructor.
  runtime.numberConstructor = createNumberConstructor(runtime);

  // Boolean constructor.
  runtime.booleanConstructor = createBooleanConstructor(runtime);

  // Date constructor.
  runtime.dateConstructor = createDateConstructor(runtime);

  // RegExp constructor
  createRegExpConstructor(runtime);

  // Array constructor.
  runtime.arrayConstructor = createArrayConstructor(runtime);

  if (runtime.hasArrayBuffer()) {
    // ArrayBuffer constructor.
    runtime.arrayBufferConstructor = createArrayBufferConstructor(runtime);

    // DataView constructor.
    runtime.dataViewConstructor = createDataViewConstructor(runtime);

    // TypedArrayBase constructor.
    runtime.typedArrayBaseConstructor =
        createTypedArrayBaseConstructor(runtime);

#define TYPED_ARRAY(name, type)                                             \
  runtime.name##ArrayConstructor = create##name##ArrayConstructor(runtime); \
  gcScope.clearAllHandles();
#include "hermes/VM/TypedArrays.def"
  } else {
    gcScope.clearAllHandles();
  } // hasArrayBuffer

  // Set constructor.
  runtime.setConstructor = createSetConstructor(runtime);

  // Map constructor.
  runtime.mapConstructor = createMapConstructor(runtime);

  // WeakMap constructor.
  runtime.weakMapConstructor = createWeakMapConstructor(runtime);

  // WeakSet constructor.
  runtime.weakSetConstructor = createWeakSetConstructor(runtime);

  // Only define WeakRef constructor if microtasks are being used.
  if (LLVM_UNLIKELY(runtime.hasMicrotaskQueue())) {
    // WeakRef constructor.
    runtime.weakRefConstructor = createWeakRefConstructor(runtime);
  }

  // Symbol constructor.
  createSymbolConstructor(runtime);

  /// %IteratorPrototype%.
  populateIteratorPrototype(runtime);

  /// Array Iterator.
  populateArrayIteratorPrototype(runtime);

  /// String Iterator.
  populateStringIteratorPrototype(runtime);

  /// RegExp String Iterator.
  populateRegExpStringIteratorPrototype(runtime);

  // GeneratorFunction constructor (not directly exposed in the global object).
  runtime.generatorFunctionConstructor =
      createGeneratorFunctionConstructor(runtime);

  // AsyncFunction constructor (not directly exposed in the global object).
  runtime.asyncFunctionConstructor = createAsyncFunctionConstructor(runtime);

  // TextEncoder constructor.
  runtime.textEncoderConstructor = createTextEncoderConstructor(runtime);

  // %GeneratorPrototype%.
  populateGeneratorPrototype(runtime);

  // Proxy constructor.
  if (LLVM_UNLIKELY(runtime.hasES6Proxy())) {
    createProxyConstructor(runtime);
  }

  // Define the global Math object
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::Math),
      normalDPF,
      createMathObject(runtime)));

  // Define the global JSON object
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::JSON),
      normalDPF,
      createJSONObject(runtime)));

  if (LLVM_UNLIKELY(runtime.hasES6Proxy())) {
    // Define the global Reflect object
    runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
        runtime.getGlobal(),
        runtime,
        Predefined::getSymbolID(Predefined::Reflect),
        normalDPF,
        createReflectObject(runtime)));
  }

  // Define the global %HermesInternal object.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::HermesInternal),
      constantDPF,
      createHermesInternalObject(runtime, jsLibFlags)));

#ifdef HERMES_ENABLE_DEBUGGER

  // Define the global %DebuggerInternal object.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      runtime.getIdentifierTable().registerLazyIdentifier(
          createASCIIRef("DebuggerInternal")),
      constantDPF,
      createDebuggerInternalObject(runtime)));

#endif // HERMES_ENABLE_DEBUGGER

  // Define the 'print' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::print), print, 1);

  // Define the 'eval' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::eval), eval, 1);

  // Define the 'isNaN' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::isNaN), isNaN, 1);

  // Define the 'isFinite' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::isFinite), isFinite, 1);

  // Define the 'escape' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::escape), escape, 1);

  // Define the 'unescape' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::unescape), unescape, 1);

  // Define the 'atob' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::atob), atob, 1);

  // Define the 'btoa' function.
  defineGlobalFunc(Predefined::getSymbolID(Predefined::btoa), btoa, 1);

  // Define the 'decodeURI' function.
  defineGlobalFunc(
      Predefined::getSymbolID(Predefined::decodeURI), decodeURI, 1);

  // Define the 'decodeURIComponent' function.
  defineGlobalFunc(
      Predefined::getSymbolID(Predefined::decodeURIComponent),
      decodeURIComponent,
      1);

  // Define the 'encodeURI' function.
  defineGlobalFunc(
      Predefined::getSymbolID(Predefined::encodeURI), encodeURI, 1);

  // Define the 'encodeURIComponent' function.
  defineGlobalFunc(
      Predefined::getSymbolID(Predefined::encodeURIComponent),
      encodeURIComponent,
      1);

  // Define the 'globalThis' property.
  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::globalThis),
      normalDPF,
      runtime.getGlobal()));

  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::SHBuiltin),
      constantDPF,
      Runtime::getUndefinedValue()));

  // Define the 'require' function.
  runtime.requireFunction = NativeFunction::create(
      runtime,
      runtime.functionPrototype,
      nullptr,
      require,
      Predefined::getSymbolID(Predefined::require),
      1,
      Runtime::makeNullHandle<JSObject>());

  if (jsLibFlags.enableHermesInternal) {
    // Define the 'gc' function.
    defineGlobalFunc(Predefined::getSymbolID(Predefined::gc), gc, 0);
  }

#ifdef HERMES_ENABLE_INTL
  // Define the global Intl object
  // TODO T65916424: Consider how we can move this somewhere more modular.
  if (runtime.hasIntl()) {
    runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
        runtime.getGlobal(),
        runtime,
        Predefined::getSymbolID(Predefined::Intl),
        normalDPF,
        intl::createIntlObject(runtime)));
  }
#endif
}

} // namespace vm
} // namespace hermes
