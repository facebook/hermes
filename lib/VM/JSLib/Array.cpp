/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.4 Initialize the Array constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/ADT/SafeInt.h"
#include "hermes/VM/HandleRootOwner-inline.h"
#include "hermes/VM/JSLib/Sorting.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/SmallHermesValue.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/StringView.h"

#include "llvh/ADT/ScopeExit.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
/// Array.

Handle<NativeConstructor> createArrayConstructor(Runtime &runtime) {
  auto arrayPrototype = Handle<JSArray>::vmcast(&runtime.arrayPrototype);

  // Array.prototype.xxx methods.
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::toString),
      nullptr,
      arrayPrototypeToString,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::toLocaleString),
      nullptr,
      arrayPrototypeToLocaleString,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::at),
      nullptr,
      arrayPrototypeAt,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::concat),
      nullptr,
      arrayPrototypeConcat,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::join),
      nullptr,
      arrayPrototypeJoin,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::push),
      nullptr,
      arrayPrototypePush,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::sort),
      nullptr,
      arrayPrototypeSort,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::forEach),
      nullptr,
      arrayPrototypeForEach,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::flat),
      nullptr,
      arrayPrototypeFlat,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::flatMap),
      nullptr,
      arrayPrototypeFlatMap,
      1);

  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::keys),
      (void *)IterationKind::Key,
      arrayPrototypeIterator,
      0);
  runtime.arrayPrototypeValues = defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::values),
      (void *)IterationKind::Value,
      arrayPrototypeIterator,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::entries),
      (void *)IterationKind::Entry,
      arrayPrototypeIterator,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::toReversed),
      nullptr,
      arrayPrototypeToReversed,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::toSpliced),
      nullptr,
      arrayPrototypeToSpliced,
      2);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::with),
      nullptr,
      arrayPrototypeWith,
      2);

  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();

  runtime.ignoreAllocationFailure(JSObject::defineOwnProperty(
      arrayPrototype,
      runtime,
      Predefined::getSymbolID(Predefined::SymbolIterator),
      dpf,
      runtime.arrayPrototypeValues));

  auto cons = defineSystemConstructor<JSArray>(
      runtime,
      Predefined::getSymbolID(Predefined::Array),
      arrayConstructor,
      arrayPrototype,
      1,
      CellKind::JSArrayKind);

  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isArray),
      nullptr,
      arrayIsArray,
      1);

  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::slice),
      nullptr,
      arrayPrototypeSlice,
      2);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::splice),
      nullptr,
      arrayPrototypeSplice,
      2);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::copyWithin),
      nullptr,
      arrayPrototypeCopyWithin,
      2);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::pop),
      nullptr,
      arrayPrototypePop,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::shift),
      nullptr,
      arrayPrototypeShift,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::unshift),
      nullptr,
      arrayPrototypeUnshift,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::indexOf),
      nullptr,
      arrayPrototypeIndexOf,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::lastIndexOf),
      nullptr,
      arrayPrototypeLastIndexOf,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::every),
      nullptr,
      arrayPrototypeEvery,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::some),
      nullptr,
      arrayPrototypeSome,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::map),
      nullptr,
      arrayPrototypeMap,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::filter),
      nullptr,
      arrayPrototypeFilter,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::fill),
      nullptr,
      arrayPrototypeFill,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::find),
      nullptr,
      arrayPrototypeFind,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::findIndex),
      // Pass a non-null pointer here to indicate we're finding the index.
      (void *)true,
      arrayPrototypeFind,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::findLast),
      nullptr,
      arrayPrototypeFindLast,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::findLastIndex),
      // Pass a non-null pointer here to indicate we're finding the index.
      (void *)true,
      arrayPrototypeFindLast,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::reduce),
      nullptr,
      arrayPrototypeReduce,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::reduceRight),
      nullptr,
      arrayPrototypeReduceRight,
      1);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::reverse),
      nullptr,
      arrayPrototypeReverse,
      0);
  defineMethod(
      runtime,
      arrayPrototype,
      Predefined::getSymbolID(Predefined::includes),
      nullptr,
      arrayPrototypeIncludes,
      1);

  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::of),
      nullptr,
      arrayOf,
      0);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::from),
      nullptr,
      arrayFrom,
      1);

  return cons;
}

CallResult<HermesValue>
arrayConstructor(void *, Runtime &runtime, NativeArgs args) {
  struct : Locals {
    PinnedValue<JSArray> self;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  // If constructor, use the allocated object, otherwise allocate a new one.
  // Everything else is the same after that.
  if (args.isConstructorCall())
    lv.self = vmcast<JSArray>(args.getThisArg());
  else {
    auto arrRes = JSArray::create(runtime, 0, 0);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.self = std::move(*arrRes);
  }

  // Possibility 1: new Array(number)
  if (args.getArgCount() == 1 && args.getArg(0).isNumber()) {
    double number = args.getArg(0).getNumber();
    uint32_t len = truncateToUInt32(number);
    if (len != number) {
      return runtime.raiseRangeError("invalid array length");
    }

    auto st = JSArray::setLengthProperty(lv.self, runtime, len);
    (void)st;
    assert(
        st != ExecutionStatus::EXCEPTION && *st &&
        "Cannot set length of a new array");

    return lv.self.getHermesValue();
  }

  // Possibility 2: new Array(elements...)
  uint32_t len = args.getArgCount();

  // Resize the array.
  auto st = JSArray::setLengthProperty(lv.self, runtime, len);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && *st &&
      "Cannot set length of a new array");

  // Initialize the elements.
  uint32_t index = 0;
  GCScopeMarkerRAII marker(runtime);
  for (Handle<> arg : args.handles()) {
    JSArray::setElementAt(lv.self, runtime, index++, arg);
    marker.flush();
  }

  return lv.self.getHermesValue();
}

CallResult<HermesValue>
arrayIsArray(void *, Runtime &runtime, NativeArgs args) {
  CallResult<bool> res = isArray(runtime, dyn_vmcast<JSObject>(args.getArg(0)));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(*res);
}

/// ES5.1 15.4.4.5.
CallResult<HermesValue>
arrayPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  struct : Locals {
    PinnedValue<JSObject> array;
    PinnedValue<Callable> func;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  lv.array = vmcast<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      lv.array, runtime, Predefined::getSymbolID(Predefined::join));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (vmisa<Callable>(propRes->getHermesValue())) {
    lv.func = vmcast<Callable>(propRes->getHermesValue());
    return Callable::executeCall0(lv.func, runtime, lv.array)
        .toCallResultHermesValue();
  } else {
    // If not callable, set func to be Object.prototype.toString.
    return directObjectPrototypeToString(runtime, lv.array);
  }
}

