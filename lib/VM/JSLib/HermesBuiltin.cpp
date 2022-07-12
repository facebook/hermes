/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/Support/Base64vlq.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringView.h"

#include <random>

namespace hermes {
namespace vm {

/// Set the parent of an object failing silently on any error.
CallResult<HermesValue>
silentObjectSetPrototypeOf(void *, Runtime &runtime, NativeArgs args) {
  JSObject *O = dyn_vmcast<JSObject>(args.getArg(0));
  if (!O)
    return HermesValue::encodeUndefinedValue();

  JSObject *parent;
  HermesValue V = args.getArg(1);
  if (V.isNull())
    parent = nullptr;
  else if (V.isObject())
    parent = vmcast<JSObject>(V);
  else
    return HermesValue::encodeUndefinedValue();

  (void)JSObject::setParent(O, runtime, parent);

  // Ignore exceptions.
  runtime.clearThrownValue();

  return HermesValue::encodeUndefinedValue();
}

/// ES6.0 12.2.9.3 Runtime Semantics: GetTemplateObject ( templateLiteral )
/// Given a template literal, return a template object that looks like this:
/// [cookedString0, cookedString1, ..., raw: [rawString0, rawString1]].
/// This object is frozen, as well as the 'raw' object nested inside.
/// We only pass the parts from the template literal that are needed to
/// construct this object. That is, the raw strings and cooked strings.
/// Arguments: \p templateObjID is the unique id associated with the template
/// object. \p dup is a boolean, when it is true, cooked strings are the same as
/// raw strings. Then raw strings are passed. Finally cooked strings are
/// optionally passed if \p dup is true.
CallResult<HermesValue>
hermesBuiltinGetTemplateObject(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(args.getArgCount() < 3)) {
    return runtime.raiseTypeError("At least three arguments expected");
  }
  if (LLVM_UNLIKELY(!args.getArg(0).isNumber())) {
    return runtime.raiseTypeError("First argument should be a number");
  }
  if (LLVM_UNLIKELY(!args.getArg(1).isBool())) {
    return runtime.raiseTypeError("Second argument should be a bool");
  }

  GCScope gcScope{runtime};

  // Try finding the template object in the template object cache.
  uint32_t templateObjID = args.getArg(0).getNumberAs<uint32_t>();
  auto savedCB = runtime.getStackFrames().begin()->getSavedCodeBlock();
  if (LLVM_UNLIKELY(!savedCB)) {
    return runtime.raiseTypeError("Cannot be called from native code");
  }
  RuntimeModule *runtimeModule = savedCB->getRuntimeModule();
  JSObject *cachedTemplateObj =
      runtimeModule->findCachedTemplateObject(templateObjID);
  if (cachedTemplateObj) {
    return HermesValue::encodeObjectValue(cachedTemplateObj);
  }

  bool dup = args.getArg(1).getBool();
  if (LLVM_UNLIKELY(!dup && args.getArgCount() % 2 == 1)) {
    return runtime.raiseTypeError(
        "There must be the same number of raw and cooked strings.");
  }
  uint32_t count = dup ? args.getArgCount() - 2 : args.getArgCount() / 2 - 1;