CallResult<HermesValue>
arrayPrototypeToLocaleString(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  struct : Locals {
    PinnedValue<JSObject> array;
    PinnedValue<> E;
    PinnedValue<JSObject> elementObj;
    PinnedValue<> length;
    PinnedValue<> i;
    PinnedValue<Callable> func;
    PinnedValue<JSArray> strings;
    PinnedValue<StringPrimitive> strElement;
    PinnedValue<> tmpValue;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.array = vmcast<JSObject>(objRes.getValue());

  auto emptyString = runtime.getPredefinedStringHandle(Predefined::emptyString);

  if (runtime.insertVisitedObject(*lv.array))
    return emptyString.getHermesValue();
  auto cycleScope = llvh::make_scope_exit(
      [&lv, &runtime] { runtime.removeVisitedObject(*lv.array); });

  auto propRes = JSObject::getNamed_RJS(
      lv.array, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.length = std::move(*propRes);
  auto intRes = toUInt32_RJS(runtime, lv.length);
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t len = intRes->getNumber();

  // TODO: Get a list-separator String for the host environment's locale.
  // Use a comma as a separator for now, as JSC does.
  const char16_t separator = u',';

  // Final size of the result string. Initialize to account for the separators.
  SafeUInt32 size(len - 1);

  if (len == 0) {
    return emptyString.getHermesValue();
  }

  // Array to store each of the strings of the elements.
  auto arrRes = JSArray::create(runtime, len, len);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.strings = std::move(*arrRes);

  // Index into the array.
  lv.i = HermesValue::encodeTrustedNumberValue(0);

  auto marker = gcScope.createMarker();
  while (lv.i->getNumber() < len) {
    gcScope.flushToMarker(marker);
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getComputed_RJS(lv.array, runtime, lv.i)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.E = std::move(*propRes);
    if (lv.E->isUndefined() || lv.E->isNull()) {
      // Empty string for undefined or null element. No need to add to size.
      JSArray::setElementAt(
          lv.strings, runtime, lv.i->getNumber(), emptyString);
    } else {
      if (LLVM_UNLIKELY(
              (objRes = toObject(runtime, lv.E)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.elementObj = vmcast<JSObject>(objRes.getValue());

      // Retrieve the toLocaleString function.
      if (LLVM_UNLIKELY(
              (propRes = JSObject::getNamed_RJS(
                   lv.elementObj,
                   runtime,
                   Predefined::getSymbolID(Predefined::toLocaleString))) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (vmisa<Callable>(propRes->get())) {
        lv.func = vmcast<Callable>(propRes->get());
        // If ECMA 402 is implemented, it provides a superseding
        // definition of Array.prototype.toLocaleString.  The only
        // difference between these two definitions is that in ECMA
        // 402, two arguments (locales and options), if provided, are
        // passed on from this function to the element's
        // "toLocaleString" method.
        auto callRes =
#ifdef HERMES_ENABLE_INTL
            Callable::executeCall2(
                lv.func,
                runtime,
                lv.elementObj,
                args.getArg(0),
                args.getArg(1));
#else
            Callable::executeCall0(lv.func, runtime, lv.elementObj);
#endif
        if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lv.tmpValue = std::move(*callRes);
        auto strRes = toString_RJS(runtime, lv.tmpValue);
        if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lv.strElement = std::move(*strRes);
        uint32_t strLength = lv.strElement->getStringLength();
        // Throw RangeError on overflow.
        size.add(strLength);
        if (LLVM_UNLIKELY(size.isOverflowed())) {
          return runtime.raiseRangeError(
              "resulting string length exceeds limit");
        }
        JSArray::setElementAt(
            lv.strings, runtime, lv.i->getNumber(), lv.strElement);
      } else {
        return runtime.raiseTypeError("toLocaleString() not callable");
      }
    }
    lv.i = HermesValue::encodeTrustedNumberValue(lv.i->getNumber() + 1);
  }

  // Create and then populate the result string.
  auto builder = StringBuilder::createStringBuilder(runtime, size);
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.strElement = lv.strings->at(runtime, 0).getString(runtime);
  builder->appendStringPrim(lv.strElement);
  for (uint32_t j = 1; j < len; ++j) {
    // Every element after the first needs a separator before it.
    builder->appendCharacter(separator);
    lv.strElement = lv.strings->at(runtime, j).getString(runtime);
    builder->appendStringPrim(lv.strElement);
  }
  return HermesValue::encodeStringValue(*builder->getStringPrimitive());
}

static inline CallResult<uint64_t>
lengthOfArrayLike(Runtime &runtime, Handle<JSObject> O, Handle<JSArray> jsArr) {
  if (LLVM_LIKELY(jsArr)) {
    // Fast path for getting the length.
    return JSArray::getLength(jsArr.get(), runtime);
  }
  struct : Locals {
    PinnedValue<> length;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  // Slow path
  CallResult<PseudoHandle<>> propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  lv.length = std::move(*propRes);
  auto lenRes = toLength(runtime, lv.length);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return lenRes->getNumber();
}

// 23.1.3.1
CallResult<HermesValue>
arrayPrototypeAt(void *, Runtime &runtime, NativeArgs args) {
  struct : Locals {
    PinnedValue<JSObject> O;
    PinnedValue<> nameVal;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  GCScope gcScope(runtime);
  // 1. Let O be ? ToObject(this value).
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.O = vmcast<JSObject>(*objRes);

  // 2. Let len be ? LengthOfArrayLike(O).
  auto jsArr = vmisa<JSArray>(*lv.O) ? Handle<JSArray>::vmcast(&lv.O)
                                     : Runtime::makeNullHandle<JSArray>();
  auto lenRes = lengthOfArrayLike(runtime, lv.O, jsArr);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = lenRes.getValue();

  // 3. Let relativeIndex be ? ToIntegerOrInfinity(index).
  auto idx = args.getArgHandle(0);
  auto relativeIndexRes = toIntegerOrInfinity(runtime, idx);
  if (relativeIndexRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  const double relativeIndex = relativeIndexRes->getNumber();

  double k;
  // 4. If relativeIndex ≥ 0, then
  if (relativeIndex >= 0) {
    // a. Let k be relativeIndex.
    k = relativeIndex;
  } else {
    // 5. Else,
    // a. Let k be len + relativeIndex.
    k = len + relativeIndex;
  }

  // 6. If k < 0 or k ≥ len, return undefined.
  if (k < 0 || k >= len) {
    return HermesValue::encodeUndefinedValue();
  }

  // 7. Return ? Get(O, ! ToString(𝔽(k))).
  if (LLVM_LIKELY(jsArr)) {
    const SmallHermesValue elm = jsArr->at(runtime, k);
    // If the element is not empty, we can return it directly here. Otherwise,
    // we must proceed to the slow path.
    if (!elm.isEmpty()) {
      return elm.unboxToHV(runtime);
    }
  }
  lv.nameVal = HermesValue::encodeTrustedNumberValue(k);
  CallResult<PseudoHandle<>> propRes =
      JSObject::getComputed_RJS(lv.O, runtime, lv.nameVal);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return propRes->getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeConcat(void *, Runtime &runtime, NativeArgs args) {
  struct : Locals {
    PinnedValue<JSObject> O;
    PinnedValue<JSArray> A;
    PinnedValue<> k;
    PinnedValue<> n;
    PinnedValue<> tmp;
    PinnedValue<JSObject> obj;
    PinnedValue<JSObject> prop;
    PinnedValue<SymbolID> tmpPropNameStorage;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.O = vmcast<JSObject>(*objRes);

  // Need a signed type here to account for uint32 and -1.
  int64_t argCount = args.getArgCount();

  // Precompute the final size of the array so it can be preallocated.
  // Note this is necessarily an estimate because an accessor on one array
  // may change the length of subsequent arrays.
  SafeUInt32 finalSizeEstimate{0};
  if (vmisa<JSArray>(*lv.O)) {
    auto arr = vmcast<JSArray>(*lv.O);
    finalSizeEstimate.add(JSArray::getLength(arr, runtime));
  } else {
    finalSizeEstimate.add(1);
  }
  for (int64_t i = 0; i < argCount; ++i) {
    if (JSArray *arr = dyn_vmcast<JSArray>(args.getArg(i))) {
      finalSizeEstimate.add(JSArray::getLength(arr, runtime));
    } else {
      finalSizeEstimate.add(1);
    }
  }
  if (finalSizeEstimate.isOverflowed()) {
    return runtime.raiseTypeError("Array.prototype.concat result out of space");
  }

  // Resultant array.
  auto arrRes =
      JSArray::create(runtime, *finalSizeEstimate, *finalSizeEstimate);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.A = std::move(*arrRes);

  // Index to insert into A.
  uint64_t n = 0;
  auto marker = gcScope.createMarker();
  ComputedPropertyDescriptor desc;

  // Loop first through the "this" value and then through the arguments.
  // If i == -1, use the "this" value, else use the ith argument.
  lv.tmp = lv.O.getHermesValue();
  for (int64_t i = -1; i < argCount; ++i, lv.tmp = args.getArg(i)) {
    CallResult<bool> spreadable = isConcatSpreadable(runtime, lv.tmp);
    if (LLVM_UNLIKELY(spreadable == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (*spreadable) {
      // 7.d. If spreadable is true, then
      lv.obj = vmcast<JSObject>(*lv.tmp);
      bool isArray = vmisa<JSArray>(*lv.obj);
      uint64_t len;
      if (LLVM_LIKELY(isArray)) {
        auto *arr = vmcast<JSArray>(*lv.obj);
        // Fast path: E is an array.
        len = JSArray::getLength(arr, runtime);
      } else {
        CallResult<PseudoHandle<>> propRes = JSObject::getNamed_RJS(
            lv.obj, runtime, Predefined::getSymbolID(Predefined::length));
        if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lv.tmp = std::move(*propRes);
        auto lengthRes = toLength(runtime, lv.tmp);
        if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        len = lengthRes->getNumberAs<uint64_t>();
      }

      // 5.c.iii. If n + len > 2^53 - 1, throw a TypeError exception
      if (LLVM_UNLIKELY(n + len > ((uint64_t)1 << 53) - 1)) {
        return runtime.raiseTypeError(
            "Array.prototype.concat result out of space");
      }

      // We know we are going to set elements in the range [n, n+len),
      // regardless of any changes to 'arrHandle' (see ES5.1 15.4.4.4). Ensure
      // we have capacity.
      if (LLVM_UNLIKELY(n + len > lv.A->getEndIndex()) &&
          LLVM_LIKELY(n + len < UINT32_MAX)) {
        // Only set the endIndex if it's going to be a valid length.
        if (LLVM_UNLIKELY(
                lv.A->setStorageEndIndex(lv.A, runtime, n + len) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }

      // Note that we must increase n every iteration even if nothing was
      // appended to the result array.
      // 5.c.iv. Repeat, while k < len
      for (uint64_t k = 0; k < len; ++k, ++n) {
        SmallHermesValue subElement = LLVM_LIKELY(isArray)
            ? vmcast<JSArray>(*lv.obj)->at(runtime, k)
            : SmallHermesValue::encodeEmptyValue();
        if (LLVM_LIKELY(!subElement.isEmpty()) &&
            LLVM_LIKELY(n < lv.A->getEndIndex())) {
          // Fast path: quickly set element without making any extra calls.
          // Cast is safe because A->getEndIndex must be in uint32_t range.
          JSArray::unsafeSetExistingElementAt(
              lv.A.get(), runtime, static_cast<uint32_t>(n), subElement);
        } else {
          // Slow path fallback if there's an empty slot in arr.
          // We have to use getComputedPrimitiveDescriptor because the property
          // may exist anywhere in the prototype chain.
          lv.k = HermesValue::encodeTrustedNumberValue(k);
          MutableHandle<JSObject> propObj{lv.prop};
          MutableHandle<SymbolID> tmpPropNameStorage{lv.tmpPropNameStorage};
          JSObject::getComputedPrimitiveDescriptor(
              lv.obj, runtime, lv.k, propObj, tmpPropNameStorage, desc);
          CallResult<PseudoHandle<>> propRes =
              JSObject::getComputedPropertyValue_RJS(
                  lv.obj, runtime, propObj, tmpPropNameStorage, desc, lv.k);
          if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
            return ExecutionStatus::EXCEPTION;
          }
          if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
            lv.tmp = std::move(*propRes);
            lv.n = HermesValue::encodeTrustedNumberValue(n);
            if (LLVM_UNLIKELY(
                    JSArray::defineOwnComputedPrimitive(
                        lv.A,
                        runtime,
                        lv.n,
                        DefinePropertyFlags::getDefaultNewPropertyFlags(),
                        lv.tmp) == ExecutionStatus::EXCEPTION)) {
              return ExecutionStatus::EXCEPTION;
            }
          }
          gcScope.flushToMarker(marker);
        }
      }
      gcScope.flushToMarker(marker);
    } else {
      // 5.d.i. NOTE: E is added as a single item rather than spread.
      // 5.d.ii. If n >= 2**53 - 1, throw a TypeError exception.
      if (LLVM_UNLIKELY(n >= ((uint64_t)1 << 53) - 1)) {
        return runtime.raiseTypeError(
            "Array.prototype.concat result out of space");
      }
      // Otherwise, just put the value into the next slot.
      if (LLVM_LIKELY(n < UINT32_MAX)) {
        JSArray::setElementAt(lv.A, runtime, n, lv.tmp);
      } else {
        lv.n = HermesValue::encodeTrustedNumberValue(n);
        auto cr = valueToSymbolID(runtime, lv.n);
        if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        if (LLVM_UNLIKELY(
                JSArray::defineOwnProperty(
                    lv.A,
                    runtime,
                    **cr,
                    DefinePropertyFlags::getDefaultNewPropertyFlags(),
                    lv.tmp) == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      gcScope.flushToMarker(marker);
      ++n;
    }
  }
  // Update the array's length. We never expect this to fail since we just
  // created the array.
  if (n > UINT32_MAX) {
    return runtime.raiseRangeError("invalid array length");
  }
  auto res =
      JSArray::setLengthProperty(lv.A, runtime, static_cast<uint32_t>(n));
  assert(
      res == ExecutionStatus::RETURNED &&
      "Setting length of new array should never fail");
  (void)res;
  return lv.A.getHermesValue();
}

/// ES5.1 15.4.4.5.
CallResult<HermesValue>
arrayPrototypeJoin(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto emptyString = runtime.getPredefinedStringHandle(Predefined::emptyString);

  if (runtime.insertVisitedObject(*O))
    return emptyString.getHermesValue();
  auto cycleScope =
      llvh::make_scope_exit([&] { runtime.removeVisitedObject(*O); });

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  // Use comma for separator if the first argument is undefined.
  auto separator = args.getArg(0).isUndefined()
      ? runtime.makeHandle(HermesValue::encodeStringValue(
            runtime.getPredefinedString(Predefined::comma)))
      : args.getArgHandle(0);
  auto strRes = toString_RJS(runtime, separator);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto sep = runtime.makeHandle(std::move(*strRes));

  if (len == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // Track the size of the resultant string. Use a 64-bit value to detect
  // overflow.
  SafeUInt32 size;

  // Storage for the strings for each element.
  if (LLVM_UNLIKELY(len > JSArray::StorageType::maxElements())) {
    return runtime.raiseRangeError("Out of memory for array elements.");
  }
  auto arrRes = JSArray::create(runtime, len, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> strings = runtime.makeHandle(std::move(*arrRes));

  // Call toString on all the elements of the array.
  for (MutableHandle<> i{runtime, HermesValue::encodeTrustedNumberValue(0)};
       i->getNumber() < len;
       i = HermesValue::encodeTrustedNumberValue(i->getNumber() + 1)) {
    // Add the size of the separator, except the first time.
    if (i->getNumberAs<uint32_t>())
      size.add(sep->getStringLength());

    GCScope gcScope2(runtime);
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getComputed_RJS(O, runtime, i)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    auto elem = runtime.makeHandle(std::move(*propRes));

    if (elem->isUndefined() || elem->isNull()) {
      JSArray::setElementAt(strings, runtime, i->getNumber(), emptyString);
    } else {
      // Otherwise, call toString_RJS() and push the result, incrementing size.
      auto strRes = toString_RJS(runtime, elem);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto S = runtime.makeHandle(std::move(*strRes));
      size.add(S->getStringLength());
      JSArray::setElementAt(strings, runtime, i->getNumber(), S);
    }

    // Check for string overflow on every iteration to create the illusion that
    // we are appending to the string. Also, prevent uint32_t overflow.
    if (size.isOverflowed()) {
      return runtime.raiseRangeError("String is too long");
    }
  }

  // Allocate the complete result.
  auto builder = StringBuilder::createStringBuilder(runtime, size);
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  MutableHandle<StringPrimitive> element{runtime};
  element = strings->at(runtime, 0).getString(runtime);
  builder->appendStringPrim(element);
  for (size_t i = 1; i < len; ++i) {
    builder->appendStringPrim(sep);
    element = strings->at(runtime, i).getString(runtime);
    builder->appendStringPrim(element);
  }
  return HermesValue::encodeStringValue(*builder->getStringPrimitive());
}

/// ES9.0 22.1.3.18.
CallResult<HermesValue>
arrayPrototypePush(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  // 1. Let O be ? ToObject(this value).
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  MutableHandle<> len{runtime};

  // 2. Let len be ? ToLength(? Get(O, "length")).
  Handle<JSArray> arr = Handle<JSArray>::dyn_vmcast(O);
  if (LLVM_LIKELY(arr)) {
    // Fast path for getting the length.
    len = HermesValue::encodeTrustedNumberValue(
        JSArray::getLength(arr.get(), runtime));
  } else {
    // Slow path, used when pushing onto non-array objects.
    auto propRes = JSObject::getNamed_RJS(
        O, runtime, Predefined::getSymbolID(Predefined::length));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto lenRes = toLength(runtime, runtime.makeHandle(std::move(*propRes)));
    if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    len = lenRes.getValue();
  }

  // 3. Let items be a List whose elements are, in left to right order, the
  // arguments that were passed to this function invocation.
  // 4. Let argCount be the number of elements in items.
  uint32_t argCount = args.getArgCount();

  // 5. If len + argCount > 2**53-1, throw a TypeError exception.
  if (len->getNumber() + (double)argCount > std::pow(2.0, 53) - 1) {
    return runtime.raiseTypeError("Array length exceeded in push()");
  }

  auto marker = gcScope.createMarker();
  // 6. Repeat, while items is not empty
  for (auto arg : args.handles()) {
    // a. Remove the first element from items and let E be the value of the
    // element.
    // b. Perform ? Set(O, ! ToString(len), E, true).
    // NOTE: If the prototype has an index-like non-writable property at
    // index n, we have to fail to push.
    // If the prototype has an index-like accessor at index n,
    // then we have to attempt to call the setter.
    // Must call putComputed because the array prototype could have values for
    // keys that haven't been inserted into O yet.
    if (LLVM_UNLIKELY(
            JSObject::putComputed_RJS(
                O, runtime, len, arg, PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    gcScope.flushToMarker(marker);
    // c. Let len be len+1.
    len = HermesValue::encodeTrustedNumberValue(len->getNumber() + 1);
  }

  // 7. Perform ? Set(O, "length", len, true).
  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              O,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              len,
              PropOpFlags().plusThrowOnError()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 8. Return len.
  return len.get();
}

namespace {
/// General object sorting model used by custom sorting routines.
/// Provides a model by which to less and swap elements, using the [[Get]],
/// [[Put]], and [[Delete]] internal methods of a supplied Object. Should be
/// allocated on the stack, because it creates its own internal GCScope, with
/// reusable MutableHandle<>-s that are used in the less and swap methods.
/// These allow for quick accesses without allocating a great number of new
/// handles every time we want to compare different elements.
/// Usage example:
///   StandardSortModel sm{runtime, obj, compareFn};
///   quickSort(sm, 0, length);
/// Note that this is generic and does nothing different if passed a JSArray.
class StandardSortModel : public SortModel {
 private:
  /// Runtime to sort in.
  Runtime &runtime_;

  /// Scope to allocate handles in, gets destroyed with this.
  GCScope gcScope_;

  /// JS comparison function, return -1 for less, 0 for equal, 1 for greater.
  /// If null, then use the built in < operator.
  Handle<Callable> compareFn_;

  /// Object to sort elements [0, length).
  Handle<JSObject> obj_;

  /// Temporary handles for property name.
  MutableHandle<SymbolID> aTmpNameStorage_;
  MutableHandle<SymbolID> bTmpNameStorage_;

  /// Preallocate handles in the current GCScope so that we don't have to make
  /// new handles in every method call.

  /// Handles for two indices.
  MutableHandle<> aHandle_;
  MutableHandle<> bHandle_;

  /// Handles for the values at two indices.
  MutableHandle<> aValue_;
  MutableHandle<> bValue_;

  /// Handles for the objects the values are retrieved from.
  MutableHandle<JSObject> aDescObjHandle_;
  MutableHandle<JSObject> bDescObjHandle_;

  /// Marker created after initializing all fields so handles allocated later
  /// can be flushed.
  GCScope::Marker gcMarker_;

 public:
  StandardSortModel(
      Runtime &runtime,
      Handle<JSObject> obj,
      Handle<Callable> compareFn)
      : runtime_(runtime),
        gcScope_(runtime),
        compareFn_(compareFn),
        obj_(obj),
        aTmpNameStorage_(runtime),
        bTmpNameStorage_(runtime),
        aHandle_(runtime),
        bHandle_(runtime),
        aValue_(runtime),
        bValue_(runtime),
        aDescObjHandle_(runtime),
        bDescObjHandle_(runtime),
        gcMarker_(gcScope_.createMarker()) {}

  /// Use getComputed and putComputed to swap the values at obj[a] and obj[b].
  ExecutionStatus swap(uint32_t a, uint32_t b) override {
    // Ensure that we don't leave here with any new handles.
    GCScopeMarkerRAII gcMarker{gcScope_, gcMarker_};

    aHandle_ = HermesValue::encodeTrustedNumberValue(a);
    bHandle_ = HermesValue::encodeTrustedNumberValue(b);

    ComputedPropertyDescriptor aDesc;
    JSObject::getComputedPrimitiveDescriptor(
        obj_, runtime_, aHandle_, aDescObjHandle_, aTmpNameStorage_, aDesc);

    if (aDescObjHandle_) {
      if (LLVM_LIKELY(!aDesc.flags.proxyObject)) {
        auto res = JSObject::getComputedPropertyValue_RJS(
            obj_,
            runtime_,
            aDescObjHandle_,
            aTmpNameStorage_,
            aDesc,
            aDescObjHandle_);
        if (res == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        if (LLVM_LIKELY(!(*res)->isEmpty())) {
          aValue_ = std::move(*res);
        }
      } else {
        auto keyRes = toPropertyKey(runtime_, aHandle_);
        if (keyRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        aHandle_ = keyRes->get();
        CallResult<bool> hasPropRes = JSProxy::getOwnProperty(
            aDescObjHandle_, runtime_, aHandle_, aDesc, nullptr);
        if (hasPropRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        if (*hasPropRes) {
          auto res =
              JSProxy::getComputed(aDescObjHandle_, runtime_, aHandle_, obj_);
          if (res == ExecutionStatus::EXCEPTION) {
            return ExecutionStatus::EXCEPTION;
          }
          aValue_ = std::move(*res);
        } else {
          aDescObjHandle_ = nullptr;
        }
      }
    }

    ComputedPropertyDescriptor bDesc;
    JSObject::getComputedPrimitiveDescriptor(
        obj_, runtime_, bHandle_, bDescObjHandle_, bTmpNameStorage_, bDesc);

    if (bDescObjHandle_) {
      if (LLVM_LIKELY(!bDesc.flags.proxyObject)) {
        auto res = JSObject::getComputedPropertyValue_RJS(
            obj_,
            runtime_,
            bDescObjHandle_,
            bTmpNameStorage_,
            bDesc,
            bDescObjHandle_);
        if (res == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        if (LLVM_LIKELY(!(*res)->isEmpty())) {
          bValue_ = std::move(*res);
        }
      } else {
        auto keyRes = toPropertyKey(runtime_, bHandle_);
        if (keyRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        bHandle_ = keyRes->get();
        CallResult<bool> hasPropRes = JSProxy::getOwnProperty(
            bDescObjHandle_, runtime_, bHandle_, bDesc, nullptr);
        if (hasPropRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        if (*hasPropRes) {
          auto res =
              JSProxy::getComputed(bDescObjHandle_, runtime_, bHandle_, obj_);
          if (res == ExecutionStatus::EXCEPTION) {
            return ExecutionStatus::EXCEPTION;
          }
          bValue_ = std::move(*res);
        } else {
          bDescObjHandle_ = nullptr;
        }
      }
    }

    if (bDescObjHandle_) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  obj_,
                  runtime_,
                  aHandle_,
                  bValue_,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  obj_, runtime_, aHandle_, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    if (aDescObjHandle_) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  obj_,
                  runtime_,
                  bHandle_,
                  aValue_,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  obj_, runtime_, bHandle_, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    return ExecutionStatus::RETURNED;
  }

  /// If compareFn isn't null, return compareFn(obj[a], obj[b])
  /// If compareFn is null, return -1 if obj[a] < obj[b], 1 if obj[a] > obj[b],
  /// 0 otherwise
  CallResult<int> compare(uint32_t a, uint32_t b) override {
    // Ensure that we don't leave here with any new handles.
    GCScopeMarkerRAII gcMarker{gcScope_, gcMarker_};

    aHandle_ = HermesValue::encodeTrustedNumberValue(a);
    bHandle_ = HermesValue::encodeTrustedNumberValue(b);

    ComputedPropertyDescriptor aDesc;
    JSObject::getComputedPrimitiveDescriptor(
        obj_, runtime_, aHandle_, aDescObjHandle_, aTmpNameStorage_, aDesc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        obj_, runtime_, aDescObjHandle_, aTmpNameStorage_, aDesc, aHandle_);
    if (propRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if ((*propRes)->isEmpty()) {
      // Spec defines empty as greater than everything.
      return 1;
    }
    aValue_ = std::move(*propRes);
    assert(!aValue_->isEmpty());

    ComputedPropertyDescriptor bDesc;
    JSObject::getComputedPrimitiveDescriptor(
        obj_, runtime_, bHandle_, bDescObjHandle_, bTmpNameStorage_, bDesc);
    if ((propRes = JSObject::getComputedPropertyValue_RJS(
             obj_,
             runtime_,
             bDescObjHandle_,
             bTmpNameStorage_,
             bDesc,
             bHandle_)) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if ((*propRes)->isEmpty()) {
      // Spec defines empty as greater than everything.
      return -1;
    }
    bValue_ = std::move(*propRes);
    assert(!bValue_->isEmpty());

    if (aValue_->isUndefined()) {
      // Spec defines undefined as greater than everything.
      return 1;
    }
    if (bValue_->isUndefined()) {
      // Spec defines undefined as greater than everything.
      return -1;
    }

    if (compareFn_) {
      // If we have a compareFn, just use that.
      auto callRes = Callable::executeCall2(
          compareFn_,
          runtime_,
          Runtime::getUndefinedValue(),
          aValue_.get(),
          bValue_.get());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto intRes =
          toNumber_RJS(runtime_, runtime_.makeHandle(std::move(*callRes)));
      if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // Cannot return intRes's value directly because it can be NaN
      auto res = intRes->getNumber();
      return (res < 0) ? -1 : (res > 0 ? 1 : 0);
    } else {
      // Convert both arguments to strings and compare
      auto aValueRes = toString_RJS(runtime_, aValue_);
      if (LLVM_UNLIKELY(aValueRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      aValue_ = aValueRes->getHermesValue();

      auto bValueRes = toString_RJS(runtime_, bValue_);
      if (LLVM_UNLIKELY(bValueRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      bValue_ = bValueRes->getHermesValue();

      return aValue_->getString()->compare(bValue_->getString());
    }
  }
};

/// Perform a sort of a sparse object by querying its properties first.
/// It cannot be a proxy or a host object because they are not guaranteed to
/// be able to list their properties.
CallResult<HermesValue> sortSparse(
    Runtime &runtime,
    Handle<JSObject> O,
    Handle<Callable> compareFn,
    uint64_t len) {
  GCScope gcScope{runtime};

  assert(
      !O->isHostObject() && !O->isProxyObject() &&
      "only non-exotic objects can be sparsely sorted");

  // This is a "non-fast" object, meaning we need to create a symbol for every
  // property name. On the assumption that it is sparse, get all properties
  // first, so that we only have to read the existing properties.

  auto crNames = JSObject::getOwnPropertyNames(O, runtime, false);
  if (crNames == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  // Get the underlying storage containing the names.
  auto names = runtime.makeHandle((*crNames)->getIndexedStorage(runtime));
  if (!names) {
    // Indexed storage can be null if there's nothing to store.
    return O.getHermesValue();
  }

  // Find out how many sortable numeric properties we have.
  JSArray::StorageType::size_type numProps = 0;
  for (JSArray::StorageType::size_type e = names->size(runtime); numProps != e;
       ++numProps) {
    SmallHermesValue hv = names->at(runtime, numProps);
    // Stop at the first non-number.
    if (!hv.isNumber())
      break;
    // Stop if the property name is beyond "len".
    if (hv.getNumber(runtime) >= len)
      break;
  }

  // If we didn't find any numeric properties, there is nothing to do.
  if (numProps == 0)
    return O.getHermesValue();

  // Create a new array which we will actually sort.
  auto crArray = JSArray::create(runtime, numProps, numProps);
  if (crArray == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  Handle<JSArray> array = runtime.makeHandle(std::move(*crArray));
  if (JSArray::setStorageEndIndex(array, runtime, numProps) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  MutableHandle<> propName{runtime};
  MutableHandle<> propVal{runtime};
  GCScopeMarkerRAII gcMarker{gcScope};

  // Copy all sortable properties into the array and delete them from the
  // source. Deleting all sortable properties makes it easy to just copy the
  // sorted result back in the end.
  for (decltype(numProps) i = 0; i != numProps; ++i) {
    gcMarker.flush();

    propName = names->at(runtime, i).unboxToHV(runtime);
    auto res = JSObject::getComputed_RJS(O, runtime, propName);
    if (res == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    // Skip empty values.
    if (res->getHermesValue().isEmpty())
      continue;

    const auto shv = SmallHermesValue::encodeHermesValue(res->get(), runtime);
    JSArray::unsafeSetExistingElementAt(*array, runtime, i, shv);

    if (JSObject::deleteComputed(
            O, runtime, propName, PropOpFlags().plusThrowOnError()) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  gcMarker.flush();

  {
    StandardSortModel sm(runtime, array, compareFn);
    if (LLVM_UNLIKELY(
            quickSort(&sm, 0u, numProps) == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
  }

  // Time to copy back the values.
  for (decltype(numProps) i = 0; i != numProps; ++i) {
    gcMarker.flush();

    auto hv = array->at(runtime, i).unboxToHV(runtime);
    assert(
        !hv.isEmpty() &&
        "empty values cannot appear in the array out of nowhere");
    propVal = hv;

    propName = HermesValue::encodeTrustedNumberValue(i);

    if (JSObject::putComputed_RJS(
            O, runtime, propName, propVal, PropOpFlags().plusThrowOnError()) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return O.getHermesValue();
}
} // anonymous namespace

/// ES5.1 15.4.4.11.
CallResult<HermesValue>
arrayPrototypeSort(void *, Runtime &runtime, NativeArgs args) {
  // Null if not a callable compareFn.
  auto compareFn = Handle<Callable>::dyn_vmcast(args.getArgHandle(0));
  if (!args.getArg(0).isUndefined() && !compareFn) {
    return runtime.raiseTypeError("Array sort argument must be callable");
  }

  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  // If we are not sorting a regular dense array, use a special routine which
  // first copies all properties into an array.
  // Proxies  and host objects however are excluded because they are weird.
  if (!O->isProxyObject() && !O->isHostObject() && !O->hasFastIndexProperties())
    return sortSparse(runtime, O, compareFn, len);

  // This is the "fast" path. We are sorting an array with indexed storage.
  StandardSortModel sm(runtime, O, compareFn);

  // Use our custom sort routine. We can't use std::sort because it performs
  // optimizations that allow it to bypass calls to std::swap, but our swap
  // function is special, since it needs to use the internal Object functions.
  if (LLVM_UNLIKELY(quickSort(&sm, 0u, len) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  return O.getHermesValue();
}

inline CallResult<HermesValue>
arrayPrototypeForEach(void *, Runtime &runtime, NativeArgs args) {
  struct : Locals {
    PinnedValue<JSObject> O;
    PinnedValue<> length;
    PinnedValue<> k;
    PinnedValue<JSObject> descObj;
    PinnedValue<SymbolID> tmpPropNameStorage;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.O = vmcast<JSObject>(*objRes);

  auto propRes = JSObject::getNamed_RJS(
      lv.O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.length = std::move(*propRes);
  auto intRes = toLengthU64(runtime, lv.length);
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  auto callbackFn = args.dyncastArg<Callable>(0);
  if (!callbackFn) {
    return runtime.raiseTypeError(
        "Array.prototype.forEach() requires a callable argument");
  }

  // Index to execute the callback on.
  lv.k = HermesValue::encodeTrustedNumberValue(0);

  MutableHandle<JSObject> descObjHandle{lv.descObj};
  MutableHandle<SymbolID> tmpPropNameStorage{lv.tmpPropNameStorage};

  // Loop through and execute the callback on all existing values.
  // TODO: Implement a fast path for actual arrays.
  auto marker = gcScope.createMarker();
  while (lv.k->getDouble() < len) {
    gcScope.flushToMarker(marker);

    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        lv.O, runtime, lv.k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        lv.O, runtime, descObjHandle, tmpPropNameStorage, desc, lv.k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      auto kValue = std::move(*propRes);
      if (LLVM_UNLIKELY(
              Callable::executeCall3(
                  callbackFn,
                  runtime,
                  args.getArgHandle(1),
                  kValue.get(),
                  lv.k.get(),
                  lv.O.getHermesValue()) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    lv.k = HermesValue::encodeTrustedNumberValue(lv.k->getDouble() + 1);
  }

  return HermesValue::encodeUndefinedValue();
}

/// ES10 22.1.3.10.1 FlattenIntoArray
/// mapperFunction may be null to signify its absence.
/// If mapperFunction is null, thisArg is ignored.
static CallResult<uint64_t> flattenIntoArray(
    Runtime &runtime,
    Handle<JSArray> target,
    Handle<JSObject> source,
    uint64_t sourceLen,
    uint64_t start,
    double depth,
    Handle<Callable> mapperFunction,
    Handle<> thisArg) {
  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }

  if (!mapperFunction) {
    assert(
        thisArg->isUndefined() &&
        "thisArg must be undefined if there is no mapper");
  }

  GCScope gcScope{runtime};
  // 1. Let targetIndex be start.
  uint64_t targetIndex = start;
  // 2. Let sourceIndex be 0.
  uint64_t sourceIndex = 0;

  // Temporary storage for sourceIndex and targetIndex.
  MutableHandle<> indexHandle{runtime};
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<JSObject> propObj{runtime};
  MutableHandle<> element{runtime};
  MutableHandle<> lenResHandle{runtime};

  auto marker = gcScope.createMarker();

  // 3. Repeat, while sourceIndex < sourceLen
  while (sourceIndex < sourceLen) {
    gcScope.flushToMarker(marker);

    // a. Let P be ! ToString(sourceIndex).
    // b. Let exists be ? HasProperty(source, P).
    ComputedPropertyDescriptor desc{};
    indexHandle = HermesValue::encodeTrustedNumberValue(sourceIndex);
    if (LLVM_UNLIKELY(
            JSObject::getComputedDescriptor(
                source,
                runtime,
                indexHandle,
                propObj,
                tmpPropNameStorage,
                desc) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If exists is true, then
    // i. Let element be ? Get(source, P).
    CallResult<PseudoHandle<>> elementRes =
        JSObject::getComputedPropertyValue_RJS(
            source, runtime, propObj, tmpPropNameStorage, desc, indexHandle);
    if (LLVM_UNLIKELY(elementRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*elementRes)->isEmpty())) {
      element = std::move(*elementRes);

      // ii. If mapperFunction is present, then
      if (mapperFunction) {
        // 1. Assert: thisArg is present.
        assert(!thisArg->isEmpty() && "mapperFunction requires a thisArg");
        // 2. Set element to ? Call(mapperFunction, thisArg , « element,
        // sourceIndex, source »).
        elementRes = Callable::executeCall3(
            mapperFunction,
            runtime,
            thisArg,
            element.getHermesValue(),
            HermesValue::encodeTrustedNumberValue(sourceIndex),
            source.getHermesValue());
        if (LLVM_UNLIKELY(elementRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        element = std::move(*elementRes);
      }
      // iii. Let shouldFlatten be false.
      bool shouldFlatten = false;
      if (depth > 0) {
        // iv. If depth > 0, then
        // 1. Set shouldFlatten to ? IsArray(element).
        // NOTE: isArray accepts nullptr for the obj argument.
        CallResult<bool> shouldFlattenRes =
            isArray(runtime, dyn_vmcast<JSObject>(element.get()));
        if (LLVM_UNLIKELY(shouldFlattenRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        shouldFlatten = *shouldFlattenRes;
      }
      if (shouldFlatten) {
        // It is valid to cast `element` to JSObject because shouldFlatten is
        // only true when `isArray(element)` is true.
        // v. If shouldFlatten is true, then
        // 1. Let elementLen be ? ToLength(? Get(element, "length")).
        CallResult<PseudoHandle<>> lenRes = JSObject::getNamed_RJS(
            Handle<JSObject>::vmcast(element),
            runtime,
            Predefined::getSymbolID(Predefined::length));
        if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lenResHandle = std::move(*lenRes);
        CallResult<uint64_t> elementLenRes = toLengthU64(runtime, lenResHandle);
        if (LLVM_UNLIKELY(elementLenRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        uint64_t elementLen = *elementLenRes;
        // 2. Set targetIndex to ? FlattenIntoArray(target, element, elementLen,
        // targetIndex, depth - 1).
        CallResult<uint64_t> targetIndexRes = flattenIntoArray(
            runtime,
            target,
            Handle<JSObject>::vmcast(element),
            elementLen,
            targetIndex,
            depth - 1,
            Runtime::makeNullHandle<Callable>(),
            runtime.getUndefinedValue());
        if (LLVM_UNLIKELY(targetIndexRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        targetIndex = *targetIndexRes;
      } else {
        // vi. Else,
        // 1. If targetIndex ≥ 2**53-1, throw a TypeError exception.
        if (targetIndex >= ((uint64_t)1 << 53) - 1) {
          return runtime.raiseTypeError("flattened array exceeds length limit");
        }
        // 2. Perform ? CreateDataPropertyOrThrow(
        //                target, !ToString(targetIndex), element).
        indexHandle = HermesValue::encodeTrustedNumberValue(targetIndex);
        if (LLVM_UNLIKELY(
                JSObject::defineOwnComputed(
                    target,
                    runtime,
                    indexHandle,
                    DefinePropertyFlags::getDefaultNewPropertyFlags(),
                    element,
                    PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }

        // 3. Increase targetIndex by 1.
        ++targetIndex;
      }
    }
    // d. Increase sourceIndex by 1.
    ++sourceIndex;
  }
  // 4. Return targetIndex.
  return targetIndex;
}

CallResult<HermesValue>
arrayPrototypeFlat(void *ctx, Runtime &runtime, NativeArgs args) {
  // 1. Let O be ? ToObject(this value).
  CallResult<HermesValue> ORes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(ORes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*ORes);

  // 2. Let sourceLen be ? ToLength(? Get(O, "length")).
  CallResult<PseudoHandle<>> lenRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<uint64_t> sourceLenRes =
      toLengthU64(runtime, runtime.makeHandle(std::move(*lenRes)));
  if (LLVM_UNLIKELY(sourceLenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t sourceLen = *sourceLenRes;

  // 3. Let depthNum be 1.
  double depthNum = 1;
  if (!args.getArg(0).isUndefined()) {
    // 4. If depth is not undefined, then
    // a.     Set depthNum to ? ToIntegerOrInfinity(depth).
    auto depthNumRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(depthNumRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    depthNum = depthNumRes->getNumber();
  }
  // 5. Let A be ? ArraySpeciesCreate(O, 0).
  auto ARes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(ARes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*ARes));

  // 6. Perform ? FlattenIntoArray(A, O, sourceLen, 0, depthNum).
  if (LLVM_UNLIKELY(
          flattenIntoArray(
              runtime,
              A,
              O,
              sourceLen,
              0,
              depthNum,
              runtime.makeNullHandle<Callable>(),
              runtime.getUndefinedValue()) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 7. Return A.
  return A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeFlatMap(void *ctx, Runtime &runtime, NativeArgs args) {
  // 1. Let O be ? ToObject(this value).
  CallResult<HermesValue> ORes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(ORes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*ORes);

  // 2. Let sourceLen be ? ToLength(? Get(O, "length")).
  CallResult<PseudoHandle<>> lenRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<uint64_t> sourceLenRes =
      toLengthU64(runtime, runtime.makeHandle(std::move(*lenRes)));
  if (LLVM_UNLIKELY(sourceLenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t sourceLen = *sourceLenRes;

  // 3. If IsCallable(mapperFunction) is false, throw a TypeError exception.
  Handle<Callable> mapperFunction = args.dyncastArg<Callable>(0);
  if (!mapperFunction) {
    return runtime.raiseTypeError("flatMap mapper must be callable");
  }
  // 4. If thisArg is present, let T be thisArg; else let T be undefined.
  auto T = args.getArgHandle(1);
  // 5. Let A be ? ArraySpeciesCreate(O, 0).
  auto ARes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(ARes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*ARes));

  // 6. Perform ? FlattenIntoArray(A, O, sourceLen, 0, 1, mapperFunction, T).
  if (LLVM_UNLIKELY(
          flattenIntoArray(runtime, A, O, sourceLen, 0, 1, mapperFunction, T) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 7. Return A.
  return A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeIterator(void *ctx, Runtime &runtime, NativeArgs args) {
  IterationKind kind = *reinterpret_cast<IterationKind *>(&ctx);
  assert(
      kind < IterationKind::NumKinds &&
      "arrayPrototypeIterator with wrong kind");
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto obj = runtime.makeHandle<JSObject>(*objRes);
  return JSArrayIterator::create(runtime, obj, kind).getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeSlice(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *lenRes;

  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // Start index. If negative, then offset from the right side of the array.
  double relativeStart = intRes->getNumber();
  // Index that we're currently copying from.
  // Starts at the actual start value, computed from relativeStart.
  MutableHandle<> k{
      runtime,
      HermesValue::encodeTrustedNumberValue(
          relativeStart < 0 ? std::max(len + relativeStart, 0.0)
                            : std::min(relativeStart, len))};

  // End index. If negative, then offset from the right side of the array.
  double relativeEnd;
  if (args.getArg(1).isUndefined()) {
    relativeEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    relativeEnd = intRes->getNumber();
  }
  // Actual end index.
  double fin = relativeEnd < 0 ? std::max(len + relativeEnd, 0.0)
                               : std::min(relativeEnd, len);

  // Create the result array.
  double count = std::max(fin - k->getNumber(), 0.0);
  if (LLVM_UNLIKELY(count > JSArray::StorageType::maxElements())) {
    return runtime.raiseRangeError("Out of memory for array elements.");
  }
  auto arrRes = JSArray::create(runtime, count, count);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*arrRes));

  // Next index in A to write to.
  uint32_t n = 0;

  MutableHandle<JSObject> descObjHandle{runtime};
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<> kValue{runtime};
  auto marker = gcScope.createMarker();

  // Copy the elements between the actual start and end indices into A.
  // TODO: Implement a fast path for actual arrays.
  while (k->getNumber() < fin) {
    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        O, runtime, k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        O, runtime, descObjHandle, tmpPropNameStorage, desc, k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      kValue = std::move(*propRes);
      JSArray::setElementAt(A, runtime, n, kValue);
    }
    k = HermesValue::encodeTrustedNumberValue(k->getNumber() + 1);
    ++n;

    gcScope.flushToMarker(marker);
  }

  if (LLVM_UNLIKELY(
          JSArray::setLengthProperty(A, runtime, n) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeSplice(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *lenRes;

  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double relativeStart = intRes->getNumber();
  // Index to start the deletion/insertion at.
  double actualStart = relativeStart < 0 ? std::max(len + relativeStart, 0.0)
                                         : std::min(relativeStart, len);

  // Implement the newer calculation of actualDeleteCount (ES6.0),
  // since 5.1 doesn't define behavior for less than 2 arguments.
  uint32_t argCount = args.getArgCount();
  uint64_t actualDeleteCount;
  uint64_t insertCount;
  switch (argCount) {
    case 0:
      insertCount = 0;
      actualDeleteCount = 0;
      break;
    case 1:
      // If just one argument specified, delete everything until the end.
      insertCount = 0;
      actualDeleteCount = len - actualStart;
      break;
    default:
      // Otherwise, use the specified delete count.
      if (LLVM_UNLIKELY(
              (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1))) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      insertCount = argCount - 2;
      actualDeleteCount =
          std::min(std::max(intRes->getNumber(), 0.0), len - actualStart);
  }

  // If len+insertCount−actualDeleteCount > 2^53-1, throw a TypeError exception.
  // Checks for overflow as well.
  auto lenAfterInsert = len + insertCount;
  if (LLVM_UNLIKELY(
          lenAfterInsert < len ||
          lenAfterInsert - actualDeleteCount > (1LLU << 53) - 1)) {
    return runtime.raiseTypeError("Array.prototype.splice result out of space");
  }

  // Let A be ? ArraySpeciesCreate(O, actualDeleteCount).
  if (LLVM_UNLIKELY(actualDeleteCount > JSArray::StorageType::maxElements())) {
    return runtime.raiseRangeError("Out of memory for array elements.");
  }
  auto arrRes = JSArray::create(runtime, actualDeleteCount, actualDeleteCount);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*arrRes));

  // Indices used for various copies in loops below.
  MutableHandle<> from{runtime};
  MutableHandle<> to{runtime};

  // Value storage used for copying values.
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<JSObject> fromDescObjHandle{runtime};
  MutableHandle<> fromValue{runtime};

  MutableHandle<> i{runtime};
  MutableHandle<> k{runtime};

  auto gcMarker = gcScope.createMarker();

  {
    // Copy actualDeleteCount elements to A, starting at actualStart.
    // TODO: Add a fast path for actual arrays.
    for (uint32_t j = 0; j < actualDeleteCount; ++j) {
      from = HermesValue::encodeTrustedNumberValue(actualStart + j);

      ComputedPropertyDescriptor fromDesc;
      JSObject::getComputedPrimitiveDescriptor(
          O, runtime, from, fromDescObjHandle, tmpPropNameStorage, fromDesc);
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputedPropertyValue_RJS(
              O,
              runtime,
              fromDescObjHandle,
              tmpPropNameStorage,
              fromDesc,
              from);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
        fromValue = std::move(*propRes);
        JSArray::setElementAt(A, runtime, j, fromValue);
      }

      gcScope.flushToMarker(gcMarker);
    }

    if (LLVM_UNLIKELY(
            JSArray::setLengthProperty(A, runtime, actualDeleteCount) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
  }

  // Perform ? Set(A, "length", actualDeleteCount, true).
  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              A,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(
                  HermesValue::encodeTrustedNumberValue(actualDeleteCount)),
              PropOpFlags().plusThrowOnError()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Number of new items to add to the array.
  uint32_t itemCount = args.getArgCount() > 2 ? args.getArgCount() - 2 : 0;

  if (itemCount < actualDeleteCount) {
    // Inserting less items than deleting.

    // Copy items from (k + actualDeleteCount) to (k + itemCount).
    // This leaves itemCount spaces to copy the arguments into.
    // TODO: Add a fast path for actual arrays.
    for (double j = actualStart; j < len - actualDeleteCount; ++j) {
      from = HermesValue::encodeTrustedNumberValue(j + actualDeleteCount);
      to = HermesValue::encodeTrustedNumberValue(j + itemCount);
      ComputedPropertyDescriptor fromDesc;
      JSObject::getComputedPrimitiveDescriptor(
          O, runtime, from, fromDescObjHandle, tmpPropNameStorage, fromDesc);
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputedPropertyValue_RJS(
              O,
              runtime,
              fromDescObjHandle,
              tmpPropNameStorage,
              fromDesc,
              from);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
        fromValue = std::move(*propRes);
        if (LLVM_UNLIKELY(
                JSObject::putComputed_RJS(
                    O,
                    runtime,
                    to,
                    fromValue,
                    PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      } else {
        if (LLVM_UNLIKELY(
                JSObject::deleteComputed(
                    O, runtime, to, PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }

      gcScope.flushToMarker(gcMarker);
    }

    // Use i here to refer to (k-1) in the spec, and reindex the loop.
    i = HermesValue::encodeTrustedNumberValue(len - 1);

    // Delete the remaining elements from the right that we didn't copy into.
    // TODO: Add a fast path for actual arrays.
    while (i->getNumber() > len - actualDeleteCount + itemCount - 1) {
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  O, runtime, i, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      i = HermesValue::encodeTrustedNumberValue(i->getDouble() - 1);
      gcScope.flushToMarker(gcMarker);
    }
  } else if (itemCount > actualDeleteCount) {
    // Inserting more items than deleting.

    // Start from the right, and copy elements to the right.
    // This makes space to insert the elements from the arguments.
    // TODO: Add a fast path for actual arrays.
    for (double j = len - actualDeleteCount; j > actualStart; --j) {
      from = HermesValue::encodeTrustedNumberValue(j + actualDeleteCount - 1);
      to = HermesValue::encodeTrustedNumberValue(j + itemCount - 1);

      ComputedPropertyDescriptor fromDesc;
      JSObject::getComputedPrimitiveDescriptor(
          O, runtime, from, fromDescObjHandle, tmpPropNameStorage, fromDesc);
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputedPropertyValue_RJS(
              O,
              runtime,
              fromDescObjHandle,
              tmpPropNameStorage,
              fromDesc,
              from);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
        fromValue = std::move(*propRes);
        if (LLVM_UNLIKELY(
                JSObject::putComputed_RJS(
                    O,
                    runtime,
                    to,
                    fromValue,
                    PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      } else {
        // fromPresent is false
        if (LLVM_UNLIKELY(
                JSObject::deleteComputed(
                    O, runtime, to, PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }

      gcScope.flushToMarker(gcMarker);
    }
  }

  {
    // Finally, just copy the elements from the args into the array.
    // TODO: Add a fast path for actual arrays.
    k = HermesValue::encodeTrustedNumberValue(actualStart);
    for (size_t j = 2; j < argCount; ++j) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  k,
                  args.getArgHandle(j),
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      k = HermesValue::encodeTrustedNumberValue(k->getDouble() + 1);
      gcScope.flushToMarker(gcMarker);
    }
  }

  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              O,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(HermesValue::encodeTrustedNumberValue(
                  len - actualDeleteCount + itemCount)),
              PropOpFlags().plusThrowOnError()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeCopyWithin(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ToObject(this value).
  // 2. ReturnIfAbrupt(O).
  auto oRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(oRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*oRes);

  // 3. Let len be ToLength(Get(O, "length")).
  // 4. ReturnIfAbrupt(len).
  // Use doubles for all lengths and indices to allow for proper Infinity
  // handling, because ToInteger may return Infinity and we must do double
  // arithmetic.
  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *lenRes;

  // 5. Let relativeTarget be ToIntegerOrInfinity(target).
  // 6. ReturnIfAbrupt(relativeTarget).
  auto relativeTargetRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(relativeTargetRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double relativeTarget = relativeTargetRes->getNumber();

  // 7. If relativeTarget < 0, let to be max((len + relativeTarget),0); else let
  // to be min(relativeTarget, len).
  double to = relativeTarget < 0 ? std::max((len + relativeTarget), (double)0)
                                 : std::min(relativeTarget, len);

  // 8. Let relativeStart be ToIntegerOrInfinity(start).
  // 9. ReturnIfAbrupt(relativeStart).
  auto relativeStartRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(relativeStartRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double relativeStart = relativeStartRes->getNumber();

  // 10. If relativeStart < 0, let from be max((len + relativeStart),0); else
  // let from be min(relativeStart, len).
  double from = relativeStart < 0 ? std::max((len + relativeStart), (double)0)
                                  : std::min(relativeStart, len);

  // 11. If end is undefined, let relativeEnd be len; else let relativeEnd be
  // ToIntegerOrInfinity(end).
  // 12. ReturnIfAbrupt(relativeEnd).
  double relativeEnd;
  if (args.getArg(2).isUndefined()) {
    relativeEnd = len;
  } else {
    auto relativeEndRes = toIntegerOrInfinity(runtime, args.getArgHandle(2));
    if (LLVM_UNLIKELY(relativeEndRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    relativeEnd = relativeEndRes->getNumber();
  }

  // 13. If relativeEnd < 0, let final be max((len + relativeEnd),0); else let
  // final be min(relativeEnd, len).
  double fin = relativeEnd < 0 ? std::max((len + relativeEnd), (double)0)
                               : std::min(relativeEnd, len);

  // 14. Let count be min(final-from, len-to).
  double count = std::min(fin - from, len - to);

  int direction;
  if (from < to && to < from + count) {
    // 15. If from<to and to<from+count
    // a. Let direction be -1.
    direction = -1;
    // b. Let from be from + count -1.
    from = from + count - 1;
    // c. Let to be to + count -1.
    to = to + count - 1;
  } else {
    // 16. Else,
    // a. Let direction = 1.
    direction = 1;
  }

  MutableHandle<> fromHandle{
      runtime, HermesValue::encodeTrustedNumberValue(from)};
  MutableHandle<> toHandle{runtime, HermesValue::encodeTrustedNumberValue(to)};

  MutableHandle<SymbolID> fromNameTmpStorage{runtime};
  MutableHandle<JSObject> fromObj{runtime};
  MutableHandle<> fromVal{runtime};

  GCScopeMarkerRAII marker{gcScope};
  for (; count > 0; marker.flush()) {
    // 17. Repeat, while count > 0
    // a. Let fromKey be ToString(from).
    // b. Let toKey be ToString(to).

    // c. Let fromPresent be HasProperty(O, fromKey).
    // d. ReturnIfAbrupt(fromPresent).
    ComputedPropertyDescriptor fromDesc;
    if (LLVM_UNLIKELY(
            JSObject::getComputedDescriptor(
                O,
                runtime,
                fromHandle,
                fromObj,
                fromNameTmpStorage,
                fromDesc) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    CallResult<PseudoHandle<>> fromValRes =
        JSObject::getComputedPropertyValue_RJS(
            O, runtime, fromObj, fromNameTmpStorage, fromDesc, fromHandle);
    if (LLVM_UNLIKELY(fromValRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // e. If fromPresent is true, then
    if (LLVM_LIKELY(!(*fromValRes)->isEmpty())) {
      // i. Let fromVal be Get(O, fromKey).
      // ii. ReturnIfAbrupt(fromVal).
      fromVal = std::move(*fromValRes);

      // iii. Let setStatus be Set(O, toKey, fromVal, true).
      // iv. ReturnIfAbrupt(setStatus).
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  toHandle,
                  fromVal,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      // f. Else fromPresent is false,
      // i. Let deleteStatus be DeletePropertyOrThrow(O, toKey).
      // ii. ReturnIfAbrupt(deleteStatus).
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  O, runtime, toHandle, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    // g. Let from be from + direction.
    fromHandle = HermesValue::encodeTrustedNumberValue(
        fromHandle->getNumber() + direction);
    // h. Let to be to + direction.
    toHandle = HermesValue::encodeTrustedNumberValue(
        toHandle->getNumber() + direction);

    // i. Let count be count − 1.
    --count;
  }
  // 18. Return O.
  return O.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypePop(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(res.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  if (len == 0) {
    if (LLVM_UNLIKELY(
            JSObject::putNamed_RJS(
                O,
                runtime,
                Predefined::getSymbolID(Predefined::length),
                runtime.makeHandle(HermesValue::encodeTrustedNumberValue(0)),
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    return HermesValue::encodeUndefinedValue();
  }

  auto idxVal =
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(len - 1));
  if (LLVM_UNLIKELY(
          (propRes = JSObject::getComputed_RJS(O, runtime, idxVal)) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto element = runtime.makeHandle(std::move(*propRes));
  if (LLVM_UNLIKELY(
          JSObject::deleteComputed(
              O, runtime, idxVal, PropOpFlags().plusThrowOnError()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              O,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(
                  HermesValue::encodeTrustedNumberValue(len - 1)),
              PropOpFlags().plusThrowOnError()) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return element.get();
}

CallResult<HermesValue>
arrayPrototypeShift(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  if (len == 0) {
    // Need to set length to 0 per spec.
    if (JSObject::putNamed_RJS(
            O,
            runtime,
            Predefined::getSymbolID(Predefined::length),
            runtime.makeHandle(HermesValue::encodeTrustedNumberValue(0)),
            PropOpFlags().plusThrowOnError()) == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return HermesValue::encodeUndefinedValue();
  }

  auto idxVal = runtime.makeHandle(HermesValue::encodeTrustedNumberValue(0));
  if (LLVM_UNLIKELY(
          (propRes = JSObject::getComputed_RJS(O, runtime, idxVal)) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto first = runtime.makeHandle(std::move(*propRes));

  MutableHandle<> from{runtime, HermesValue::encodeTrustedNumberValue(1)};
  MutableHandle<> to{runtime};

  MutableHandle<SymbolID> fromNameTmpStorage{runtime};
  MutableHandle<JSObject> fromDescObjHandle{runtime};
  MutableHandle<> fromVal{runtime};

  // Move every element to the left one slot.
  // TODO: Add a fast path for actual arrays.
  while (from->getDouble() < len) {
    GCScopeMarkerRAII marker{gcScope};

    // Moving an element from "from" to "from - 1".
    to = HermesValue::encodeTrustedNumberValue(from->getDouble() - 1);

    ComputedPropertyDescriptor fromDesc;
    JSObject::getComputedPrimitiveDescriptor(
        O, runtime, from, fromDescObjHandle, fromNameTmpStorage, fromDesc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        O, runtime, fromDescObjHandle, fromNameTmpStorage, fromDesc, from);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      // fromPresent is true, so read fromVal and set the "to" index.
      fromVal = std::move(*propRes);
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O, runtime, to, fromVal, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      // fromVal is not present so move the empty slot to the left.
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  O, runtime, to, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    from = HermesValue::encodeTrustedNumberValue(from->getDouble() + 1);
  }

  // Delete last element of the array.
  if (LLVM_UNLIKELY(
          JSObject::deleteComputed(
              O,
              runtime,
              runtime.makeHandle(
                  HermesValue::encodeTrustedNumberValue(len - 1)),
              PropOpFlags().plusThrowOnError()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Decrement length.
  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              O,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(
                  HermesValue::encodeTrustedNumberValue(len - 1)),
              PropOpFlags().plusThrowOnError()) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return first.get();
}

/// Used to help with indexOf and lastIndexOf.
/// \p reverse true if searching in reverse (lastIndexOf), false otherwise.
static inline CallResult<HermesValue>
indexOfHelper(Runtime &runtime, NativeArgs args, const bool reverse) {
  GCScope gcScope(runtime);
  struct : Locals {
    PinnedValue<JSObject> obj;
    PinnedValue<> length;
    PinnedValue<SymbolID> tmpPropNameStorage;
    PinnedValue<JSObject> descObj;
    PinnedValue<> k;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.obj = vmcast<JSObject>(*objRes);

  // Array length is less than 2^32, length of array-like object is less than
  // 2^53.
  uint64_t len;
  auto arrHandle = vmisa<JSArray>(*lv.obj) ? Handle<JSArray>::vmcast(&lv.obj)
                                           : Runtime::makeNullHandle<JSArray>();
  if (LLVM_LIKELY(arrHandle)) {
    // Fast path: get array length.
    len = JSArray::getLength(arrHandle.get(), runtime);
  } else {
    auto propRes = JSObject::getNamed_RJS(
        lv.obj, runtime, Predefined::getSymbolID(Predefined::length));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.length = std::move(*propRes);
    auto lenRes = toLengthU64(runtime, lv.length);
    if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // toLengthU64() returns an unsigned integer smaller than 2^53, so it fits
    // into int64_t.
    len = *lenRes;
  }

  // Early return before running into any coercions on args.
  // 2. Let len be ? LengthOfArrayLike(O).
  // 3. If len is 0, return -1.
  if (len == 0) {
    return HermesValue::encodeTrustedNumberValue(-1);
  }

  // Actual index to start the search at.
  uint64_t k;
  if (args.getArgCount() > 1) {
    auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    double n = intRes->getNumber();
    if (!reverse) {
      if (LLVM_UNLIKELY(n >= len)) {
        // If n >= len, nothing to search. This also handles n = +Inf.
        return HermesValue::encodeTrustedNumberValue(-1);
      } else if (n < 0) {
        // 9. k = len + n. If k < 0, set k = 0. This handles n == -Inf.
        k = std::max(len + n, 0.0);
      } else {
        // Here 0 <= n < len.
        k = n;
      }
    } else {
      if (LLVM_UNLIKELY(n + len < 0)) {
        // Return -1 since nothing to search. This handles n = -Inf.
        return HermesValue::encodeTrustedNumberValue(-1);
      } else if (n >= 0) {
        // If n is larger than len - 1, set k = len - 1. This handles n = +Inf.
        k = std::min(n, (double)(len - 1));
      } else {
        // Here 0 <= n + len < len.
        k = len + n;
      }
    }
  } else {
    k = !reverse ? 0 : len - 1;
  }

  // Search for the element.
  auto searchElement = args.getArgHandle(0);
  // If the range of Array IndexedStorage is not [0, len), there could be holes
  // at some indices.
  if (LLVM_LIKELY(
          arrHandle && (0 == arrHandle->getBeginIndex()) &&
          (len == arrHandle->getEndIndex()))) {
    // Fast path: access array storage directly.
    auto searchElementVal =
        SmallHermesValue::encodeHermesValue(searchElement.get(), runtime);
    NoAllocScope noAlloc{runtime};
    bool hasHole = false;
    auto *arrStorage = arrHandle->getIndexedStorage(runtime);

    // Macro for searching the array with given loop initialization, terminating
    // condition and updating rule after each iteration. Note that the macro
    // COMPARE_EXPR, which takes the array element of each iteration and
    // produces a boolean value, must be defined befofe exeucting this.
#define SEARCH_ARRAY_DIRECTED(INIT, LOOP_COND, STEP)   \
  for (INIT; LOOP_COND; STEP) {                        \
    auto element = arrStorage->at(runtime, k);         \
    if (LLVM_UNLIKELY(element.isEmpty())) {            \
      hasHole = true;                                  \
      break;                                           \
    }                                                  \
    if (COMPARE_EXPR(element))                         \
      return HermesValue::encodeTrustedNumberValue(k); \
  }

    // Specialize the loop with search direction.
#define SEARCH_ARRAY                             \
  if (reverse) {                                 \
    SEARCH_ARRAY_DIRECTED(++k, k-- > 0, (void)0) \
  } else {                                       \
    SEARCH_ARRAY_DIRECTED((void)0, k < len, ++k) \
  }

    // Specialize the search loop with the type of the target value.
    if (searchElementVal.isInlinedDouble()) {
      auto searchNum = searchElementVal.getNumber(runtime);
      // If it's NaN, no need to do any comparison.
      if (LLVM_UNLIKELY(std::isnan(searchNum))) {
        return HermesValue::encodeTrustedNumberValue(-1);
      }
      // If it's +0.0/-0.0.
      if (searchNum == 0) {
        auto negativeZero = SmallHermesValue::encodeNumberValue(-0.0, runtime);
        auto negativeZeroBits = negativeZero.getRaw();
        auto positiveZero = SmallHermesValue::encodeNumberValue(+0.0, runtime);
        auto positiveZeroBits = positiveZero.getRaw();
        assert(
            negativeZero.isInlinedDouble() && positiveZero.isInlinedDouble() &&
            "Both +0.0/-0.0 should be inline double.");
        // Compare bits with +0.0/-0.0 directly.
#define COMPARE_EXPR(element) \
  element.getRaw() == negativeZeroBits || element.getRaw() == positiveZeroBits
        SEARCH_ARRAY
#undef COMPARE_EXPR
      } else {
        // If it's not +0.0/-0.0/NaN, compare raw bits directly.
#define COMPARE_EXPR(element) searchElementVal.getRaw() == element.getRaw()
        SEARCH_ARRAY
#undef COMPARE_EXPR
      }
    } else if (searchElementVal.isBoxedDouble()) {
      // Only HV32 can have boxed doubles, compare the double value.
      auto searchNum = searchElementVal.getBoxedDouble(runtime);
#define COMPARE_EXPR(element) \
  element.isBoxedDouble() && searchNum == element.getBoxedDouble(runtime)
      SEARCH_ARRAY
#undef COMPARE_EXPR
    } else if (searchElementVal.isString()) {
      auto searchStr = searchElementVal.getString(runtime);

#define COMPARE_EXPR(element) \
  element.isString() && searchStr->equals(element.getString(runtime))
      SEARCH_ARRAY
#undef COMPARE_EXPR
    } else if (searchElementVal.isBigInt()) {
      auto searchBigInt = searchElementVal.getBigInt(runtime);

#define COMPARE_EXPR(element) \
  element.isBigInt() && !searchBigInt->compare(element.getBigInt(runtime))
      SEARCH_ARRAY
#undef COMPARE_EXPR
    } else {
      // For all other types (e.g., Object), compare the exact bits.
#define COMPARE_EXPR(element) searchElementVal.getRaw() == element.getRaw()
      SEARCH_ARRAY
#undef COMPARE_EXPR
    }

#undef SEARCH_ARRAY_DIRECTED
#undef SEARCH_ARRAY

    // If array has no hole and target is not found, return -1.
    if (!hasHole) {
      return HermesValue::encodeTrustedNumberValue(-1);
    }
  }

  // Slow path for non-array objects or arrays with holes.
  MutableHandle<SymbolID> tmpPropNameStorage = lv.tmpPropNameStorage;
  MutableHandle<JSObject> descObjHandle = lv.descObj;
  lv.k = HermesValue::encodeTrustedNumberValue(k);
  auto marker = gcScope.createMarker();
  while (true) {
    gcScope.flushToMarker(marker);
    // Check that we're not done yet.
    if (!reverse) {
      if (lv.k->getDouble() >= len) {
        break;
      }
    } else {
      if (lv.k->getDouble() < 0) {
        break;
      }
    }
    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        lv.obj, runtime, lv.k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        lv.obj, runtime, descObjHandle, tmpPropNameStorage, desc, lv.k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!(*propRes)->isEmpty() &&
        strictEqualityTest(searchElement.get(), propRes->get())) {
      return lv.k.get();
    }
    // Update the index based on the direction of the search.
    lv.k = HermesValue::encodeTrustedNumberValue(
        lv.k->getDouble() + (reverse ? -1 : 1));
  }

  // Not found, return -1.
  return HermesValue::encodeTrustedNumberValue(-1);
}

CallResult<HermesValue>
arrayPrototypeUnshift(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;
  size_t argCount = args.getArgCount();

  // 4. If argCount > 0, then
  if (argCount > 0) {
    // If len+ argCount > (2 ^ 53) -1, throw a TypeError exception.
    if (LLVM_UNLIKELY(len + argCount >= ((uint64_t)1 << 53) - 1)) {
      return runtime.raiseTypeError(
          "Array.prototype.unshift result out of space");
    }

    // Loop indices.
    MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(len)};
    MutableHandle<> j{runtime, HermesValue::encodeTrustedNumberValue(0)};

    // Indices to copy from/to when shifting.
    MutableHandle<> from{runtime};
    MutableHandle<> to{runtime};

    // Value that is being copied.
    MutableHandle<SymbolID> fromNameTmpStorage{runtime};
    MutableHandle<JSObject> fromDescObjHandle{runtime};
    MutableHandle<> fromValue{runtime};

    // Move elements to the right by argCount to account for the new elements.
    // TODO: Add a fast path for actual arrays.
    auto marker = gcScope.createMarker();
    while (k->getDouble() > 0) {
      gcScope.flushToMarker(marker);
      from = HermesValue::encodeTrustedNumberValue(k->getDouble() - 1);
      to = HermesValue::encodeTrustedNumberValue(k->getDouble() + argCount - 1);

      ComputedPropertyDescriptor fromDesc;
      JSObject::getComputedPrimitiveDescriptor(
          O, runtime, from, fromDescObjHandle, fromNameTmpStorage, fromDesc);
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputedPropertyValue_RJS(
              O,
              runtime,
              fromDescObjHandle,
              fromNameTmpStorage,
              fromDesc,
              from);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
        fromValue = std::move(*propRes);
        if (LLVM_UNLIKELY(
                JSObject::putComputed_RJS(
                    O,
                    runtime,
                    to,
                    fromValue,
                    PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      } else {
        // Shift the empty slot by deleting at the destination.
        if (LLVM_UNLIKELY(
                JSObject::deleteComputed(
                    O, runtime, to, PropOpFlags().plusThrowOnError()) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      k = HermesValue::encodeTrustedNumberValue(k->getDouble() - 1);
    }

    // Put the arguments into the beginning of the array.
    for (auto arg : args.handles()) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O, runtime, j, arg, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      gcScope.flushToMarker(marker);
      j = HermesValue::encodeTrustedNumberValue(j->getDouble() + 1);
    }
  }

  // Increment length by argCount.
  auto newLen = HermesValue::encodeTrustedNumberValue(len + argCount);
  if (LLVM_UNLIKELY(
          JSObject::putNamed_RJS(
              O,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(newLen),
              PropOpFlags().plusThrowOnError()) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return newLen;
}

CallResult<HermesValue>
arrayPrototypeIndexOf(void *, Runtime &runtime, NativeArgs args) {
  return indexOfHelper(runtime, args, false);
}

CallResult<HermesValue>
arrayPrototypeLastIndexOf(void *, Runtime &runtime, NativeArgs args) {
  return indexOfHelper(runtime, args, true);
}

/// Helper function for every/some.
/// \param every true if calling every(), false if calling some().
static inline CallResult<HermesValue>
everySomeHelper(Runtime &runtime, NativeArgs args, const bool every) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  auto callbackFn = args.dyncastArg<Callable>(0);
  if (!callbackFn) {
    return runtime.raiseTypeError(
        "Array.prototype.every() requires a callable argument");
  }

  // Index to check the callback on.
  MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(0)};

  // Value at index k;
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<JSObject> descObjHandle{runtime};
  MutableHandle<> kValue{runtime};

  // Loop through and run the callback.
  auto marker = gcScope.createMarker();
  while (k->getDouble() < len) {
    gcScope.flushToMarker(marker);

    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        O, runtime, k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        O, runtime, descObjHandle, tmpPropNameStorage, desc, k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      // kPresent is true, call the callback on the kth element.
      kValue = std::move(*propRes);
      auto callRes = Callable::executeCall3(
          callbackFn,
          runtime,
          args.getArgHandle(1),
          kValue.get(),
          k.get(),
          O.getHermesValue());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto testResult = std::move(*callRes);
      if (every) {
        // Done if one is false.
        if (!toBoolean(testResult.get())) {
          return HermesValue::encodeBoolValue(false);
        }
      } else {
        // Done if one is true.
        if (toBoolean(testResult.get())) {
          return HermesValue::encodeBoolValue(true);
        }
      }
    }

    k = HermesValue::encodeTrustedNumberValue(k->getDouble() + 1);
  }

  // If we're looking for every, then we finished without returning true.
  // If we're looking for some, then we finished without returning false.
  return HermesValue::encodeBoolValue(every);
}

CallResult<HermesValue>
arrayPrototypeEvery(void *, Runtime &runtime, NativeArgs args) {
  return everySomeHelper(runtime, args, true);
}

CallResult<HermesValue>
arrayPrototypeSome(void *, Runtime &runtime, NativeArgs args) {
  return everySomeHelper(runtime, args, false);
}

CallResult<HermesValue>
arrayPrototypeMap(void *, Runtime &runtime, NativeArgs args) {
  struct : Locals {
    PinnedValue<JSObject> O;
    PinnedValue<> length;
    PinnedValue<JSArray> A;
    PinnedValue<> k;
    PinnedValue<JSObject> descObj;
    PinnedValue<SymbolID> tmpPropNameStorage;
    PinnedValue<> value;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.O = vmcast<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      lv.O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.length = std::move(*propRes);
  auto intRes = toLengthU64(runtime, lv.length);
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  auto callbackFn = args.dyncastArg<Callable>(0);
  if (!callbackFn) {
    return runtime.raiseTypeError(
        "Array.prototype.map() requires a callable argument");
  }

  // Resultant array.
  if (LLVM_UNLIKELY(len > JSArray::StorageType::maxElements())) {
    return runtime.raiseRangeError("Out of memory for array elements.");
  }
  auto arrRes = JSArray::create(runtime, len, len);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.A = std::move(*arrRes);

  // Current index to execute callback on.
  lv.k = HermesValue::encodeTrustedNumberValue(0);

  MutableHandle<SymbolID> tmpPropNameStorage{lv.tmpPropNameStorage};
  MutableHandle<JSObject> descObjHandle{lv.descObj};

  // Main loop to execute callback and store the results in A.
  // TODO: Implement a fast path for actual arrays.
  auto marker = gcScope.createMarker();
  while (lv.k->getDouble() < len) {
    gcScope.flushToMarker(marker);

    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        lv.O, runtime, lv.k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        lv.O, runtime, descObjHandle, tmpPropNameStorage, desc, lv.k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      // kPresent is true, execute callback and store result in A[k].
      auto kValue = std::move(*propRes);
      auto callRes = Callable::executeCall3(
          callbackFn,
          runtime,
          args.getArgHandle(1),
          kValue.get(),
          lv.k.get(),
          lv.O.getHermesValue());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lv.value = std::move(*callRes);
      JSArray::setElementAt(lv.A, runtime, lv.k->getDouble(), lv.value);
    }

    lv.k = HermesValue::encodeTrustedNumberValue(lv.k->getDouble() + 1);
  }

  return lv.A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeFilter(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *intRes;

  auto callbackFn = args.dyncastArg<Callable>(0);
  if (!callbackFn) {
    return runtime.raiseTypeError(
        "Array.prototype.filter() requires a callable argument");
  }

  if (LLVM_UNLIKELY(len > JSArray::StorageType::maxElements())) {
    return runtime.raiseRangeError("Out of memory for array elements.");
  }
  auto arrRes = JSArray::create(runtime, len, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*arrRes));

  // Index in the original array.
  MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(0)};
  // Index to copy to in the new array.
  uint32_t to = 0;

  // Value at index k.
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<JSObject> descObjHandle{runtime};
  MutableHandle<> kValue{runtime};

  auto marker = gcScope.createMarker();
  while (k->getDouble() < len) {
    gcScope.flushToMarker(marker);

    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        O, runtime, k, descObjHandle, tmpPropNameStorage, desc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        O, runtime, descObjHandle, tmpPropNameStorage, desc, k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      kValue = std::move(*propRes);
      // Call the callback.
      auto callRes = Callable::executeCall3(
          callbackFn,
          runtime,
          args.getArgHandle(1),
          kValue.get(),
          k.get(),
          O.getHermesValue());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (toBoolean(callRes->get())) {
        // Add the element to the array if it passes the callback.
        JSArray::setElementAt(A, runtime, to, kValue);
        ++to;
      }
    }

    k = HermesValue::encodeTrustedNumberValue(k->getDouble() + 1);
  }

  if (LLVM_UNLIKELY(
          JSArray::setLengthProperty(A, runtime, to) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return A.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeFill(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());
  // Get the length.
  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *lenRes;
  // Get the value to be filled.
  MutableHandle<> value(runtime, args.getArg(0));
  // Get the relative start and end.
  auto intRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double relativeStart = intRes->getNumber();
  // Index to start the deletion/insertion at.
  double actualStart = relativeStart < 0 ? std::max(len + relativeStart, 0.0)
                                         : std::min(relativeStart, len);
  double relativeEnd;
  if (args.getArg(2).isUndefined()) {
    relativeEnd = len;
  } else {
    if (LLVM_UNLIKELY(
            (intRes = toIntegerOrInfinity(runtime, args.getArgHandle(2))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    relativeEnd = intRes->getNumber();
  }
  // Actual end index.
  double actualEnd = relativeEnd < 0 ? std::max(len + relativeEnd, 0.0)
                                     : std::min(relativeEnd, len);
  MutableHandle<> k(
      runtime, HermesValue::encodeTrustedNumberValue(actualStart));
  auto marker = gcScope.createMarker();
  while (k->getDouble() < actualEnd) {
    if (LLVM_UNLIKELY(
            JSObject::putComputed_RJS(
                O, runtime, k, value, PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    k.set(HermesValue::encodeTrustedNumberValue(k->getDouble() + 1));
    gcScope.flushToMarker(marker);
  }
  return O.getHermesValue();
}

static CallResult<HermesValue>
findHelper(void *ctx, bool reverse, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  bool findIndex = ctx != nullptr;
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  // Get the length.
  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *intRes;

  auto predicate = args.dyncastArg<Callable>(0);
  if (!predicate) {
    return runtime.raiseTypeError("Find argument must be a function");
  }

  // "this" argument to the callback function.
  auto T = args.getArgHandle(1);
  MutableHandle<> kHandle{runtime};
  MutableHandle<> kValue{runtime};
  auto marker = gcScope.createMarker();
  for (size_t i = 0; i < len; ++i) {
    kHandle =
        HermesValue::encodeTrustedNumberValue(reverse ? (len - i - 1) : i);
    gcScope.flushToMarker(marker);
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getComputed_RJS(O, runtime, kHandle)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    kValue = std::move(*propRes);
    auto callRes = Callable::executeCall3(
        predicate,
        runtime,
        T,
        kValue.getHermesValue(),
        kHandle.getHermesValue(),
        O.getHermesValue());
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    bool testResult = toBoolean(callRes->get());
    if (testResult) {
      // If this is index find variant, then return the index k.
      // Else, return the value at the index k.
      return findIndex ? kHandle.getHermesValue() : kValue.getHermesValue();
    }
  }

  // Failure case for Array.prototype.findIndex is -1.
  // Failure case for Array.prototype.find is undefined.
  // The last variants share the same failure case values.
  return findIndex ? HermesValue::encodeTrustedNumberValue(-1)
                   : HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
arrayPrototypeFind(void *ctx, Runtime &runtime, NativeArgs args) {
  return findHelper(ctx, false, runtime, args);
}

CallResult<HermesValue>
arrayPrototypeFindLast(void *ctx, Runtime &runtime, NativeArgs args) {
  return findHelper(ctx, true, runtime, args);
}

/// Helper for reduce and reduceRight.
/// \param reverse set to true to reduceRight, false to reduce from the left.
static inline CallResult<HermesValue>
reduceHelper(Runtime &runtime, NativeArgs args, const bool reverse) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *intRes;

  size_t argCount = args.getArgCount();

  auto callbackFn = args.dyncastArg<Callable>(0);
  if (!callbackFn) {
    return runtime.raiseTypeError(
        "Array.prototype.reduce() requires a callable argument");
  }

  // Can't reduce an empty array without an initial value.
  if (len == 0 && argCount < 2) {
    return runtime.raiseTypeError(
        "Array.prototype.reduce() requires an initial value with empty array");
  }

  // Current index in the reduction iteration.
  MutableHandle<> k{
      runtime, HermesValue::encodeTrustedNumberValue(reverse ? len - 1 : 0)};
  MutableHandle<SymbolID> kNameTmpStorage{runtime};
  MutableHandle<JSObject> kDescObjHandle{runtime};

  MutableHandle<> accumulator{runtime};

  auto marker = gcScope.createMarker();

  // How much to increment k by each iteration of a loop.
  double increment = reverse ? -1 : 1;

  // Initialize the accumulator to either the intialValue arg or the first value
  // of the array.
  if (argCount >= 2) {
    accumulator = args.getArg(1);
  } else {
    bool kPresent = false;
    while (!kPresent) {
      gcScope.flushToMarker(marker);
      if (!reverse) {
        if (k->getDouble() >= len) {
          break;
        }
      } else {
        if (k->getDouble() < 0) {
          break;
        }
      }
      ComputedPropertyDescriptor kDesc;
      JSObject::getComputedPrimitiveDescriptor(
          O, runtime, k, kDescObjHandle, kNameTmpStorage, kDesc);
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputedPropertyValue_RJS(
              O, runtime, kDescObjHandle, kNameTmpStorage, kDesc, k);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
        kPresent = true;
        accumulator = std::move(*propRes);
      }
      k = HermesValue::encodeTrustedNumberValue(k->getDouble() + increment);
    }
    if (!kPresent) {
      return runtime.raiseTypeError(
          "Array.prototype.reduce() requires an intial value with empty array");
    }
  }

  // Perform the reduce.
  while (true) {
    gcScope.flushToMarker(marker);
    if (!reverse) {
      if (k->getDouble() >= len) {
        break;
      }
    } else {
      if (k->getDouble() < 0) {
        break;
      }
    }

    ComputedPropertyDescriptor kDesc;
    JSObject::getComputedPrimitiveDescriptor(
        O, runtime, k, kDescObjHandle, kNameTmpStorage, kDesc);
    CallResult<PseudoHandle<>> propRes = JSObject::getComputedPropertyValue_RJS(
        O, runtime, kDescObjHandle, kNameTmpStorage, kDesc, k);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_LIKELY(!(*propRes)->isEmpty())) {
      // kPresent is true, run the accumulation step.
      auto kValue = std::move(*propRes);
      auto callRes = Callable::executeCall4(
          callbackFn,
          runtime,
          Runtime::getUndefinedValue(),
          accumulator.get(),
          kValue.get(),
          k.get(),
          O.getHermesValue());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      accumulator = std::move(*callRes);
    }
    k = HermesValue::encodeTrustedNumberValue(k->getDouble() + increment);
  }

  return accumulator.get();
}

CallResult<HermesValue>
arrayPrototypeReduce(void *, Runtime &runtime, NativeArgs args) {
  return reduceHelper(runtime, args, false);
}

CallResult<HermesValue>
arrayPrototypeReduceRight(void *, Runtime &runtime, NativeArgs args) {
  return reduceHelper(runtime, args, true);
}

/// ES10.0 22.1.3.23.
CallResult<HermesValue>
arrayPrototypeReverse(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(objRes.getValue());

  MutableHandle<> lower{runtime, HermesValue::encodeTrustedNumberValue(0)};
  MutableHandle<> upper{runtime};

  // The values at the lower and upper indices.
  MutableHandle<> lowerValue{runtime};
  MutableHandle<> upperValue{runtime};

  auto marker = gcScope.createMarker();

  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes = toLengthU64(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = *lenRes;

  // Indices used in the reversal process.
  uint64_t middle = len / 2;

  while (lower->getDouble() != middle) {
    gcScope.flushToMarker(marker);
    upper = HermesValue::encodeTrustedNumberValue(len - lower->getNumber() - 1);

    CallResult<bool> lowerExistsRes = JSObject::hasComputed(O, runtime, lower);
    if (LLVM_UNLIKELY(lowerExistsRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (*lowerExistsRes) {
      CallResult<PseudoHandle<>> lowerValueRes =
          JSObject::getComputed_RJS(O, runtime, lower);
      if (LLVM_UNLIKELY(lowerValueRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      lowerValue = std::move(*lowerValueRes);
      gcScope.flushToMarker(marker);
    }

    CallResult<bool> upperExistsRes = JSObject::hasComputed(O, runtime, upper);
    if (LLVM_UNLIKELY(upperExistsRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (*upperExistsRes) {
      CallResult<PseudoHandle<>> upperValueRes =
          JSObject::getComputed_RJS(O, runtime, upper);
      if (LLVM_UNLIKELY(upperValueRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      upperValue = std::move(*upperValueRes);
      gcScope.flushToMarker(marker);
    }

    // Handle cases in which lower/upper do/don't exist.
    if (*lowerExistsRes && *upperExistsRes) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  lower,
                  upperValue,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  upper,
                  lowerValue,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else if (*upperExistsRes) {
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  lower,
                  upperValue,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  O, runtime, upper, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    } else if (*lowerExistsRes) {
      if (LLVM_UNLIKELY(
              JSObject::deleteComputed(
                  O, runtime, lower, PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (LLVM_UNLIKELY(
              JSObject::putComputed_RJS(
                  O,
                  runtime,
                  upper,
                  lowerValue,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    lower = HermesValue::encodeTrustedNumberValue(lower->getDouble() + 1);
  }

  return O.getHermesValue();
}

CallResult<HermesValue>
arrayPrototypeIncludes(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? ToObject(this value).
  auto oRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(oRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*oRes);

  // 2. Let len be ? ToLength(? Get(O, "length")).
  auto lenPropRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(lenPropRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lenRes =
      toLengthU64(runtime, runtime.makeHandle(std::move(*lenPropRes)));
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = *lenRes;

  // 3. If len is 0, return false.
  if (len == 0) {
    return HermesValue::encodeBoolValue(false);
  }

  // 4. Let n be ? ToIntegerOrInfinity(fromIndex).
  // (If fromIndex is undefined, this step produces the value 0.)
  auto nRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(nRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // Use double here, because ToInteger may return Infinity.
  double n = nRes->getNumber();

  double k;
  if (n >= 0) {
    // 5. If n ≥ 0, then
    // 5a. Let k be n.
    k = n;
  } else {
    // 6. Else n < 0,
    // 6a. Let k be len + n.
    k = len + n;
    // 6b. If k < 0, let k be 0.
    if (k < 0) {
      k = 0;
    }
  }

  MutableHandle<> kHandle{runtime};

  // 7. Repeat, while k < len
  auto marker = gcScope.createMarker();
  while (k < len) {
    gcScope.flushToMarker(marker);

    // 7a. Let elementK be the result of ? Get(O, ! ToString(k)).
    kHandle = HermesValue::encodeTrustedNumberValue(k);
    auto elementKRes = JSObject::getComputed_RJS(O, runtime, kHandle);
    if (LLVM_UNLIKELY(elementKRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // 7b. If SameValueZero(searchElement, elementK) is true, return true.
    if (isSameValueZero(args.getArg(0), elementKRes->get())) {
      return HermesValue::encodeBoolValue(true);
    }

    // 7c. Increase k by 1.
    ++k;
  }

  // 8. Return false.
  return HermesValue::encodeBoolValue(false);
}

/// ES14.0 23.1.3.33
CallResult<HermesValue>
arrayPrototypeToReversed(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? ToObject(this value).
  auto oRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(oRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*oRes);

  // 2. Let len be ? LengthOfArrayLike(O).
  Handle<JSArray> jsArr = Handle<JSArray>::dyn_vmcast(O);
  auto lenRes = lengthOfArrayLike(runtime, O, jsArr);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = lenRes.getValue();

  // 3. Let A be ArrayCreate(len).
  uint32_t len32 = truncateToUInt32(len);
  if (LLVM_UNLIKELY(len32 != len)) {
    return runtime.raiseRangeError("invalid array length");
  }
  auto ARes = JSArray::create(runtime, len32, len32);
  if (LLVM_UNLIKELY(ARes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*ARes));

  // 4. Let k be 0.
  double k = 0;

  MutableHandle<> kHandle{runtime};
  MutableHandle<> fromHandle{runtime};
  MutableHandle<> fromValueHandle{runtime};

  auto marker = gcScope.createMarker();
  // 5. Repeat, while k < len,
  while (k < len) {
    gcScope.flushToMarker(marker);

    double from = len - k - 1;
    // 5a. Let from be ! ToString(𝔽(len - k - 1)).
    fromHandle = HermesValue::encodeUntrustedNumberValue(from);

    // 5b. Let Pk be ! ToString(𝔽(k)).
    kHandle = HermesValue::encodeTrustedNumberValue(k);

    // 5c. Let fromValue be ? Get(O, from).
    if (LLVM_LIKELY(jsArr)) {
      const SmallHermesValue elm = jsArr->at(runtime, from);
      // If the element is not empty, we can return it directly here.
      // Otherwise, we must proceed to the slow path.
      if (!elm.isEmpty()) {
        fromValueHandle = elm.unboxToHV(runtime);
      }
    }
    // Slow path
    else {
      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputed_RJS(O, runtime, fromHandle);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      fromValueHandle = propRes->getHermesValue();
    }

    // 5d. Perform ! CreateDataPropertyOrThrow(A, Pk, fromValue).
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                A,
                runtime,
                kHandle,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                fromValueHandle,
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // 5e. Set k to k + 1.
    ++k;
  }

  return A.getHermesValue();
}

/// Copies \p count elements from \p from (or \p fromArr if is simple array)
static inline CallResult<double> arrayCopyHelper(
    Runtime &runtime,
    GCScope &gcScope,
    Handle<JSObject> from,
    Handle<JSArray> fromArr,
    uint32_t fromStartIndex,
    Handle<JSArray> to,
    uint32_t toStartIndex,
    uint32_t count) {
  MutableHandle<> fromIndexHandle{runtime};
  MutableHandle<> toIndexHandle{runtime};
  MutableHandle<> fromValueHandle{runtime};

  auto marker = gcScope.createMarker();
  double i = 0;

  while (i < count) {
    gcScope.flushToMarker(marker);
    auto fromIndex = fromStartIndex + i;

    if (LLVM_LIKELY(fromArr)) {
      const SmallHermesValue elem = fromArr->at(runtime, fromIndex);
      if (!elem.isEmpty()) {
        fromValueHandle = elem.unboxToHV(runtime);
      }
    }
    // Slow path
    else {
      fromIndexHandle = HermesValue::encodeTrustedNumberValue(fromIndex);

      CallResult<PseudoHandle<>> propRes =
          JSObject::getComputed_RJS(from, runtime, fromIndexHandle);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      fromValueHandle = propRes->getHermesValue();
    }

    toIndexHandle = HermesValue::encodeTrustedNumberValue(toStartIndex + i);

    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                to,
                runtime,
                toIndexHandle,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                fromValueHandle,
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    ++i;
  }

  return i;
}

/// ES14.0 23.1.3.35
CallResult<HermesValue>
arrayPrototypeToSpliced(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? ToObject(this value).
  auto oRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(oRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*oRes);

  // 2. Let len be ? LengthOfArrayLike(O).
  Handle<JSArray> jsArr = Handle<JSArray>::dyn_vmcast(O);
  auto lenRes = lengthOfArrayLike(runtime, O, jsArr);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double len = lenRes.getValue();

  // 3. Let relativeIndex be ? ToIntegerOrInfinity(index).
  auto relativeStartRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(relativeStartRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // Use double here, because ToInteger may return Infinity.
  double relativeStart = relativeStartRes->getNumber();

  // 4. If relativeStart is -∞, let actualStart be 0.
  // 5. Else if relativeStart < 0, let actualStart be max(len + relativeStart,
  // 0).
  // 6. Else, let actualStart be min(relativeStart, len).
  double actualStart = relativeStart < 0 ? std::max(len + relativeStart, 0.0)
                                         : std::min(relativeStart, len);

  uint32_t argCount = args.getArgCount();
  uint64_t actualSkipCount;
  uint64_t insertCount;

  switch (argCount) {
    // 8. If start is not present, then
    case 0:
      insertCount = 0;
      // 8a. Let actualSkipCount be 0.
      actualSkipCount = 0;
      break;
    // 9. Else if skipCount is not present, then
    case 1:
      insertCount = 0;
      // 9a. Let actualSkipCount be len - actualStart.
      actualSkipCount = len - actualStart;
      break;
    // 10. Else
    default: {
      // 10a. Let sc be ? ToIntegerOrInfinity(skipCount).
      auto skipCountRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
      if (LLVM_UNLIKELY(skipCountRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      insertCount = argCount - 2;

      // 10b. Let actualSkipCount be the result of clamping dc between 0 and len
      // - actualStart.
      actualSkipCount =
          std::min(std::max(skipCountRes->getNumber(), 0.0), len - actualStart);
    }
  }

  // 11. Let newLen be len + insertCount - actualSkipCount.
  auto lenAfterInsert = len + insertCount - actualSkipCount;

  // 12. If newLen > 253 - 1, throw a TypeError exception.
  if (LLVM_UNLIKELY(
          // lenAfterInsert < len ||
          lenAfterInsert - actualSkipCount > (1LLU << 53) - 1)) {
    return runtime.raiseTypeError(
        "Array.prototype.toSpliced result out of space");
  }

  // 13. Let A be ArrayCreate(len).
  uint32_t lenAfterInsert32 = truncateToUInt32(lenAfterInsert);
  if (LLVM_UNLIKELY(lenAfterInsert32 != lenAfterInsert)) {
    return runtime.raiseRangeError("invalid array length");
  }
  auto ARes = JSArray::create(runtime, lenAfterInsert32, lenAfterInsert32);
  if (LLVM_UNLIKELY(ARes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*ARes));

  // 14. Let i be 0
  double i = 0;
  MutableHandle<> insertIndexHandle{runtime};

  // 15. Let r be actualStart + actualSkipCount.
  uint64_t r = actualStart + actualSkipCount;

  uint64_t paramIndex = 2;

  Handle<JSArray> fromArr = Handle<JSArray>::dyn_vmcast(O);

  // 16a - 16d
  // Copy elements from original array O from beginning until actualStart into
  // new array A
  auto copyRes =
      arrayCopyHelper(runtime, gcScope, O, fromArr, 0, A, 0, actualStart);
  if (LLVM_UNLIKELY(copyRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  i += copyRes.getValue();

  // 17. For each element E of items, do
  while (paramIndex < argCount) {
    // 17a. Let Pi be ! ToString(𝔽(i)).
    insertIndexHandle = HermesValue::encodeTrustedNumberValue(i);

    // 17b. Perform ! CreateDataPropertyOrThrow(A, Pi, E).
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                A,
                runtime,
                insertIndexHandle,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                args.getArgHandle(paramIndex),
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // 17c. Set i to i + 1.
    ++i;
    ++paramIndex;
  }

  // 18a - 18f
  // Copy remaining elements from original array O including skipCount into new
  // array A
  copyRes = arrayCopyHelper(
      runtime, gcScope, O, fromArr, r, A, i, lenAfterInsert - i);
  if (LLVM_UNLIKELY(copyRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  i += copyRes.getValue();

  return A.getHermesValue();
}

/// ES14.0 23.1.3.39
CallResult<HermesValue>
arrayPrototypeWith(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be ? ToObject(this value).
  auto oRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(oRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime.makeHandle<JSObject>(*oRes);

  // 2. Let len be ? LengthOfArrayLike(O).
  Handle<JSArray> jsArr = Handle<JSArray>::dyn_vmcast(O);
  auto lenRes = lengthOfArrayLike(runtime, O, jsArr);
  if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto len = lenRes.getValue();

  // 3. Let relativeIndex be ? ToIntegerOrInfinity(index).
  auto relativeIndexRes = toIntegerOrInfinity(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(relativeIndexRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // Use double here, because ToInteger may return Infinity.
  double relativeIndex = relativeIndexRes->getNumber();

  double actualIndex = 0;
  // 4. If relativeIndex ≥ 0, let actualIndex be relativeIndex.
  if (relativeIndex >= 0) {
    actualIndex = relativeIndex;
  }
  // 5. Else, let actualIndex be len + relativeIndex.
  else {
    actualIndex = len + relativeIndex;
  }

  // 6. If actualIndex ≥ len or actualIndex < 0, throw a RangeError exception.
  if (actualIndex < 0 || actualIndex >= len) {
    return runtime.raiseRangeError("invalid or out-of-range index");
  }

  // 7. Let A be ArrayCreate(len).
  uint32_t len32 = truncateToUInt32(len);
  if (LLVM_UNLIKELY(len32 != len)) {
    return runtime.raiseRangeError("invalid array length");
  }
  auto ARes = JSArray::create(runtime, len32, len32);
  if (LLVM_UNLIKELY(ARes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> A = runtime.makeHandle(std::move(*ARes));

  // 8. Let k be 0.
  double k = 0;

  MutableHandle<> kHandle{runtime};
  MutableHandle<> fromValueHandle{runtime};

  auto marker = gcScope.createMarker();
  // 9. Repeat, while k < len,
  while (k < len) {
    gcScope.flushToMarker(marker);

    // 9a. Let Pk be the result of ? Get(O, ! ToString(k)).
    kHandle = HermesValue::encodeUntrustedNumberValue(k);
    auto PkRes = JSObject::getComputed_RJS(O, runtime, kHandle);
    if (LLVM_UNLIKELY(PkRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // 9b. If k is actualIndex, let fromValue be value.
    if (k == actualIndex) {
      fromValueHandle = args.getArgHandle(1);
    }
    // 9c. Else, let fromValue be ? Get(O, Pk).
    else {
      if (LLVM_LIKELY(jsArr)) {
        const SmallHermesValue elm = jsArr->at(runtime, k);
        // If the element is not empty, we can return it directly here.
        // Otherwise, we must proceed to the slow path.
        if (!elm.isEmpty()) {
          fromValueHandle = elm.unboxToHV(runtime);
        }
      }
      // Slow path
      else {
        CallResult<PseudoHandle<>> propRes =
            JSObject::getComputed_RJS(O, runtime, kHandle);
        if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        fromValueHandle = propRes->getHermesValue();
      }
    }

    // 9d. Perform ! CreateDataPropertyOrThrow(A, Pk, fromValue).
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                A,
                runtime,
                kHandle,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                fromValueHandle,
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // 9e. Set k to k + 1.
    ++k;
  }

  return A.getHermesValue();
}

CallResult<HermesValue> arrayOf(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let len be the actual number of arguments passed to this function.
  uint32_t len = args.getArgCount();
  // 2. Let items be the List of arguments passed to this function.
  // 3. Let C be the this value.
  auto C = args.getThisHandle();

  MutableHandle<JSObject> A{runtime};
  CallResult<bool> isConstructorRes = isConstructor(runtime, *C);
  if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 4. If IsConstructor(C) is true, then
  if (*isConstructorRes) {
    // a. Let A be Construct(C, «len»).
    auto aRes = Callable::executeConstruct1(
        Handle<Callable>::vmcast(C),
        runtime,
        runtime.makeHandle(HermesValue::encodeTrustedNumberValue(len)));
    if (LLVM_UNLIKELY(aRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    A = PseudoHandle<JSObject>::vmcast(std::move(*aRes));
  } else {
    // 5. Else,
    // a. Let A be ArrayCreate(len).
    auto aRes = JSArray::create(runtime, len, len);
    if (LLVM_UNLIKELY(aRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    A = std::move(*aRes);
  }
  // 7. Let k be 0.
  MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(0)};
  MutableHandle<> kValue{runtime};

  GCScopeMarkerRAII marker{gcScope};
  // 8. Repeat, while k < len
  for (; k->getNumberAs<uint32_t>() < len; marker.flush()) {
    // a. Let kValue be items[k].
    kValue = args.getArg(k->getNumber());

    // c. Let defineStatus be CreateDataPropertyOrThrow(A,Pk, kValue).
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                A,
                runtime,
                k,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                kValue,
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // e. Increase k by 1.
    k = HermesValue::encodeTrustedNumberValue(k->getNumber() + 1);
  }

  // 9. Let setStatus be Set(A, "length", len, true).
  // 10. ReturnIfAbrupt(setStatus).
  auto setStatus = JSObject::putNamed_RJS(
      A,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(len)),
      PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(setStatus == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 11. Return A.
  return A.getHermesValue();
}

/// ES6.0 22.1.2.1 Array.from ( items [ , mapfn [ , thisArg ] ] )
CallResult<HermesValue> arrayFrom(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  auto itemsHandle = args.getArgHandle(0);
  // 1. Let C be the this value.
  auto C = args.getThisHandle();
  // 2. If mapfn is undefined, let mapping be false.
  // 3. else
  MutableHandle<Callable> mapfn{runtime};
  MutableHandle<> T{runtime, HermesValue::encodeUndefinedValue()};
  if (!args.getArg(1).isUndefined()) {
    mapfn = dyn_vmcast<Callable>(args.getArg(1));
    // a. If IsCallable(mapfn) is false, throw a TypeError exception.
    if (LLVM_UNLIKELY(!mapfn)) {
      return runtime.raiseTypeError("Mapping function is not callable.");
    }
    // b. If thisArg was supplied, let T be thisArg; else let T be undefined.
    if (args.getArgCount() >= 3) {
      T = args.getArg(2);
    }
    // c. Let mapping be true
  }
  // 4. Let usingIterator be GetMethod(items, @@iterator).
  // 5. ReturnIfAbrupt(usingIterator).
  auto methodRes = getMethod(
      runtime,
      itemsHandle,
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolIterator)));
  if (LLVM_UNLIKELY(methodRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto usingIterator = runtime.makeHandle(methodRes->getHermesValue());

  MutableHandle<JSObject> A{runtime};
  // 6. If usingIterator is not undefined, then
  if (!usingIterator->isUndefined()) {
    CallResult<bool> isConstructorRes = isConstructor(runtime, *C);
    if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // a. If IsConstructor(C) is true, then
    if (*isConstructorRes) {
      GCScopeMarkerRAII markerConstruct{gcScope};
      // i. Let A be Construct(C).
      auto callRes =
          Callable::executeConstruct0(Handle<Callable>::vmcast(C), runtime);
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      A = PseudoHandle<JSObject>::vmcast(std::move(*callRes));
    } else {
      // b. Else,
      //  i. Let A be ArrayCreate(0).
      auto arrRes = JSArray::create(runtime, 0, 0);
      if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      A = std::move(*arrRes);
    }
    // c. ReturnIfAbrupt(A).
    // d. Let iterator be GetIterator(items, usingIterator).
    // Assert we can cast usingIterator to a Callable otherwise getMethod would
    // have thrown.
    // e. ReturnIfAbrupt(iterator).
    auto iterRes = getCheckedIterator(
        runtime, args.getArgHandle(0), Handle<Callable>::vmcast(usingIterator));
    if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto iteratorRecord = *iterRes;
    // f. Let k be 0.
    MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(0)};
    // g. Repeat
    MutableHandle<> mappedValue{runtime};
    MutableHandle<> nextValue{runtime};
    while (true) {
      GCScopeMarkerRAII marker1{runtime};
      // ii. Let next be IteratorStep(iteratorRecord).
      // iii. ReturnIfAbrupt(next).
      auto next = iteratorStep(runtime, iteratorRecord);
      if (LLVM_UNLIKELY(next == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // iv. If next is false, then
      if (!next.getValue()) {
        // 1. Let setStatus be Set(A, "length", k, true).
        // 2. ReturnIfAbrupt(setStatus).
        // 3. Return A.
        auto setStatus = JSObject::putNamed_RJS(
            A,
            runtime,
            Predefined::getSymbolID(Predefined::length),
            k,
            PropOpFlags().plusThrowOnError());
        if (LLVM_UNLIKELY(setStatus == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        return A.getHermesValue();
      }
      // v. Let nextValue be IteratorValue(next).
      // vi. ReturnIfAbrupt(nextValue).
      auto propRes = JSObject::getNamed_RJS(
          *next, runtime, Predefined::getSymbolID(Predefined::value));
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      nextValue = std::move(*propRes);
      // vii. If mapping is true, then
      if (mapfn) {
        // 1. Let mappedValue be Call(mapfn, T, «nextValue, k»).
        auto callRes = Callable::executeCall2(
            mapfn, runtime, T, nextValue.getHermesValue(), k.getHermesValue());
        // 2. If mappedValue is an abrupt completion, return
        // IteratorClose(iterator, mappedValue).
        if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
          return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
        }
        // 3. Let mappedValue be mappedValue.[[value]].
        mappedValue = std::move(*callRes);
      } else {
        // viii. Else, let mappedValue be nextValue.
        mappedValue = nextValue.getHermesValue();
      }
      // ix. Let defineStatus be CreateDataPropertyOrThrow(A, Pk, mappedValue).
      // x. If defineStatus is an abrupt completion, return
      // IteratorClose(iterator, defineStatus).
      if (LLVM_UNLIKELY(
              JSObject::defineOwnComputedPrimitive(
                  A,
                  runtime,
                  k,
                  DefinePropertyFlags::getDefaultNewPropertyFlags(),
                  mappedValue,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
      }
      // xi. Increase k by 1.
      k = HermesValue::encodeTrustedNumberValue(k->getNumber() + 1);
    }
  }
  // 7. Assert: items is not an Iterable so assume it is an array-like object.
  // 8. Let arrayLike be ToObject(items).
  auto objRes = toObject(runtime, itemsHandle);
  // 9. ReturnIfAbrupt(arrayLike).
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arrayLike = runtime.makeHandle<JSObject>(objRes.getValue());
  // 10. Let len be ToLength(Get(arrayLike, "length")).
  // 11. ReturnIfAbrupt(len).
  auto propRes = JSObject::getNamed_RJS(
      arrayLike, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto lengthRes = toLength(runtime, runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t len = lengthRes->getNumberAs<uint64_t>();
  CallResult<bool> isConstructorRes = isConstructor(runtime, *C);
  if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 12. If IsConstructor(C) is true, then
  if (*isConstructorRes) {
    // a. Let A be Construct(C, «len»).
    auto callRes = Callable::executeConstruct1(
        Handle<Callable>::vmcast(C),
        runtime,
        runtime.makeHandle(lengthRes.getValue()));
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    A = PseudoHandle<JSObject>::vmcast(std::move(*callRes));
  } else {
    // 13. Else,
    //  a. Let A be ArrayCreate(len).
    if (LLVM_UNLIKELY(len > JSArray::StorageType::maxElements())) {
      return runtime.raiseRangeError("Out of memory for array elements.");
    }
    auto arrRes = JSArray::create(runtime, len, len);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    A = std::move(*arrRes);
  }
  // 14. ReturnIfAbrupt(A).
  // 15. Let k be 0.
  MutableHandle<> k{runtime, HermesValue::encodeTrustedNumberValue(0)};
  // 16. Repeat, while k < len
  MutableHandle<> mappedValue{runtime};
  while (k->getNumberAs<uint32_t>() < len) {
    GCScopeMarkerRAII marker2{runtime};
    // b. Let kValue be Get(arrayLike, Pk).
    propRes = JSObject::getComputed_RJS(arrayLike, runtime, k);
    // c. ReturnIfAbrupt(kValue).
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // d. If mapping is true, then
    if (mapfn) {
      // i. Let mappedValue be Call(mapfn, T, «kValue, k»).
      // ii. ReturnIfAbrupt(mappedValue).
      auto callRes = Callable::executeCall2(
          mapfn, runtime, T, propRes->get(), k.getHermesValue());
      if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      mappedValue = std::move(*callRes);
    } else {
      // e. Else, let mappedValue be kValue.
      mappedValue = std::move(*propRes);
    }
    // f. Let defineStatus be CreateDataPropertyOrThrow(A, Pk, mappedValue).
    // g. ReturnIfAbrupt(defineStatus).
    if (LLVM_UNLIKELY(
            JSObject::defineOwnComputedPrimitive(
                A,
                runtime,
                k,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                mappedValue,
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // h. Increase k by 1.
    k = HermesValue::encodeTrustedNumberValue(k->getNumber() + 1);
  }
  // 17. Let setStatus be Set(A, "length", len, true).
  auto setStatus = JSObject::putNamed_RJS(
      A,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      k,
      PropOpFlags().plusThrowOnError());
  // 18. ReturnIfAbrupt(setStatus).
  if (LLVM_UNLIKELY(setStatus == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 19. Return A.
  return A.getHermesValue();
}

} // namespace vm
} // namespace hermes