  // Create template object and raw object.
  auto arrRes = JSArray::create(runtime, count, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto rawObj = Handle<JSObject>::vmcast(*arrRes);
  auto arrRes2 = JSArray::create(runtime, count, 0);
  if (LLVM_UNLIKELY(arrRes2 == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto templateObj = Handle<JSObject>::vmcast(*arrRes2);

  // Set cooked and raw strings as elements in template object and raw object,
  // respectively.
  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.configurable = 0;
  MutableHandle<> idx{runtime};
  MutableHandle<> rawValue{runtime};
  MutableHandle<> cookedValue{runtime};
  uint32_t cookedBegin = dup ? 2 : 2 + count;
  auto marker = gcScope.createMarker();
  for (uint32_t i = 0; i < count; ++i) {
    idx = HermesValue::encodeNumberValue(i);

    cookedValue = args.getArg(cookedBegin + i);
    auto putRes = JSObject::defineOwnComputedPrimitive(
        templateObj, runtime, idx, dpf, cookedValue);
    assert(
        putRes != ExecutionStatus::EXCEPTION && *putRes &&
        "Failed to set cooked value to template object.");

    rawValue = args.getArg(2 + i);
    putRes = JSObject::defineOwnComputedPrimitive(
        rawObj, runtime, idx, dpf, rawValue);
    assert(
        putRes != ExecutionStatus::EXCEPTION && *putRes &&
        "Failed to set raw value to raw object.");

    gcScope.flushToMarker(marker);
  }
  // Make 'length' property on the raw object read-only.
  DefinePropertyFlags readOnlyDPF{};
  readOnlyDPF.setWritable = 1;
  readOnlyDPF.setConfigurable = 1;
  readOnlyDPF.writable = 0;
  readOnlyDPF.configurable = 0;
  auto readOnlyRes = JSObject::defineOwnProperty(
      rawObj,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      readOnlyDPF,
      Runtime::getUndefinedValue(),
      PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(readOnlyRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!*readOnlyRes)) {
    return runtime.raiseTypeError(
        "Failed to set 'length' property on the raw object read-only.");
  }
  JSObject::preventExtensions(rawObj.get());

  // Set raw object as a read-only non-enumerable property of the template
  // object.
  PropertyFlags constantPF{};
  constantPF.writable = 0;
  constantPF.configurable = 0;
  constantPF.enumerable = 0;
  auto putNewRes = JSObject::defineNewOwnProperty(
      templateObj,
      runtime,
      Predefined::getSymbolID(Predefined::raw),
      constantPF,
      rawObj);
  if (LLVM_UNLIKELY(putNewRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // Make 'length' property on the template object read-only.
  readOnlyRes = JSObject::defineOwnProperty(
      templateObj,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      readOnlyDPF,
      Runtime::getUndefinedValue(),
      PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(readOnlyRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!*readOnlyRes)) {
    return runtime.raiseTypeError(
        "Failed to set 'length' property on the raw object read-only.");
  }
  JSObject::preventExtensions(templateObj.get());

  // Cache the template object.
  runtimeModule->cacheTemplateObject(templateObjID, templateObj);

  return templateObj.getHermesValue();
}

/// If the first argument is not an object, throw a type error with the second
/// argument as a message.
///
/// \code
///   HermesBuiltin.ensureObject = function(value, errorMessage) {...}
/// \endcode
CallResult<HermesValue>
hermesBuiltinEnsureObject(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_LIKELY(args.getArg(0).isObject()))
    return HermesValue::encodeUndefinedValue();

  return runtime.raiseTypeError(args.getArgHandle(1));
}

/// Perform the GetMethod() abstract operation.
///
/// \code
///   HermesBuiltin.getMethod = function(object, property) {...}
/// \endcode
CallResult<HermesValue>
hermesBuiltinGetMethod(void *, Runtime &runtime, NativeArgs args) {
  return getMethod(runtime, args.getArgHandle(0), args.getArgHandle(1))
      .toCallResultHermesValue();
}

/// Throw a type error with the argument as a message.
///
/// \code
///   HermesBuiltin.throwTypeError = function(errorMessage) {...}
/// \endcode
CallResult<HermesValue>
hermesBuiltinThrowTypeError(void *, Runtime &runtime, NativeArgs args) {
  return runtime.raiseTypeError(args.getArgHandle(0));
}

/// Set the isDelegated flag on the GeneratorInnerFunction which calls
/// this function.
/// \pre the caller must be an interpreted GeneratorInnerFunction
/// \return `undefined`
CallResult<HermesValue>
hermesBuiltinGeneratorSetDelegated(void *, Runtime &runtime, NativeArgs args) {
  auto *gen = dyn_vmcast<GeneratorInnerFunction>(
      runtime.getCurrentFrame().getPreviousFrame().getCalleeClosureOrCBRef());
  if (!gen) {
    return runtime.raiseTypeError(
        "generatorSetDelegated can only be called as part of yield*");
  }
  gen->setIsDelegated(true);
  return HermesValue::encodeUndefinedValue();
}

namespace {

CallResult<HermesValue> copyDataPropertiesSlowPath_RJS(
    Runtime &runtime,
    Handle<JSObject> target,
    Handle<JSObject> from,
    Handle<JSObject> excludedItems) {
  assert(
      from->isProxyObject() &&
      "copyDataPropertiesSlowPath_RJS is only for Proxy");

  // 5. Let keys be ? from.[[OwnPropertyKeys]]().
  auto cr = JSProxy::getOwnPropertyKeys(
      from,
      runtime,
      OwnKeysFlags()
          .plusIncludeSymbols()
          .plusIncludeNonSymbols()
          .plusIncludeNonEnumerable());
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto keys = *cr;

  MutableHandle<> nextKeyHandle{runtime};
  MutableHandle<> propValueHandle{runtime};
  MutableHandle<SymbolID> tmpSymbolStorage{runtime};
  GCScopeMarkerRAII marker{runtime};
  // 6. For each element nextKey of keys in List order, do
  for (uint32_t nextKeyIdx = 0, endIdx = keys->getEndIndex();
       nextKeyIdx < endIdx;
       ++nextKeyIdx) {
    marker.flush();
    nextKeyHandle = keys->at(runtime, nextKeyIdx).unboxToHV(runtime);
    if (nextKeyHandle->isNumber()) {
      CallResult<PseudoHandle<StringPrimitive>> strRes =
          toString_RJS(runtime, nextKeyHandle);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      nextKeyHandle = strRes->getHermesValue();
    }

    // b. For each element e of excludedItems in List order, do
    //   i. If SameValue(e, nextKey) is true, then
    //     1. Set excluded to true.
    if (excludedItems) {
      assert(
          !excludedItems->isProxyObject() &&
          "internal excludedItems object is a proxy");
      ComputedPropertyDescriptor desc;
      CallResult<bool> cr = JSObject::getOwnComputedPrimitiveDescriptor(
          excludedItems,
          runtime,
          nextKeyHandle,
          JSObject::IgnoreProxy::Yes,
          tmpSymbolStorage,
          desc);
      if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      if (*cr)
        continue;
    }

    //   i. Let desc be ? from.[[GetOwnProperty]](nextKey).
    ComputedPropertyDescriptor desc;
    CallResult<bool> crb =
        JSProxy::getOwnProperty(from, runtime, nextKeyHandle, desc, nullptr);
    if (LLVM_UNLIKELY(crb == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    //   ii. If desc is not undefined and desc.[[Enumerable]] is true, then
    if (*crb && desc.flags.enumerable) {
      //     1. Let propValue be ? Get(from, nextKey).
      CallResult<PseudoHandle<>> crv =
          JSProxy::getComputed(from, runtime, nextKeyHandle, from);
      if (LLVM_UNLIKELY(crv == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      propValueHandle = std::move(*crv);
      //     2. Perform ! CreateDataProperty(target, nextKey, propValue).
      crb = JSObject::defineOwnComputed(
          target,
          runtime,
          nextKeyHandle,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          propValueHandle);
      if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      assert(
          crb != ExecutionStatus::EXCEPTION && *crb &&
          "CreateDataProperty failed");
    }
  }
  return target.getHermesValue();
}

} // namespace

/// \code
///   HermesBuiltin.copyDataProperties =
///         function (target, source, excludedItems) {}
/// \endcode
///
/// Copy all enumerable own properties of object \p source, that are not also
/// properties of \p excludedItems, into \p target, which must be an object, and
/// return \p target. If \p excludedItems is not specified, it is assumed
/// to be empty.
CallResult<HermesValue>
hermesBuiltinCopyDataProperties(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Assert: Type(target) is Object.
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  // To be safe, ignore non-objects.
  if (!target)
    return HermesValue::encodeUndefinedValue();

  // 3. If source is undefined or null, return target.
  Handle<> untypedSource = args.getArgHandle(1);
  if (untypedSource->isNull() || untypedSource->isUndefined())
    return target.getHermesValue();

  // 4. Let from be ! ToObject(source).
  Handle<JSObject> source = untypedSource->isObject()
      ? Handle<JSObject>::vmcast(untypedSource)
      : Handle<JSObject>::vmcast(
            runtime.makeHandle(*toObject(runtime, untypedSource)));

  // 2. Assert: excludedItems is a List of property keys.
  // In Hermes, excludedItems is represented as a JSObject, created by
  // bytecode emitted by the compiler, whose keys are the excluded
  // propertyKeys
  Handle<JSObject> excludedItems = args.dyncastArg<JSObject>(2);
  assert(
      (!excludedItems || !excludedItems->isProxyObject()) &&
      "excludedItems internal List is a Proxy");

  if (source->isProxyObject()) {
    return copyDataPropertiesSlowPath_RJS(
        runtime, target, source, excludedItems);
  }

  MutableHandle<> nameHandle{runtime};
  MutableHandle<> valueHandle{runtime};
  MutableHandle<SymbolID> tmpSymbolStorage{runtime};

  // Process all named properties/symbols.
  bool success = JSObject::forEachOwnPropertyWhile(
      source,
      runtime,
      // indexedCB.
      [&source,
       &target,
       &excludedItems,
       &nameHandle,
       &valueHandle,
       &tmpSymbolStorage](
          Runtime &runtime, uint32_t index, ComputedPropertyDescriptor desc) {
        if (!desc.flags.enumerable)
          return true;

        nameHandle = HermesValue::encodeNumberValue(index);

        if (excludedItems) {
          assert(
              !excludedItems->isProxyObject() &&
              "internal excludedItems object is a proxy");
          ComputedPropertyDescriptor xdesc;
          auto cr = JSObject::getOwnComputedPrimitiveDescriptor(
              excludedItems,
              runtime,
              nameHandle,
              JSObject::IgnoreProxy::Yes,
              tmpSymbolStorage,
              xdesc);
          if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
            return false;
          if (*cr)
            return true;
        }

        valueHandle = JSObject::getOwnIndexed(
            createPseudoHandle(source.get()), runtime, index);

        if (LLVM_UNLIKELY(
                JSObject::defineOwnComputedPrimitive(
                    target,
                    runtime,
                    nameHandle,
                    DefinePropertyFlags::getDefaultNewPropertyFlags(),
                    valueHandle) == ExecutionStatus::EXCEPTION)) {
          return false;
        }

        return true;
      },
      // namedCB.
      [&source, &target, &excludedItems, &valueHandle](
          Runtime &runtime, SymbolID sym, NamedPropertyDescriptor desc) {
        if (!desc.flags.enumerable)
          return true;
        if (InternalProperty::isInternal(sym))
          return true;

        // Skip excluded items.
        if (excludedItems) {
          auto cr = JSObject::hasNamedOrIndexed(excludedItems, runtime, sym);
          assert(
              cr != ExecutionStatus::EXCEPTION &&
              "hasNamedOrIndex failed, which can only happen with a proxy, "
              "but excludedItems should never be a proxy");
          if (*cr)
            return true;
        }

        auto cr =
            JSObject::getNamedPropertyValue_RJS(source, runtime, source, desc);
        if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
          return false;

        valueHandle = std::move(*cr);

        // sym can be an index-like property, so we have to bypass the assert in
        // defineOwnPropertyInternal.
        if (LLVM_UNLIKELY(
                JSObject::defineOwnPropertyInternal(
                    target,
                    runtime,
                    sym,
                    DefinePropertyFlags::getDefaultNewPropertyFlags(),
                    valueHandle) == ExecutionStatus::EXCEPTION)) {
          return false;
        }

        return true;
      });

  if (LLVM_UNLIKELY(!success))
    return ExecutionStatus::EXCEPTION;

  return target.getHermesValue();
}

/// \code
///   HermesBuiltin.copyRestArgs = function (from) {}
/// \endcode
/// Copy the callers parameters starting from index \c from (where the first
/// parameter is index 0) into a JSArray.
CallResult<HermesValue>
hermesBuiltinCopyRestArgs(void *, Runtime &runtime, NativeArgs args) {
  GCScopeMarkerRAII marker{runtime};

  // Obtain the caller's stack frame.
  auto frames = runtime.getStackFrames();
  auto it = frames.begin();
  ++it;
  // Check for the extremely unlikely case where there is no caller frame.
  if (LLVM_UNLIKELY(it == frames.end()))
    return HermesValue::encodeUndefinedValue();

  // "from" should be a number.
  if (!args.getArg(0).isNumber())
    return HermesValue::encodeUndefinedValue();
  uint32_t from = truncateToUInt32(args.getArg(0).getNumber());

  uint32_t argCount = it->getArgCount();
  uint32_t length = from <= argCount ? argCount - from : 0;

  auto cr = JSArray::create(runtime, length, length);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  auto array = *cr;
  JSArray::setStorageEndIndex(array, runtime, length);

  for (uint32_t i = 0; i != length; ++i) {
    const auto shv =
        SmallHermesValue::encodeHermesValue(it->getArgRef(from), runtime);
    JSArray::unsafeSetExistingElementAt(*array, runtime, i, shv);
    ++from;
  }

  return array.getHermesValue();
}

/// \code
///   HermesBuiltin.arraySpread = function(target, source, nextIndex) {}
/// /endcode
/// ES9.0 12.2.5.2
/// Iterate the iterable source (as if using a for-of) and copy the values from
/// the spread source into the target array, starting at `nextIndex`.
/// \return the next empty index in the array to use for additional properties.
CallResult<HermesValue>
hermesBuiltinArraySpread(void *, Runtime &runtime, NativeArgs args) {
  GCScopeMarkerRAII topMarker{runtime};
  Handle<JSArray> target = args.dyncastArg<JSArray>(0);
  // To be safe, check for non-arrays.
  if (!target) {
    return runtime.raiseTypeError(
        "HermesBuiltin.arraySpread requires an array target");
  }

  MutableHandle<> nextValue{runtime};

  Handle<JSArray> arr = args.dyncastArg<JSArray>(1);
  if (arr) {
    // Copying from an array, first check and make sure that
    // `arr[Symbol.iterator]` hasn't been changed by the user.
    NamedPropertyDescriptor desc;
    PseudoHandle<JSObject> propObj =
        createPseudoHandle(JSObject::getNamedDescriptorPredefined(
            arr, runtime, Predefined::SymbolIterator, desc));
    if (LLVM_LIKELY(propObj) && LLVM_LIKELY(!desc.flags.proxyObject)) {
      PseudoHandle<> slotValue = createPseudoHandle(
          JSObject::getNamedSlotValueUnsafe(propObj.get(), runtime, desc)
              .unboxToHV(runtime));
      propObj.invalidate();
      if (LLVM_LIKELY(
              slotValue->getRaw() == runtime.arrayPrototypeValues.getRaw())) {
        slotValue.invalidate();
        auto nextIndex = args.getArg(2).getNumberAs<JSArray::size_type>();
        MutableHandle<> idxHandle{runtime};
        GCScopeMarkerRAII marker{runtime};
        for (JSArray::size_type i = 0; i < JSArray::getLength(*arr, runtime);
             ++i) {
          marker.flush();
          // Fast path: look up the property in indexed storage.
          nextValue = arr->at(runtime, i).unboxToHV(runtime);
          if (LLVM_UNLIKELY(nextValue->isEmpty())) {
            // Slow path, just run the full getComputed_RJS path.
            // Runs when there is a hole, accessor, non-regular property, etc.
            idxHandle = HermesValue::encodeNumberValue(i);
            CallResult<PseudoHandle<>> valueRes =
                JSObject::getComputed_RJS(arr, runtime, idxHandle);
            if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
              return ExecutionStatus::EXCEPTION;
            }
            nextValue = std::move(*valueRes);
          }
          // It is valid to use setElementAt here because we know that
          // `target` was created immediately prior to running the spread
          // and no non-standard properties were added to it,
          // because the only actions that can be performed between array
          // creation and running this spread are DefineOwnProperty calls with
          // standard flags (as well as other spread operations, which do the
          // same thing).
          JSArray::setElementAt(target, runtime, nextIndex, nextValue);
          ++nextIndex;
        }

        if (LLVM_UNLIKELY(
                JSArray::setLengthProperty(target, runtime, nextIndex) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }

        return HermesValue::encodeNumberValue(nextIndex);
      }
    }
  }

  // 3. Let iteratorRecord be ? GetIterator(spreadObj).
  auto iteratorRecordRes = getIterator(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(iteratorRecordRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  IteratorRecord iteratorRecord = *iteratorRecordRes;

  MutableHandle<> nextIndex{runtime, args.getArg(2)};

  // 4. Repeat,
  for (GCScopeMarkerRAII marker{runtime}; /* nothing */; marker.flush()) {
    // a. Let next be ? IteratorStep(iteratorRecord).
    auto nextRes = iteratorStep(runtime, iteratorRecord);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    Handle<JSObject> next = *nextRes;

    // b. If next is false, return nextIndex.
    if (!next) {
      return nextIndex.getHermesValue();
    }
    // c. Let nextValue be ? IteratorValue(next).
    auto nextItemRes = JSObject::getNamed_RJS(
        next, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextItemRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nextValue = std::move(*nextItemRes);

    // d. Let status be CreateDataProperty(array,
    //    ToString(ToUint32(nextIndex)), nextValue).
    // e. Assert: status is true.
    if (LLVM_UNLIKELY(
            JSArray::defineOwnComputed(
                target,
                runtime,
                nextIndex,
                DefinePropertyFlags::getDefaultNewPropertyFlags(),
                nextValue) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // f. Let nextIndex be nextIndex + 1.
    nextIndex = HermesValue::encodeNumberValue(nextIndex->getNumber() + 1);
  }

  return nextIndex.getHermesValue();
}

/// \code
///   HermesBuiltin.apply = function(fn, argArray, thisVal(opt)) {}
/// /endcode
/// Faster version of Function.prototype.apply which does not use its `this`
/// argument.
/// `argArray` must be a JSArray with no getters.
/// Equivalent to fn.apply(thisVal, argArray) if thisVal is provided.
/// If thisVal is not provided, equivalent to running `new fn` and passing the
/// arguments in argArray.
CallResult<HermesValue>
hermesBuiltinApply(void *, Runtime &runtime, NativeArgs args) {
  GCScopeMarkerRAII marker{runtime};

  Handle<Callable> fn = args.dyncastArg<Callable>(0);
  if (LLVM_UNLIKELY(!fn)) {
    return runtime.raiseTypeErrorForValue(
        args.getArgHandle(0), " is not a function");
  }

  Handle<JSArray> argArray = args.dyncastArg<JSArray>(1);
  if (LLVM_UNLIKELY(!argArray)) {
    return runtime.raiseTypeError("args must be an array");
  }

  uint32_t len = JSArray::getLength(*argArray, runtime);

  bool isConstructor = args.getArgCount() == 2;

  MutableHandle<> thisVal{runtime};
  if (isConstructor) {
    auto thisValRes = Callable::createThisForConstruct(fn, runtime);
    if (LLVM_UNLIKELY(thisValRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    thisVal = thisValRes->getHermesValue();
  } else {
    thisVal = args.getArg(2);
  }

  ScopedNativeCallFrame newFrame{
      runtime, len, *fn, isConstructor, thisVal.getHermesValue()};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);

  for (uint32_t i = 0; i < len; ++i) {
    assert(!argArray->at(runtime, i).isEmpty() && "arg array must be dense");
    HermesValue arg = argArray->at(runtime, i).unboxToHV(runtime);
    newFrame->getArgRef(i) = LLVM_UNLIKELY(arg.isEmpty())
        ? HermesValue::encodeUndefinedValue()
        : arg;
  }
  if (isConstructor) {
    auto res = Callable::construct(fn, runtime, thisVal);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return res->getHermesValue();
  }
  auto res = Callable::call(fn, runtime);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return res->getHermesValue();
}

/// HermesBuiltin.exportAll(exports, source) will copy exported named
/// properties from `source` to `exports`, defining them on `exports` as
/// non-configurable.
/// Note that the default exported property on `source` is ignored,
/// as are non-enumerable properties on `source`.
CallResult<HermesValue>
hermesBuiltinExportAll(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> exports = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!exports)) {
    return runtime.raiseTypeError(
        "exportAll() exports argument must be object");
  }

  Handle<JSObject> source = args.dyncastArg<JSObject>(1);
  if (LLVM_UNLIKELY(!source) || LLVM_UNLIKELY(source->isProxyObject())) {
    return runtime.raiseTypeError(
        "exportAll() source argument must be non-Proxy object");
  }

  MutableHandle<> propertyHandle{runtime};

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.configurable = 0;

  CallResult<bool> defineRes{ExecutionStatus::EXCEPTION};

  // Iterate the named properties excluding those which use Symbols.
  bool result = HiddenClass::forEachPropertyWhile(
      runtime.makeHandle(source->getClass(runtime)),
      runtime,
      [&source, &exports, &propertyHandle, &dpf, &defineRes](
          Runtime &runtime, SymbolID id, NamedPropertyDescriptor desc) {
        if (!desc.flags.enumerable)
          return true;

        if (id == Predefined::getSymbolID(Predefined::defaultExport)) {
          return true;
        }

        propertyHandle =
            JSObject::getNamedSlotValueUnsafe(*source, runtime, desc)
                .unboxToHV(runtime);
        defineRes = JSObject::defineOwnProperty(
            exports, runtime, id, dpf, propertyHandle);
        if (LLVM_UNLIKELY(defineRes == ExecutionStatus::EXCEPTION)) {
          return false;
        }

        return true;
      });
  if (LLVM_UNLIKELY(!result)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesBuiltinExponentiate(void *ctx, Runtime &runtime, NativeArgs args) {
  CallResult<HermesValue> res = toNumeric_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_LIKELY(!res->isBigInt())) {
    double left = res->getNumber();
    // Using ? toNumber() here causes an exception to be raised if args[1] is a
    // BigInt.
    CallResult<HermesValue> res = toNumber_RJS(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeNumberValue(expOp(left, res->getNumber()));
  }

  Handle<BigIntPrimitive> lhs = runtime.makeHandle(res->getBigInt());

  // Can't use toBigInt() here as it converts boolean/strings to bigint.
  res = toNumeric_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (!res->isBigInt()) {
    return runtime.raiseTypeErrorForValue(
        "Cannot convert ", args.getArgHandle(1), " to BigInt");
  }

  return BigIntPrimitive::exponentiate(
      runtime, std::move(lhs), runtime.makeHandle(res->getBigInt()));
}

void createHermesBuiltins(
    Runtime &runtime,
    llvh::MutableArrayRef<Callable *> builtins) {
  auto defineInternMethod = [&](BuiltinMethod::Enum builtinIndex,
                                Predefined::Str symID,
                                NativeFunctionPtr func,
                                uint8_t count = 0) {
    auto method = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime.functionPrototype),
        nullptr /* context */,
        func,
        Predefined::getSymbolID(symID),
        count,
        Runtime::makeNullHandle<JSObject>());

    assert(builtins[builtinIndex] == nullptr && "builtin already defined");
    builtins[builtinIndex] = *method;
  };

  // HermesBuiltin function properties
  namespace P = Predefined;
  namespace B = BuiltinMethod;
  defineInternMethod(
      B::HermesBuiltin_silentSetPrototypeOf,
      P::silentSetPrototypeOf,
      silentObjectSetPrototypeOf,
      2);
  defineInternMethod(
      B::HermesBuiltin_getTemplateObject,
      P::getTemplateObject,
      hermesBuiltinGetTemplateObject);
  defineInternMethod(
      B::HermesBuiltin_ensureObject,
      P::ensureObject,
      hermesBuiltinEnsureObject,
      2);
  defineInternMethod(
      B::HermesBuiltin_getMethod, P::getMethod, hermesBuiltinGetMethod, 2);
  defineInternMethod(
      B::HermesBuiltin_throwTypeError,
      P::throwTypeError,
      hermesBuiltinThrowTypeError,
      1);
  defineInternMethod(
      B::HermesBuiltin_generatorSetDelegated,
      P::generatorSetDelegated,
      hermesBuiltinGeneratorSetDelegated,
      1);
  defineInternMethod(
      B::HermesBuiltin_copyDataProperties,
      P::copyDataProperties,
      hermesBuiltinCopyDataProperties,
      3);
  defineInternMethod(
      B::HermesBuiltin_copyRestArgs,
      P::copyRestArgs,
      hermesBuiltinCopyRestArgs,
      1);
  defineInternMethod(
      B::HermesBuiltin_arraySpread,
      P::arraySpread,
      hermesBuiltinArraySpread,
      2);
  defineInternMethod(B::HermesBuiltin_apply, P::apply, hermesBuiltinApply, 2);
  defineInternMethod(
      B::HermesBuiltin_exportAll, P::exportAll, hermesBuiltinExportAll);
  defineInternMethod(
      B::HermesBuiltin_exponentiationOperator,
      P::exponentiationOperator,
      hermesBuiltinExponentiate);

  // Define the 'requireFast' function, which takes a number argument.
  defineInternMethod(
      B::HermesBuiltin_requireFast, P::requireFast, requireFast, 1);
}

} // namespace vm
} // namespace hermes
