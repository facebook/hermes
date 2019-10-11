/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "vm"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/Runtime.h"

#include "hermes/Inst/InstDecode.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/Support/Statistic.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JIT/JIT.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSGenerator.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Profiler.h"
#include "hermes/VM/Runtime-inline.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include "Interpreter-internal.h"

using llvm::dbgs;
using namespace hermes::inst;

HERMES_SLOW_STATISTIC(
    NumGetById,
    "NumGetById: Number of property 'read by id' accesses");
HERMES_SLOW_STATISTIC(
    NumGetByIdCacheHits,
    "NumGetByIdCacheHits: Number of property 'read by id' cache hits");
HERMES_SLOW_STATISTIC(
    NumGetByIdProtoHits,
    "NumGetByIdProtoHits: Number of property 'read by id' cache hits for the prototype");
HERMES_SLOW_STATISTIC(
    NumGetByIdCacheEvicts,
    "NumGetByIdCacheEvicts: Number of property 'read by id' cache evictions");
HERMES_SLOW_STATISTIC(
    NumGetByIdFastPaths,
    "NumGetByIdFastPaths: Number of property 'read by id' fast paths");
HERMES_SLOW_STATISTIC(
    NumGetByIdAccessor,
    "NumGetByIdAccessor: Number of property 'read by id' accessors");
HERMES_SLOW_STATISTIC(
    NumGetByIdProto,
    "NumGetByIdProto: Number of property 'read by id' in the prototype chain");
HERMES_SLOW_STATISTIC(
    NumGetByIdNotFound,
    "NumGetByIdNotFound: Number of property 'read by id' not found");
HERMES_SLOW_STATISTIC(
    NumGetByIdTransient,
    "NumGetByIdTransient: Number of property 'read by id' of non-objects");

HERMES_SLOW_STATISTIC(
    NumPutById,
    "NumPutById: Number of property 'write by id' accesses");
HERMES_SLOW_STATISTIC(
    NumPutByIdCacheHits,
    "NumPutByIdCacheHits: Number of property 'write by id' cache hits");
HERMES_SLOW_STATISTIC(
    NumPutByIdCacheEvicts,
    "NumPutByIdCacheEvicts: Number of property 'write by id' cache evictions");
HERMES_SLOW_STATISTIC(
    NumPutByIdFastPaths,
    "NumPutByIdFastPaths: Number of property 'write by id' fast paths");
HERMES_SLOW_STATISTIC(
    NumPutByIdTransient,
    "NumPutByIdTransient: Number of property 'write by id' to non-objects");

HERMES_SLOW_STATISTIC(
    NumNativeFunctionCalls,
    "NumNativeFunctionCalls: Number of native function calls");
HERMES_SLOW_STATISTIC(
    NumBoundFunctionCalls,
    "NumBoundCalls: Number of bound function calls");

// Ensure that instructions declared as having matching layouts actually do.
#include "InstLayout.inc"

#if defined(HERMESVM_PROFILER_EXTERN)
// External profiler mode wraps calls to each JS function with a unique native
// function that recusively calls the interpreter. See Profiler.{h,cpp} for how
// these symbols are subsequently patched with JS function names.
#define INTERP_WRAPPER(name)                                                \
  __attribute__((__noinline__)) static llvm::CallResult<llvm::HermesValue>  \
  name(hermes::vm::Runtime *runtime, hermes::vm::CodeBlock *newCodeBlock) { \
    return runtime->interpretFunctionImpl(newCodeBlock);                    \
  }
PROFILER_SYMBOLS(INTERP_WRAPPER)
#endif

namespace hermes {
namespace vm {

#if defined(HERMESVM_PROFILER_EXTERN)
typedef CallResult<HermesValue> (*WrapperFunc)(Runtime *, CodeBlock *);
#define LIST_ITEM(name) name,
static const WrapperFunc interpWrappers[] = {PROFILER_SYMBOLS(LIST_ITEM)};
#endif

/// Initialize the state of some internal variables based on the current
/// code block.
#define INIT_STATE_FOR_CODEBLOCK(codeBlock)                 \
  do {                                                      \
    strictMode = (codeBlock)->isStrictMode();               \
    defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(strictMode); \
  } while (0)

CallResult<HermesValue> Interpreter::createGeneratorClosure(
    Runtime *runtime,
    RuntimeModule *runtimeModule,
    unsigned funcIndex,
    Handle<Environment> envHandle) {
  return JSGeneratorFunction::create(
      runtime,
      runtimeModule->getDomain(runtime),
      Handle<JSObject>::vmcast(&runtime->generatorFunctionPrototype),
      envHandle,
      runtimeModule->getCodeBlockMayAllocate(funcIndex));
}

CallResult<HermesValue> Interpreter::createGenerator_RJS(
    Runtime *runtime,
    RuntimeModule *runtimeModule,
    unsigned funcIndex,
    Handle<Environment> envHandle,
    NativeArgs args) {
  auto gifRes = GeneratorInnerFunction::create(
      runtime,
      runtimeModule->getDomain(runtime),
      Handle<JSObject>::vmcast(&runtime->functionPrototype),
      envHandle,
      runtimeModule->getCodeBlockMayAllocate(funcIndex),
      args);
  if (LLVM_UNLIKELY(gifRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto generatorFunction = runtime->makeHandle(vmcast<JSGeneratorFunction>(
      runtime->getCurrentFrame().getCalleeClosure()));

  auto prototypeProp = JSObject::getNamed_RJS(
      generatorFunction,
      runtime,
      Predefined::getSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(prototypeProp == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> prototype = vmisa<JSObject>(*prototypeProp)
      ? runtime->makeHandle<JSObject>(*prototypeProp)
      : Handle<JSObject>::vmcast(&runtime->generatorPrototype);

  auto res = JSGenerator::create(runtime, *gifRes, prototype);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return res->getHermesValue();
}

CallResult<HermesValue> Interpreter::reifyArgumentsSlowPath(
    Runtime *runtime,
    Handle<Callable> curFunction,
    bool strictMode) {
  auto frame = runtime->getCurrentFrame();
  uint32_t argCount = frame.getArgCount();
  // Define each JavaScript argument.
  auto argRes = Arguments::create(runtime, argCount, curFunction, strictMode);
  if (LLVM_UNLIKELY(argRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Arguments *args = vmcast<Arguments>(*argRes);

  for (uint32_t argIndex = 0; argIndex < argCount; ++argIndex) {
    Arguments::unsafeSetExistingElementAt(
        args, runtime, argIndex, frame.getArgRef(argIndex));
  }

  // The returned value should already be set from the create call.
  return argRes;
}

CallResult<HermesValue> Interpreter::getArgumentsPropByValSlowPath_RJS(
    Runtime *runtime,
    PinnedHermesValue *lazyReg,
    PinnedHermesValue *valueReg,
    Handle<Callable> curFunction,
    bool strictMode) {
  auto frame = runtime->getCurrentFrame();

  // If the arguments object has already been created.
  if (!lazyReg->isUndefined()) {
    // The arguments object has been created, so this is a regular property
    // get.
    assert(lazyReg->isObject() && "arguments lazy register is not an object");

    return JSObject::getComputed_RJS(
        Handle<JSObject>::vmcast(lazyReg), runtime, Handle<>(valueReg));
  }

  if (!valueReg->isSymbol()) {
    // Attempt a fast path in the case that the key is not a symbol.
    // If it is a symbol, force reification for now.
    // Convert the value to a string.
    auto strRes = toString_RJS(runtime, Handle<>(valueReg));
    if (strRes == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    auto strPrim = toHandle(runtime, std::move(*strRes));

    // Check if the string is a valid argument index.
    if (auto index = toArrayIndex(runtime, strPrim)) {
      if (*index < frame.getArgCount()) {
        return frame.getArgRef(*index);
      }

      auto objectPrototype =
          Handle<JSObject>::vmcast(&runtime->objectPrototype);

      // OK, they are requesting an index that either doesn't exist or is
      // somewhere up in the prototype chain. Since we want to avoid reifying,
      // check which it is:
      MutableHandle<JSObject> inObject{runtime};
      ComputedPropertyDescriptor desc;
      JSObject::getComputedPrimitiveDescriptor(
          objectPrototype, runtime, strPrim, inObject, desc);

      // If we couldn't find the property, just return 'undefined'.
      if (!inObject)
        return HermesValue::encodeUndefinedValue();

      // If the property isn't an accessor, we can just return it without
      // reifying.
      if (!desc.flags.accessor) {
        return JSObject::getComputedSlotValue(inObject.get(), runtime, desc);
      }
    }

    // Are they requesting "arguments.length"?
    if (runtime->symbolEqualsToStringPrim(
            Predefined::getSymbolID(Predefined::length), *strPrim)) {
      return HermesValue::encodeDoubleValue(frame.getArgCount());
    }
  }

  // Looking for an accessor or a property that needs reification.
  auto argRes = reifyArgumentsSlowPath(runtime, curFunction, strictMode);
  if (argRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Update the register with the reified value.
  *lazyReg = *argRes;

  // For simplicity, call ourselves again.
  return getArgumentsPropByValSlowPath_RJS(
      runtime, lazyReg, valueReg, curFunction, strictMode);
}

ExecutionStatus Interpreter::handleGetPNameList(
    Runtime *runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  if (O2REG(GetPNameList).isUndefined() || O2REG(GetPNameList).isNull()) {
    // Set the iterator to be undefined value.
    O1REG(GetPNameList) = HermesValue::encodeUndefinedValue();
    return ExecutionStatus::RETURNED;
  }

  // Convert to object and store it back to the register.
  auto res = toObject(runtime, Handle<>(&O2REG(GetPNameList)));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  O2REG(GetPNameList) = res.getValue();

  auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));
  uint32_t beginIndex;
  uint32_t endIndex;
  auto cr = getForInPropertyNames(runtime, obj, beginIndex, endIndex);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arr = *cr;
  O1REG(GetPNameList) = arr.getHermesValue();
  O3REG(GetPNameList) = HermesValue::encodeNumberValue(beginIndex);
  O4REG(GetPNameList) = HermesValue::encodeNumberValue(endIndex);
  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> Interpreter::handleCallSlowPath(
    Runtime *runtime,
    PinnedHermesValue *callTarget) {
  if (auto *native = dyn_vmcast<NativeFunction>(*callTarget)) {
    ++NumNativeFunctionCalls;
    // Call the native function directly
    return NativeFunction::_nativeCall(native, runtime);
  } else if (auto *bound = dyn_vmcast<BoundFunction>(*callTarget)) {
    ++NumBoundFunctionCalls;
    // Call the bound function.
    return BoundFunction::_boundCall(bound, runtime);
  } else {
    return runtime->raiseTypeErrorForValue(
        Handle<>(callTarget), " is not a function");
  }
}

inline OptValue<HermesValue> Interpreter::tryGetPrimitiveOwnPropertyById(
    Runtime *runtime,
    Handle<> base,
    SymbolID id) {
  if (base->isString() && id == Predefined::getSymbolID(Predefined::length)) {
    return HermesValue::encodeNumberValue(base->getString()->getStringLength());
  }
  return llvm::None;
}

CallResult<HermesValue> Interpreter::getByIdTransient_RJS(
    Runtime *runtime,
    Handle<> base,
    SymbolID id) {
  // ES5.1 8.7.1 special [[Get]] internal method.

  // Spec steps #1 and #2 are optimized with fast path below.

  // Fast path #1: try to get primitive own property directly first.
  OptValue<HermesValue> valOpt =
      tryGetPrimitiveOwnPropertyById(runtime, base, id);
  if (valOpt.hasValue()) {
    return valOpt.getValue();
  }

  // Fast path #2: get the property descriptor from primitive prototype
  // without boxing with vm::toObject().
  CallResult<Handle<JSObject>> primitivePrototypeResult =
      getPrimitivePrototype(runtime, base);
  if (primitivePrototypeResult == ExecutionStatus::EXCEPTION) {
    // If an exception is thrown, likely we are trying to read property on
    // undefined/null. Passing over the name of the property
    // so that we could emit more meaningful error messages.
    return amendPropAccessErrorMsgWithPropName(runtime, base, "read", id);
  }

  NamedPropertyDescriptor desc;
  JSObject *propObj = JSObject::getNamedDescriptor(
      primitivePrototypeResult.getValue(), runtime, id, desc);

  // 3. If desc is undefined, return undefined.
  if (!propObj)
    return HermesValue::encodeUndefinedValue();

  HermesValue slotValue = JSObject::getNamedSlotValue(propObj, runtime, desc);

  // 4. If IsDataDescriptor(desc) is true, return desc.[[Value]].
  if (LLVM_LIKELY(!desc.flags.accessor))
    return slotValue;

  // 5. Otherwise, IsAccessorDescriptor(desc) must be true so, let getter be
  //  desc.[[Get]] (see 8.10).
  // 6. If getter is undefined, return undefined.
  auto *accessor = vmcast<PropertyAccessor>(slotValue);
  if (!accessor->getter)
    return HermesValue::encodeUndefinedValue();

  // 7. Return the result calling the [[Call]] internal method of getter
  //  providing base as the this value and providing no arguments.
  return accessor->getter.get(runtime)->executeCall0(
      runtime->makeHandle(accessor->getter), runtime, base);
}

OptValue<HermesValue> Interpreter::getByValTransientFast(
    Runtime *runtime,
    Handle<> base,
    Handle<> nameHandle) {
  if (base->isString()) {
    // Handle most common fast path -- array index property for string
    // primitive.
    // Since primitive string cannot have index like property we can
    // skip ObjectFlags::fastIndexProperties checking and directly
    // checking index storage from StringPrimitive.

    OptValue<uint32_t> arrayIndex = toArrayIndexFastPath(*nameHandle);
    // Get character directly from primitive if arrayIndex is within range.
    // Otherwise we need to fall back to prototype lookup.
    if (arrayIndex &&
        arrayIndex.getValue() < base->getString()->getStringLength()) {
      return runtime
          ->getCharacterString(base->getString()->at(arrayIndex.getValue()))
          .getHermesValue();
    }
  }
  return llvm::None;
}

CallResult<HermesValue> Interpreter::getByValTransient_RJS(
    Runtime *runtime,
    Handle<> base,
    Handle<> name) {
  // ES5.1 8.7.1 special [[Get]] internal method.

  // Optimization: check fast path first.
  OptValue<HermesValue> fastRes = getByValTransientFast(runtime, base, name);
  if (fastRes.hasValue()) {
    return fastRes.getValue();
  }

  // 1. Let O be ToObject(base)
  auto res = toObject(runtime, base);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  auto O = runtime->makeHandle<JSObject>(res.getValue());

  // 2. Let desc be the result of calling the [[GetProperty]] internal method of
  //  O with property name P.
  ComputedPropertyDescriptor desc;
  MutableHandle<JSObject> propObj{runtime};
  if (JSObject::getComputedDescriptor(O, runtime, name, propObj, desc) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // 3. If desc is undefined, return undefined.
  if (!propObj)
    return HermesValue::encodeUndefinedValue();

  HermesValue slotValue =
      JSObject::getComputedSlotValue(propObj.get(), runtime, desc);

  // 4. If IsDataDescriptor(desc) is true, return desc.[[Value]].
  if (LLVM_LIKELY(!desc.flags.accessor))
    return slotValue;

  // 5. Otherwise, IsAccessorDescriptor(desc) must be true so, let getter be
  //  desc.[[Get]] (see 8.10).
  // 6. If getter is undefined, return undefined.
  auto *accessor = vmcast<PropertyAccessor>(slotValue);
  if (!accessor->getter)
    return HermesValue::encodeUndefinedValue();

  // 7. Return the result calling the [[Call]] internal method of getter
  //  providing base as the this value and providing no arguments.
  return accessor->getter.get(runtime)->executeCall0(
      runtime->makeHandle(accessor->getter), runtime, base);
}

ExecutionStatus Interpreter::putByIdTransient_RJS(
    Runtime *runtime,
    Handle<> base,
    SymbolID id,
    Handle<> value,
    bool strictMode) {
  // ES5.1 8.7.2 special [[Get]] internal method.
  // TODO: avoid boxing primitives unless we are calling an accessor.

  // 1. Let O be ToObject(base)
  auto res = toObject(runtime, base);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    // If an exception is thrown, likely we are trying to convert
    // undefined/null to an object. Passing over the name of the property
    // so that we could emit more meaningful error messages.
    return amendPropAccessErrorMsgWithPropName(runtime, base, "set", id);
  }

  auto O = runtime->makeHandle<JSObject>(res.getValue());

  NamedPropertyDescriptor desc;
  JSObject *propObj = JSObject::getNamedDescriptor(O, runtime, id, desc);

  // Is this a missing property, or a data property defined in the prototype
  // chain? In both cases we would need to create an own property on the
  // transient object, which is prohibited.
  if (!propObj || (!desc.flags.accessor && propObj != O.get())) {
    if (strictMode) {
      return runtime->raiseTypeError(
          "Cannot create a new property on a transient object");
    }
    return ExecutionStatus::RETURNED;
  }

  // Modifying an own data property in a transient object is prohibited.
  if (!desc.flags.accessor) {
    if (strictMode) {
      return runtime->raiseTypeError(
          "Cannot modify a property in a transient object");
    }
    return ExecutionStatus::RETURNED;
  }

  // This is an accessor.
  auto *accessor = vmcast<PropertyAccessor>(
      JSObject::getNamedSlotValue(propObj, runtime, desc));

  // It needs to have a setter.
  if (!accessor->setter) {
    if (strictMode) {
      return runtime->raiseTypeError("Cannot modify a read-only accessor");
    }
    return ExecutionStatus::RETURNED;
  }

  // PutById only needs an ExecutionStatus but not a CallResult<HermesValue> as
  // return value
  return accessor->setter.get(runtime)
      ->executeCall1(
          runtime->makeHandle(accessor->setter), runtime, base, *value)
      .getStatus();
}

ExecutionStatus Interpreter::putByValTransient_RJS(
    Runtime *runtime,
    Handle<> base,
    Handle<> name,
    Handle<> value,
    bool strictMode) {
  auto idRes = valueToSymbolID(runtime, name);
  if (idRes == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  return putByIdTransient_RJS(runtime, base, **idRes, value, strictMode);
}

CallResult<HermesValue> Interpreter::createObjectFromBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    unsigned numLiterals,
    unsigned keyBufferIndex,
    unsigned valBufferIndex) {
  // Fetch any cached hidden class first.
  auto *runtimeModule = curCodeBlock->getRuntimeModule();
  const llvm::Optional<Handle<HiddenClass>> optCachedHiddenClassHandle =
      runtimeModule->findCachedLiteralHiddenClass(keyBufferIndex, numLiterals);
  // Create a new object using the built-in constructor or cached hidden class.
  // Note that the built-in constructor is empty, so we don't actually need to
  // call it.
  auto obj = toHandle(
      runtime,
      optCachedHiddenClassHandle.hasValue()
          ? JSObject::create(runtime, optCachedHiddenClassHandle.getValue())
          : JSObject::create(runtime, numLiterals));

  MutableHandle<> tmpHandleKey(runtime);
  MutableHandle<> tmpHandleVal(runtime);
  auto &gcScope = *runtime->getTopGCScope();
  auto marker = gcScope.createMarker();

  auto genPair = curCodeBlock->getObjectBufferIter(
      keyBufferIndex, valBufferIndex, numLiterals);
  auto keyGen = genPair.first;
  auto valGen = genPair.second;

  if (optCachedHiddenClassHandle.hasValue()) {
    uint32_t propIndex = 0;
    // keyGen should always have the same amount of elements as valGen
    while (valGen.hasNext()) {
#ifndef NDEBUG
      {
        // keyGen points to an element in the key buffer, which means it will
        // only ever generate a Number or a Symbol. This means it will never
        // allocate memory, and it is safe to not use a Handle.
        SymbolID stringIdResult{};
        auto key = keyGen.get(runtime);
        if (key.isSymbol()) {
          stringIdResult = ID(key.getSymbol().unsafeGetIndex());
        } else {
          tmpHandleKey = HermesValue::encodeDoubleValue(key.getNumber());
          auto idRes = valueToSymbolID(runtime, tmpHandleKey);
          assert(
              idRes != ExecutionStatus::EXCEPTION &&
              "valueToIdentifier() failed for uint32_t value");
          stringIdResult = **idRes;
        }
        NamedPropertyDescriptor desc;
        auto pos = HiddenClass::findProperty(
            optCachedHiddenClassHandle.getValue(),
            runtime,
            stringIdResult,
            PropertyFlags::defaultNewNamedPropertyFlags(),
            desc);
        assert(
            pos &&
            "Should find this property in cached hidden class property table.");
        assert(
            desc.slot == propIndex &&
            "propIndex should be the same as recorded in hidden class table.");
      }
#endif
      // Explicitly make sure valGen.get() is called before obj.get() so that
      // any allocation in valGen.get() won't invalidate the raw pointer
      // retruned from obj.get().
      auto val = valGen.get(runtime);
      JSObject::setNamedSlotValue(obj.get(), runtime, propIndex, val);
      gcScope.flushToMarker(marker);
      ++propIndex;
    }
  } else {
    // keyGen should always have the same amount of elements as valGen
    while (keyGen.hasNext()) {
      // keyGen points to an element in the key buffer, which means it will
      // only ever generate a Number or a Symbol. This means it will never
      // allocate memory, and it is safe to not use a Handle.
      auto key = keyGen.get(runtime);
      tmpHandleVal = valGen.get(runtime);
      if (key.isSymbol()) {
        auto stringIdResult = ID(key.getSymbol().unsafeGetIndex());
        if (LLVM_UNLIKELY(
                JSObject::defineNewOwnProperty(
                    obj,
                    runtime,
                    stringIdResult,
                    PropertyFlags::defaultNewNamedPropertyFlags(),
                    tmpHandleVal) == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      } else {
        tmpHandleKey = HermesValue::encodeDoubleValue(key.getNumber());
        if (LLVM_UNLIKELY(
                !JSObject::defineOwnComputedPrimitive(
                     obj,
                     runtime,
                     tmpHandleKey,
                     DefinePropertyFlags::getDefaultNewPropertyFlags(),
                     tmpHandleVal)
                     .getValue())) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      gcScope.flushToMarker(marker);
    }
  }
  tmpHandleKey.clear();
  tmpHandleVal.clear();

  // Hidden class in dictionary mode can't be shared.
  HiddenClass *const clazz = obj->getClass(runtime);
  if (!optCachedHiddenClassHandle.hasValue() && !clazz->isDictionary()) {
    assert(
        numLiterals == clazz->getNumProperties() &&
        "numLiterals should match hidden class property count.");
    assert(
        clazz->getNumProperties() < 256 &&
        "cached hidden class should have property count less than 256");
    runtimeModule->tryCacheLiteralHiddenClass(keyBufferIndex, clazz);
  }

  return HermesValue::encodeObjectValue(*obj);
}

CallResult<HermesValue> Interpreter::createArrayFromBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    unsigned numElements,
    unsigned numLiterals,
    unsigned bufferIndex) {
  // Create a new array using the built-in constructor, and initialize
  // the elements from a literal array buffer.
  auto arrRes = JSArray::create(runtime, numElements, numElements);
  if (arrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // Resize the array storage in advance.
  auto arr = toHandle(runtime, std::move(*arrRes));
  JSArray::setStorageEndIndex(arr, runtime, numElements);

  auto iter = curCodeBlock->getArrayBufferIter(bufferIndex, numLiterals);
  JSArray::size_type i = 0;
  while (iter.hasNext()) {
    // NOTE: we must get the value in a separate step to guarantee ordering.
    auto value = iter.get(runtime);
    JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, value);
  }

  return HermesValue::encodeObjectValue(*arr);
}

#ifndef NDEBUG
namespace {
/// A tag used to instruct the output stream to dump more details about the
/// HermesValue, like the length of the string, etc.
struct DumpHermesValue {
  const HermesValue hv;
  DumpHermesValue(HermesValue hv) : hv(hv) {}
};

} // anonymous namespace.

static llvm::raw_ostream &operator<<(
    llvm::raw_ostream &OS,
    DumpHermesValue dhv) {
  OS << dhv.hv;
  // If it is a string, dump the contents, truncated to 8 characters.
  if (dhv.hv.isString()) {
    SmallU16String<32> str;
    dhv.hv.getString()->copyUTF16String(str);
    UTF16Ref ref = str.arrayRef();
    if (str.size() <= 8) {
      OS << ":'" << ref << "'";
    } else {
      OS << ":'" << ref.slice(0, 8) << "'";
      OS << "...[" << str.size() << "]";
    }
  }
  return OS;
}

/// Dump the arguments from a callee frame.
LLVM_ATTRIBUTE_UNUSED
static void dumpCallArguments(
    llvm::raw_ostream &OS,
    Runtime *runtime,
    StackFramePtr calleeFrame) {
  OS << "arguments:\n";
  OS << "  " << 0 << " " << DumpHermesValue(calleeFrame.getThisArgRef())
     << "\n";
  for (unsigned i = 0; i < calleeFrame.getArgCount(); ++i) {
    OS << "  " << i << " " << DumpHermesValue(calleeFrame.getArgRef(i)) << "\n";
  }
}

LLVM_ATTRIBUTE_UNUSED
static void printDebugInfo(
    CodeBlock *curCodeBlock,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  // Check if LLVm debugging is enabled for us.
  bool debug = false;
  SLOW_DEBUG(debug = true);
  if (!debug)
    return;

  DecodedInstruction decoded = decodeInstruction(ip);

  dbgs() << llvm::format_decimal((const uint8_t *)ip - curCodeBlock->begin(), 4)
         << " OpCode::" << getOpCodeString(decoded.meta.opCode);

  for (unsigned i = 0; i < decoded.meta.numOperands; ++i) {
    auto operandType = decoded.meta.operandType[i];
    auto value = decoded.operandValue[i];

    dbgs() << (i == 0 ? " " : ", ");
    dumpOperand(dbgs(), operandType, value);

    if (operandType == OperandType::Reg8 || operandType == OperandType::Reg32) {
      // Print the register value, if source.
      if (i != 0 || decoded.meta.numOperands == 1)
        dbgs() << "=" << DumpHermesValue(REG(value.integer));
    }
  }

  dbgs() << "\n";
}

/// \return whether \p opcode is a call opcode (Call, CallDirect, Construct,
/// CallLongIndex, etc). Note CallBuiltin is not really a Call.
LLVM_ATTRIBUTE_UNUSED
static bool isCallType(OpCode opcode) {
  switch (opcode) {
#define DEFINE_RET_TARGET(name) \
  case OpCode::name:            \
    return true;
#include "hermes/BCGen/HBC/BytecodeList.def"
    default:
      return false;
  }
}

#endif

/// \return the address of the next instruction after \p ip, which must be a
/// call-type instruction.
LLVM_ATTRIBUTE_ALWAYS_INLINE
static inline const Inst *nextInstCall(const Inst *ip) {
  HERMES_SLOW_ASSERT(isCallType(ip->opCode) && "ip is not of call type");

  // The following is written to elicit compares instead of table lookup.
  // The idea is to present code like so:
  //   if (opcode <= 70) return ip + 4;
  //   if (opcode <= 71) return ip + 4;
  //   if (opcode <= 72) return ip + 4;
  //   if (opcode <= 73) return ip + 5;
  //   if (opcode <= 74) return ip + 5;
  //   ...
  // and the compiler will retain only compares where the result changes (here,
  // 72 and 74). This allows us to compute the next instruction using three
  // compares, instead of a naive compare-per-call type (or lookup table).
  //
  // Statically verify that increasing call opcodes correspond to monotone
  // instruction sizes; this enables the compiler to do a better job optimizing.
  constexpr bool callSizesMonotoneIncreasing = monotoneIncreasing(
#define DEFINE_RET_TARGET(name) sizeof(inst::name##Inst),
#include "hermes/BCGen/HBC/BytecodeList.def"
      SIZE_MAX // sentinel avoiding a trailing comma.
  );
  static_assert(
      callSizesMonotoneIncreasing,
      "Call instruction sizes are not monotone increasing");

#define DEFINE_RET_TARGET(name)   \
  if (ip->opCode <= OpCode::name) \
    return NEXTINST(name);
#include "hermes/BCGen/HBC/BytecodeList.def"
  llvm_unreachable("Not a call type");
}

CallResult<HermesValue> Runtime::interpretFunctionImpl(
    CodeBlock *newCodeBlock) {
  InterpreterState state{newCodeBlock, 0};
  return Interpreter::interpretFunction<false>(this, state);
}

CallResult<HermesValue> Runtime::interpretFunction(CodeBlock *newCodeBlock) {
#ifdef HERMESVM_PROFILER_EXTERN
  auto id = getProfilerID(newCodeBlock);
  if (id >= NUM_PROFILER_SYMBOLS) {
    id = NUM_PROFILER_SYMBOLS - 1; // Overflow entry.
  }
  return interpWrappers[id](this, newCodeBlock);
#else
  return interpretFunctionImpl(newCodeBlock);
#endif
}

#ifdef HERMES_ENABLE_DEBUGGER
ExecutionStatus Runtime::stepFunction(InterpreterState &state) {
  return Interpreter::interpretFunction<true>(this, state).getStatus();
}
#endif

/// \return the quotient of x divided by y.
static double doDiv(double x, double y)
    LLVM_NO_SANITIZE("float-divide-by-zero");
static inline double doDiv(double x, double y) {
  // UBSan will complain about float divide by zero as our implementation
  // of OpCode::Div depends on IEEE 754 float divide by zero. All modern
  // compilers implement this and there is no trivial work-around without
  // sacrificing performance and readability.
  // NOTE: This was pulled out of the interpreter to avoid putting the sanitize
  // silencer on the entire interpreter function.
  return x / y;
}

/// \return the product of x multiplied by y.
static inline double doMult(double x, double y) {
  return x * y;
}

/// \return the difference of y subtracted from x.
static inline double doSub(double x, double y) {
  return x - y;
}

template <bool SingleStep>
CallResult<HermesValue> Interpreter::interpretFunction(
    Runtime *runtime,
    InterpreterState &state) {
#ifndef HERMES_ENABLE_DEBUGGER
  static_assert(!SingleStep, "can't use single-step mode without the debugger");
#endif
  // Make sure that the cache can use an optimization by avoiding a branch to
  // access the property storage.
  static_assert(
      HiddenClass::kDictionaryThreshold <=
          SegmentedArray::kValueToSegmentThreshold,
      "Cannot avoid branches in cache check if the dictionary "
      "crossover point is larger than the inline storage");

  CodeBlock *curCodeBlock = state.codeBlock;
  const Inst *ip = nullptr;
  // Holds runtime->currentFrame_.ptr()-1 which is the first local
  // register. This eliminates the indirect load from Runtime and the -1 offset.
  PinnedHermesValue *frameRegs;
  // Strictness of current function.
  bool strictMode;
  // Default flags when accessing properties.
  PropOpFlags defaultPropOpFlags;

  LLVM_DEBUG(dbgs() << "interpretFunction() called\n");

  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime->raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }

  if (!SingleStep) {
    curCodeBlock->lazyCompile(runtime);
    if (auto jitPtr = runtime->jitContext_.compile(runtime, curCodeBlock))
      return (*jitPtr)(runtime);
  }

  GCScope gcScope(runtime);
  // Avoid allocating a handle dynamically by reusing this one.
  MutableHandle<> tmpHandle(runtime);
  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  CallResult<bool> boolRes{ExecutionStatus::EXCEPTION};

  // Mark the gcScope so we can clear all allocated handles.
  // Remember how many handles the scope has so we can clear them in the loop.
  static constexpr unsigned KEEP_HANDLES = 1;
  assert(
      gcScope.getHandleCountDbg() == KEEP_HANDLES &&
      "scope has unexpected number of handles");

  INIT_OPCODE_PROFILER;

#if !defined(HERMESVM_PROFILER_EXTERN)
tailCall:
#endif
  PROFILER_ENTER_FUNCTION(curCodeBlock);

#ifdef HERMES_ENABLE_DEBUGGER
  runtime->getDebugger().willEnterCodeBlock(curCodeBlock);
#endif

  // Update function executionCount_ count
  curCodeBlock->incrementExecutionCount();

  if (!SingleStep) {
    auto newFrame = runtime->setCurrentFrameToTopOfStack();
    runtime->saveCallerIPInStackFrame();

    // Point frameRegs to the first register in the new frame. Note that at this
    // moment technically it points above the top of the stack, but we are never
    // going to access it.
    frameRegs = &newFrame.getFirstLocalRef();

#ifndef NDEBUG
    LLVM_DEBUG(
        dbgs() << "function entry: stackLevel=" << runtime->getStackLevel()
               << ", argCount=" << runtime->getCurrentFrame().getArgCount()
               << ", frameSize=" << curCodeBlock->getFrameSize() << "\n");

    LLVM_DEBUG(
        dbgs() << " callee "
               << DumpHermesValue(
                      runtime->getCurrentFrame().getCalleeClosureOrCBRef())
               << "\n");
    LLVM_DEBUG(
        dbgs() << "   this "
               << DumpHermesValue(runtime->getCurrentFrame().getThisArgRef())
               << "\n");
    for (uint32_t i = 0; i != runtime->getCurrentFrame()->getArgCount(); ++i) {
      LLVM_DEBUG(
          dbgs() << " " << llvm::format_decimal(i, 4) << " "
                 << DumpHermesValue(runtime->getCurrentFrame().getArgRef(i))
                 << "\n");
    }
#endif

    // Allocate the registers for the new frame.
    if (LLVM_UNLIKELY(!runtime->checkAndAllocStack(
            curCodeBlock->getFrameSize() +
                StackFrameLayout::CalleeExtraRegistersAtStart,
            HermesValue::encodeUndefinedValue())))
      goto stackOverflow;

    ip = (Inst const *)curCodeBlock->begin();

    // Check for invalid invocation.
    if (LLVM_UNLIKELY(curCodeBlock->getHeaderFlags().isCallProhibited(
            newFrame.isConstructorCall()))) {
      if (!newFrame.isConstructorCall()) {
        runtime->raiseTypeError("Class constructor invoked without new");
      } else {
        runtime->raiseTypeError("Function is not a constructor");
      }
      goto handleExceptionInParent;
    }
  } else {
    // Point frameRegs to the first register in the frame.
    frameRegs = &runtime->getCurrentFrame().getFirstLocalRef();
    ip = (Inst const *)(curCodeBlock->begin() + state.offset);
  }

  assert((const uint8_t *)ip < curCodeBlock->end() && "CodeBlock is empty");

  INIT_STATE_FOR_CODEBLOCK(curCodeBlock);

#define BEFORE_OP_CODE                                                       \
  {                                                                          \
    UPDATE_OPCODE_TIME_SPENT;                                                \
    HERMES_SLOW_ASSERT((printDebugInfo(curCodeBlock, frameRegs, ip), true)); \
    HERMES_SLOW_ASSERT(                                                      \
        gcScope.getHandleCountDbg() == KEEP_HANDLES &&                       \
        "unaccounted handles were created");                                 \
    HERMES_SLOW_ASSERT(tmpHandle->isUndefined() && "tmpHandle not cleared"); \
    RECORD_OPCODE_START_TIME;                                                \
    INC_OPCODE_COUNT;                                                        \
  }

#ifdef HERMESVM_INDIRECT_THREADING
  static void *opcodeDispatch[] = {
#define DEFINE_OPCODE(name) &&case_##name,
#include "hermes/BCGen/HBC/BytecodeList.def"
      &&case__last};

#define CASE(name) case_##name:
#define DISPATCH                                \
  BEFORE_OP_CODE;                               \
  if (SingleStep) {                             \
    state.codeBlock = curCodeBlock;             \
    state.offset = CUROFFSET;                   \
    return HermesValue::encodeUndefinedValue(); \
  }                                             \
  goto *opcodeDispatch[(unsigned)ip->opCode]

#else // HERMESVM_INDIRECT_THREADING

#define CASE(name) case OpCode::name:
#define DISPATCH                                \
  if (SingleStep) {                             \
    state.codeBlock = curCodeBlock;             \
    state.offset = CUROFFSET;                   \
    return HermesValue::encodeUndefinedValue(); \
  }                                             \
  continue

#endif // HERMESVM_INDIRECT_THREADING

  for (;;) {
    BEFORE_OP_CODE;

#ifdef HERMESVM_INDIRECT_THREADING
    goto *opcodeDispatch[(unsigned)ip->opCode];
#else
    switch (ip->opCode)
#endif
    {
      const Inst *nextIP;
      uint32_t idVal;
      bool tryProp;
      uint32_t callArgCount;
      // This is HermesValue::getRaw(), since HermesValue cannot be assigned
      // to. It is meant to be used only for very short durations, in the
      // dispatch of call instructions, when there is definitely no possibility
      // of a GC.
      uint64_t callNewTarget;

/// Handle an opcode \p name with an out-of-line implementation in a function
///   ExecutionStatus caseName(
///       Runtime *,
///       PinnedHermesValue *frameRegs,
///       Inst *ip)
#define CASE_OUTOFLINE(name)                      \
  CASE(name) {                                    \
    if (LLVM_UNLIKELY(                            \
            case##name(runtime, frameRegs, ip) == \
            ExecutionStatus::EXCEPTION)) {        \
      goto exception;                             \
    }                                             \
    gcScope.flushToSmallCount(KEEP_HANDLES);      \
    ip = NEXTINST(name);                          \
    DISPATCH;                                     \
  }

/// Implement a binary arithmetic instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction. The fast path case will have a
///     "n" appended to the name.
/// \param oper the C++ operator to use to actually perform the arithmetic
///     operation.
#define BINOP(name, oper)                                                \
  CASE(name) {                                                           \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) { \
      /* Fast-path. */                                                   \
      CASE(name##N) {                                                    \
        O1REG(name) = HermesValue::encodeDoubleValue(                    \
            oper(O2REG(name).getNumber(), O3REG(name).getNumber()));     \
        ip = NEXTINST(name);                                             \
        DISPATCH;                                                        \
      }                                                                  \
    }                                                                    \
    runtime->storeCallerIP(ip);                                          \
    res = toNumber_RJS(runtime, Handle<>(&O2REG(name)));                 \
    runtime->clearCallerIP();                                            \
    if (res == ExecutionStatus::EXCEPTION)                               \
      goto exception;                                                    \
    double left = res->getDouble();                                      \
    runtime->storeCallerIP(ip);                                          \
    res = toNumber_RJS(runtime, Handle<>(&O3REG(name)));                 \
    runtime->clearCallerIP();                                            \
    if (res == ExecutionStatus::EXCEPTION)                               \
      goto exception;                                                    \
    O1REG(name) =                                                        \
        HermesValue::encodeDoubleValue(oper(left, res->getDouble()));    \
    gcScope.flushToSmallCount(KEEP_HANDLES);                             \
    ip = NEXTINST(name);                                                 \
    DISPATCH;                                                            \
  }

/// Implement a shift instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction.
/// \param oper the C++ operator to use to actually perform the shift
///     operation.
/// \param lConv the conversion function for the LHS of the expression.
/// \param lType the type of the LHS operand.
/// \param returnType the type of the return value.
#define SHIFTOP(name, oper, lConv, lType, returnType)                     \
  CASE(name) {                                                            \
    if (LLVM_LIKELY(                                                      \
            O2REG(name).isNumber() &&                                     \
            O3REG(name).isNumber())) { /* Fast-path. */                   \
      auto lnum = static_cast<lType>(                                     \
          hermes::truncateToInt32(O2REG(name).getNumber()));              \
      auto rnum = static_cast<uint32_t>(                                  \
                      hermes::truncateToInt32(O3REG(name).getNumber())) & \
          0x1f;                                                           \
      O1REG(name) = HermesValue::encodeDoubleValue(                       \
          static_cast<returnType>(lnum oper rnum));                       \
      ip = NEXTINST(name);                                                \
      DISPATCH;                                                           \
    }                                                                     \
    runtime->storeCallerIP(ip);                                           \
    res = lConv(runtime, Handle<>(&O2REG(name)));                         \
    runtime->clearCallerIP();                                             \
    if (res == ExecutionStatus::EXCEPTION) {                              \
      goto exception;                                                     \
    }                                                                     \
    auto lnum = static_cast<lType>(res->getNumber());                     \
    runtime->storeCallerIP(ip);                                           \
    res = toUInt32_RJS(runtime, Handle<>(&O3REG(name)));                  \
    runtime->clearCallerIP();                                             \
    if (res == ExecutionStatus::EXCEPTION) {                              \
      goto exception;                                                     \
    }                                                                     \
    auto rnum = static_cast<uint32_t>(res->getNumber()) & 0x1f;           \
    gcScope.flushToSmallCount(KEEP_HANDLES);                              \
    O1REG(name) = HermesValue::encodeDoubleValue(                         \
        static_cast<returnType>(lnum oper rnum));                         \
    ip = NEXTINST(name);                                                  \
    DISPATCH;                                                             \
  }

/// Implement a binary bitwise instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction.
/// \param oper the C++ operator to use to actually perform the bitwise
///     operation.
#define BITWISEBINOP(name, oper)                                               \
  CASE(name) {                                                                 \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) {       \
      /* Fast-path. */                                                         \
      O1REG(name) = HermesValue::encodeDoubleValue(                            \
          hermes::truncateToInt32(O2REG(name).getNumber())                     \
              oper hermes::truncateToInt32(O3REG(name).getNumber()));          \
      ip = NEXTINST(name);                                                     \
      DISPATCH;                                                                \
    }                                                                          \
    runtime->storeCallerIP(ip);                                                \
    res = toInt32_RJS(runtime, Handle<>(&O2REG(name)));                        \
    runtime->clearCallerIP();                                                  \
    if (res == ExecutionStatus::EXCEPTION) {                                   \
      goto exception;                                                          \
    }                                                                          \
    int32_t left = res->getNumberAs<int32_t>();                                \
    runtime->storeCallerIP(ip);                                                \
    res = toInt32_RJS(runtime, Handle<>(&O3REG(name)));                        \
    runtime->clearCallerIP();                                                  \
    if (res == ExecutionStatus::EXCEPTION) {                                   \
      goto exception;                                                          \
    }                                                                          \
    O1REG(name) =                                                              \
        HermesValue::encodeNumberValue(left oper res->getNumberAs<int32_t>()); \
    gcScope.flushToSmallCount(KEEP_HANDLES);                                   \
    ip = NEXTINST(name);                                                       \
    DISPATCH;                                                                  \
  }

/// Implement a comparison instruction.
/// \param name the name of the instruction.
/// \param oper the C++ operator to use to actually perform the fast arithmetic
///     comparison.
/// \param operFuncName  function to call for the slow-path comparison.
#define CONDOP(name, oper, operFuncName)                                       \
  CASE(name) {                                                                 \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) {       \
      /* Fast-path. */                                                         \
      O1REG(name) = HermesValue::encodeBoolValue(                              \
          O2REG(name).getNumber() oper O3REG(name).getNumber());               \
      ip = NEXTINST(name);                                                     \
      DISPATCH;                                                                \
    }                                                                          \
    runtime->storeCallerIP(ip);                                                \
    boolRes =                                                                  \
        operFuncName(runtime, Handle<>(&O2REG(name)), Handle<>(&O3REG(name))); \
    runtime->clearCallerIP();                                                  \
    if (boolRes == ExecutionStatus::EXCEPTION)                                 \
      goto exception;                                                          \
    gcScope.flushToSmallCount(KEEP_HANDLES);                                   \
    O1REG(name) = HermesValue::encodeBoolValue(boolRes.getValue());            \
    ip = NEXTINST(name);                                                       \
    DISPATCH;                                                                  \
  }

/// Implement a comparison conditional jump with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction. The fast path case will have a
///     "N" appended to the name.
/// \param suffix  Optional suffix to be added to the end (e.g. Long)
/// \param oper the C++ operator to use to actually perform the fast arithmetic
///     comparison.
/// \param operFuncName  function to call for the slow-path comparison.
/// \param trueDest  ip value if the conditional evaluates to true
/// \param falseDest  ip value if the conditional evaluates to false
#define JCOND_IMPL(name, suffix, oper, operFuncName, trueDest, falseDest) \
  CASE(name##suffix) {                                                    \
    if (LLVM_LIKELY(                                                      \
            O2REG(name##suffix).isNumber() &&                             \
            O3REG(name##suffix).isNumber())) {                            \
      /* Fast-path. */                                                    \
      CASE(name##N##suffix) {                                             \
        if (O2REG(name##N##suffix)                                        \
                .getNumber() oper O3REG(name##N##suffix)                  \
                .getNumber()) {                                           \
          ip = trueDest;                                                  \
          DISPATCH;                                                       \
        }                                                                 \
        ip = falseDest;                                                   \
        DISPATCH;                                                         \
      }                                                                   \
    }                                                                     \
    runtime->storeCallerIP(ip);                                           \
    boolRes = operFuncName(                                               \
        runtime,                                                          \
        Handle<>(&O2REG(name##suffix)),                                   \
        Handle<>(&O3REG(name##suffix)));                                  \
    runtime->clearCallerIP();                                             \
    if (boolRes == ExecutionStatus::EXCEPTION)                            \
      goto exception;                                                     \
    gcScope.flushToSmallCount(KEEP_HANDLES);                              \
    if (boolRes.getValue()) {                                             \
      ip = trueDest;                                                      \
      DISPATCH;                                                           \
    }                                                                     \
    ip = falseDest;                                                       \
    DISPATCH;                                                             \
  }

/// Implement a strict equality conditional jump
/// \param name the name of the instruction.
/// \param suffix  Optional suffix to be added to the end (e.g. Long)
/// \param trueDest  ip value if the conditional evaluates to true
/// \param falseDest  ip value if the conditional evaluates to false
#define JCOND_STRICT_EQ_IMPL(name, suffix, trueDest, falseDest)         \
  CASE(name##suffix) {                                                  \
    if (strictEqualityTest(O2REG(name##suffix), O3REG(name##suffix))) { \
      ip = trueDest;                                                    \
      DISPATCH;                                                         \
    }                                                                   \
    ip = falseDest;                                                     \
    DISPATCH;                                                           \
  }

/// Implement an equality conditional jump
/// \param name the name of the instruction.
/// \param suffix  Optional suffix to be added to the end (e.g. Long)
/// \param trueDest  ip value if the conditional evaluates to true
/// \param falseDest  ip value if the conditional evaluates to false
#define JCOND_EQ_IMPL(name, suffix, trueDest, falseDest) \
  CASE(name##suffix) {                                   \
    runtime->storeCallerIP(ip);                          \
    res = abstractEqualityTest_RJS(                      \
        runtime,                                         \
        Handle<>(&O2REG(name##suffix)),                  \
        Handle<>(&O3REG(name##suffix)));                 \
    runtime->clearCallerIP();                            \
    if (res == ExecutionStatus::EXCEPTION) {             \
      goto exception;                                    \
    }                                                    \
    gcScope.flushToSmallCount(KEEP_HANDLES);             \
    if (res->getBool()) {                                \
      ip = trueDest;                                     \
      DISPATCH;                                          \
    }                                                    \
    ip = falseDest;                                      \
    DISPATCH;                                            \
  }

/// Implement the long and short forms of a conditional jump, and its negation.
#define JCOND(name, oper, operFuncName) \
  JCOND_IMPL(                           \
      J##name,                          \
      ,                                 \
      oper,                             \
      operFuncName,                     \
      IPADD(ip->iJ##name.op1),          \
      NEXTINST(J##name));               \
  JCOND_IMPL(                           \
      J##name,                          \
      Long,                             \
      oper,                             \
      operFuncName,                     \
      IPADD(ip->iJ##name##Long.op1),    \
      NEXTINST(J##name##Long));         \
  JCOND_IMPL(                           \
      JNot##name,                       \
      ,                                 \
      oper,                             \
      operFuncName,                     \
      NEXTINST(JNot##name),             \
      IPADD(ip->iJNot##name.op1));      \
  JCOND_IMPL(                           \
      JNot##name,                       \
      Long,                             \
      oper,                             \
      operFuncName,                     \
      NEXTINST(JNot##name##Long),       \
      IPADD(ip->iJNot##name##Long.op1));

/// Load a constant.
/// \param value is the value to store in the output register.
#define LOAD_CONST(name, value) \
  CASE(name) {                  \
    O1REG(name) = value;        \
    ip = NEXTINST(name);        \
    DISPATCH;                   \
  }

      CASE(Mov) {
        O1REG(Mov) = O2REG(Mov);
        ip = NEXTINST(Mov);
        DISPATCH;
      }

      CASE(MovLong) {
        O1REG(MovLong) = O2REG(MovLong);
        ip = NEXTINST(MovLong);
        DISPATCH;
      }

      CASE(LoadParam) {
        if (LLVM_LIKELY(ip->iLoadParam.op2 <= FRAME.getArgCount())) {
          // index 0 must load 'this'. Index 1 the first argument, etc.
          O1REG(LoadParam) = FRAME.getArgRef((int32_t)ip->iLoadParam.op2 - 1);
          ip = NEXTINST(LoadParam);
          DISPATCH;
        }
        O1REG(LoadParam) = HermesValue::encodeUndefinedValue();
        ip = NEXTINST(LoadParam);
        DISPATCH;
      }

      CASE(LoadParamLong) {
        if (LLVM_LIKELY(ip->iLoadParamLong.op2 <= FRAME.getArgCount())) {
          // index 0 must load 'this'. Index 1 the first argument, etc.
          O1REG(LoadParamLong) =
              FRAME.getArgRef((int32_t)ip->iLoadParamLong.op2 - 1);
          ip = NEXTINST(LoadParamLong);
          DISPATCH;
        }
        O1REG(LoadParamLong) = HermesValue::encodeUndefinedValue();
        ip = NEXTINST(LoadParamLong);
        DISPATCH;
      }

      CASE(CoerceThisNS) {
        if (LLVM_LIKELY(O2REG(CoerceThisNS).isObject())) {
          O1REG(CoerceThisNS) = O2REG(CoerceThisNS);
        } else if (
            O2REG(CoerceThisNS).isNull() || O2REG(CoerceThisNS).isUndefined()) {
          O1REG(CoerceThisNS) = runtime->global_;
        } else {
          tmpHandle = O2REG(CoerceThisNS);
          nextIP = NEXTINST(CoerceThisNS);
          goto coerceThisSlowPath;
        }
        ip = NEXTINST(CoerceThisNS);
        DISPATCH;
      }
      CASE(LoadThisNS) {
        if (LLVM_LIKELY(FRAME.getThisArgRef().isObject())) {
          O1REG(LoadThisNS) = FRAME.getThisArgRef();
        } else if (
            FRAME.getThisArgRef().isNull() ||
            FRAME.getThisArgRef().isUndefined()) {
          O1REG(LoadThisNS) = runtime->global_;
        } else {
          tmpHandle = FRAME.getThisArgRef();
          nextIP = NEXTINST(LoadThisNS);
          goto coerceThisSlowPath;
        }
        ip = NEXTINST(LoadThisNS);
        DISPATCH;
      }
    coerceThisSlowPath : {
      res = toObject(runtime, tmpHandle);
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        goto exception;
      }
      O1REG(CoerceThisNS) = res.getValue();
      tmpHandle.clear();
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(ConstructLong) {
        callArgCount = (uint32_t)ip->iConstructLong.op3;
        nextIP = NEXTINST(ConstructLong);
        callNewTarget = O2REG(ConstructLong).getRaw();
        goto doCall;
      }
      CASE(CallLong) {
        callArgCount = (uint32_t)ip->iCallLong.op3;
        nextIP = NEXTINST(CallLong);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      // Note in Call1 through Call4, the first argument is 'this' which has
      // argument index -1.
      // Also note that we are writing to callNewTarget last, to avoid the
      // possibility of it being aliased by the arg writes.
      CASE(Call1) {
        callArgCount = 1;
        nextIP = NEXTINST(Call1);
        StackFramePtr fr{runtime->stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call1);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call2) {
        callArgCount = 2;
        nextIP = NEXTINST(Call2);
        StackFramePtr fr{runtime->stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call2);
        fr.getArgRefUnsafe(0) = O4REG(Call2);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call3) {
        callArgCount = 3;
        nextIP = NEXTINST(Call3);
        StackFramePtr fr{runtime->stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call3);
        fr.getArgRefUnsafe(0) = O4REG(Call3);
        fr.getArgRefUnsafe(1) = O5REG(Call3);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call4) {
        callArgCount = 4;
        nextIP = NEXTINST(Call4);
        StackFramePtr fr{runtime->stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call4);
        fr.getArgRefUnsafe(0) = O4REG(Call4);
        fr.getArgRefUnsafe(1) = O5REG(Call4);
        fr.getArgRefUnsafe(2) = O6REG(Call4);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Construct) {
        callArgCount = (uint32_t)ip->iConstruct.op3;
        nextIP = NEXTINST(Construct);
        callNewTarget = O2REG(Construct).getRaw();
        goto doCall;
      }
      CASE(Call) {
        callArgCount = (uint32_t)ip->iCall.op3;
        nextIP = NEXTINST(Call);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        // Fall through.
      }

    doCall : {
#ifdef HERMES_ENABLE_DEBUGGER
      // Check for an async debugger request.
      if (LLVM_UNLIKELY(runtime->testAndClearDebuggerAsyncBreakRequest())) {
        if (runDebuggerUpdatingState(
                Debugger::RunReason::AsyncBreak,
                runtime,
                curCodeBlock,
                ip,
                frameRegs) == ExecutionStatus::EXCEPTION)
          goto exception;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        DISPATCH;
      }
#endif
      runtime->storeCallerIP(ip);

      // Subtract 1 from callArgCount as 'this' is considered an argument in the
      // instruction, but not in the frame.
      auto newFrame = StackFramePtr::initFrame(
          runtime->stackPointer_,
          FRAME,
          ip,
          curCodeBlock,
          callArgCount - 1,
          O2REG(Call),
          HermesValue{callNewTarget});
      (void)newFrame;

      SLOW_DEBUG(dumpCallArguments(dbgs(), runtime, newFrame));

      if (auto *func = dyn_vmcast<JSFunction>(O2REG(Call))) {
        assert(!SingleStep && "can't single-step a call");

        CodeBlock *calleeBlock = func->getCodeBlock();
        calleeBlock->lazyCompile(runtime);
#if defined(HERMESVM_PROFILER_EXTERN)
        res = runtime->interpretFunction(calleeBlock);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(Call) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = nextIP;
        DISPATCH;
#else
        if (auto jitPtr = runtime->jitContext_.compile(runtime, calleeBlock)) {
          res = (*jitPtr)(runtime);
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
            goto exception;
          O1REG(Call) = *res;
          SLOW_DEBUG(
              dbgs() << "JIT return value r" << (unsigned)ip->iCall.op1 << "="
                     << DumpHermesValue(O1REG(Call)) << "\n");
          gcScope.flushToSmallCount(KEEP_HANDLES);
          ip = nextIP;
          DISPATCH;
        }
        curCodeBlock = calleeBlock;
        goto tailCall;
#endif
      }
      res = Interpreter::handleCallSlowPath(runtime, &O2REG(Call));
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        goto exception;
      }
      O1REG(Call) = *res;
      SLOW_DEBUG(
          dbgs() << "native return value r" << (unsigned)ip->iCall.op1 << "="
                 << DumpHermesValue(O1REG(Call)) << "\n");
      runtime->clearCallerIP();
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CallDirect)
      CASE(CallDirectLongIndex) {
#ifdef HERMES_ENABLE_DEBUGGER
        // Check for an async debugger request.
        if (LLVM_UNLIKELY(runtime->testAndClearDebuggerAsyncBreakRequest())) {
          if (runDebuggerUpdatingState(
                  Debugger::RunReason::AsyncBreak,
                  runtime,
                  curCodeBlock,
                  ip,
                  frameRegs) == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          DISPATCH;
        }
#endif
        runtime->storeCallerIP(ip);

        CodeBlock *calleeBlock = ip->opCode == OpCode::CallDirect
            ? curCodeBlock->getRuntimeModule()->getCodeBlockMayAllocate(
                  ip->iCallDirect.op3)
            : curCodeBlock->getRuntimeModule()->getCodeBlockMayAllocate(
                  ip->iCallDirectLongIndex.op3);

        auto newFrame = StackFramePtr::initFrame(
            runtime->stackPointer_,
            FRAME,
            ip,
            curCodeBlock,
            (uint32_t)ip->iCallDirect.op2 - 1,
            HermesValue::encodeNativePointer(calleeBlock),
            HermesValue::encodeUndefinedValue());
        (void)newFrame;

        LLVM_DEBUG(dumpCallArguments(dbgs(), runtime, newFrame));

        assert(!SingleStep && "can't single-step a call");

        calleeBlock->lazyCompile(runtime);
#if defined(HERMESVM_PROFILER_EXTERN)
        res = runtime->interpretFunction(calleeBlock);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CallDirect) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = ip->opCode == OpCode::CallDirect ? NEXTINST(CallDirect)
                                              : NEXTINST(CallDirectLongIndex);
        DISPATCH;
#else
        if (auto jitPtr = runtime->jitContext_.compile(runtime, calleeBlock)) {
          res = (*jitPtr)(runtime);
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
            goto exception;
          O1REG(CallDirect) = *res;
          LLVM_DEBUG(
              dbgs() << "JIT return value r" << (unsigned)ip->iCallDirect.op1
                     << "=" << DumpHermesValue(O1REG(Call)) << "\n");
          gcScope.flushToSmallCount(KEEP_HANDLES);
          ip = ip->opCode == OpCode::CallDirect ? NEXTINST(CallDirect)
                                                : NEXTINST(CallDirectLongIndex);
          DISPATCH;
        }
        curCodeBlock = calleeBlock;
        goto tailCall;
#endif
      }

      CASE(CallBuiltin) {
        NativeFunction *nf =
            runtime->getBuiltinNativeFunction(ip->iCallBuiltin.op2);

        auto newFrame = StackFramePtr::initFrame(
            runtime->stackPointer_,
            FRAME,
            ip,
            curCodeBlock,
            (uint32_t)ip->iCallBuiltin.op3 - 1,
            nf,
            false);
        (void)newFrame;

        SLOW_DEBUG(dumpCallArguments(dbgs(), runtime, newFrame));

        runtime->storeCallerIP(ip);
        res = NativeFunction::_nativeCall(nf, runtime);
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
          goto exception;
        O1REG(CallBuiltin) = *res;
        SLOW_DEBUG(
            dbgs() << "native return value r" << (unsigned)ip->iCallBuiltin.op1
                   << "=" << DumpHermesValue(O1REG(CallBuiltin)) << "\n");
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CallBuiltin);
        DISPATCH;
      }

      CASE(CompleteGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime->getCurrentFrame().getCalleeClosure());
        innerFn->setState(GeneratorInnerFunction::State::Completed);
        ip = NEXTINST(CompleteGenerator);
        DISPATCH;
      }

      CASE(SaveGenerator) {
        nextIP = IPADD(ip->iSaveGenerator.op1);
        goto doSaveGen;
      }
      CASE(SaveGeneratorLong) {
        nextIP = IPADD(ip->iSaveGeneratorLong.op1);
        goto doSaveGen;
      }

    doSaveGen : {
      auto *innerFn = vmcast<GeneratorInnerFunction>(
          runtime->getCurrentFrame().getCalleeClosure());

      innerFn->saveStack(runtime);
      innerFn->setNextIP(nextIP);
      innerFn->setState(GeneratorInnerFunction::State::SuspendedYield);
      ip = NEXTINST(SaveGenerator);
      DISPATCH;
    }

      CASE(StartGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime->getCurrentFrame().getCalleeClosure());
        if (innerFn->getState() ==
            GeneratorInnerFunction::State::SuspendedStart) {
          nextIP = NEXTINST(StartGenerator);
        } else {
          nextIP = innerFn->getNextIP();
          innerFn->restoreStack(runtime);
        }
        innerFn->setState(GeneratorInnerFunction::State::Executing);
        ip = nextIP;
        DISPATCH;
      }

      CASE(ResumeGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime->getCurrentFrame().getCalleeClosure());
        O1REG(ResumeGenerator) = innerFn->getResult();
        O2REG(ResumeGenerator) = HermesValue::encodeBoolValue(
            innerFn->getAction() == GeneratorInnerFunction::Action::Return);
        innerFn->clearResult();
        if (innerFn->getAction() == GeneratorInnerFunction::Action::Throw) {
          runtime->setThrownValue(O1REG(ResumeGenerator));
          goto exception;
        }
        ip = NEXTINST(ResumeGenerator);
        DISPATCH;
      }

      CASE(Ret) {
#ifdef HERMES_ENABLE_DEBUGGER
        // Check for an async debugger request.
        if (LLVM_UNLIKELY(runtime->testAndClearDebuggerAsyncBreakRequest())) {
          if (runDebuggerUpdatingState(
                  Debugger::RunReason::AsyncBreak,
                  runtime,
                  curCodeBlock,
                  ip,
                  frameRegs) == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          DISPATCH;
        }
#endif
        runtime->restoreCallerIPFromStackFrame();

        PROFILER_EXIT_FUNCTION(curCodeBlock);

        // Store the return value.
        res = O1REG(Ret);

        ip = FRAME.getSavedIP();
        curCodeBlock = FRAME.getSavedCodeBlock();

        frameRegs =
            &runtime->restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

        SLOW_DEBUG(
            dbgs() << "function exit: restored stackLevel="
                   << runtime->getStackLevel() << "\n");

        // Are we returning to native code?
        if (!curCodeBlock) {
          SLOW_DEBUG(dbgs() << "function exit: returning to native code\n");
          return res;
        }

// Return because of recursive calling structure
#if defined(HERMESVM_PROFILER_EXTERN)
        return res;
#endif

        INIT_STATE_FOR_CODEBLOCK(curCodeBlock);
        O1REG(Call) = res.getValue();
        ip = nextInstCall(ip);
        DISPATCH;
      }

      CASE(Catch) {
        assert(!runtime->thrownValue_.isEmpty() && "Invalid thrown value");
        assert(
            !isUncatchableError(runtime->thrownValue_) &&
            "Uncatchable thrown value was caught");
        O1REG(Catch) = runtime->thrownValue_;
        runtime->clearThrownValue();
#ifdef HERMES_ENABLE_DEBUGGER
        // Signal to the debugger that we're done unwinding an exception,
        // and we can resume normal debugging flow.
        runtime->debugger_.finishedUnwindingException();
#endif
        ip = NEXTINST(Catch);
        DISPATCH;
      }

      CASE(Throw) {
        runtime->thrownValue_ = O1REG(Throw);
        SLOW_DEBUG(
            dbgs() << "Exception thrown: "
                   << DumpHermesValue(runtime->thrownValue_) << "\n");
        goto exception;
      }

      CASE(ThrowIfUndefinedInst) {
        if (LLVM_UNLIKELY(O1REG(ThrowIfUndefinedInst).isUndefined())) {
          SLOW_DEBUG(
              dbgs() << "Throwing ReferenceError for undefined variable");
          runtime->raiseReferenceError("accessing an uninitialized variable");
          goto exception;
        }
        ip = NEXTINST(ThrowIfUndefinedInst);
        DISPATCH;
      }

      CASE(Debugger) {
        SLOW_DEBUG(dbgs() << "debugger statement executed\n");
#ifdef HERMES_ENABLE_DEBUGGER
        {
          if (!runtime->debugger_.isDebugging()) {
            // Only run the debugger if we're not already debugging.
            // Don't want to call it again and mess with its state.
            if (runDebuggerUpdatingState(
                    Debugger::RunReason::Opcode,
                    runtime,
                    curCodeBlock,
                    ip,
                    frameRegs) == ExecutionStatus::EXCEPTION) {
              // If one of the internal steps threw,
              // then handle that here by jumping to where we're supposed to go.
              // If we're in mid-step, the breakpoint at the catch point
              // will have been set by the debugger.
              // We don't want to execute this instruction because it's already
              // thrown.
              goto exception;
            }
          }
          auto breakpointOpt = runtime->debugger_.getBreakpointLocation(ip);
          if (breakpointOpt.hasValue()) {
            // We're on a breakpoint but we're supposed to continue.
            curCodeBlock->uninstallBreakpointAtOffset(
                CUROFFSET, breakpointOpt->opCode);
            if (ip->opCode == OpCode::Debugger) {
              // Breakpointed a debugger instruction, so move past it
              // since we've already called the debugger on this instruction.
              ip = NEXTINST(Debugger);
            } else {
              InterpreterState newState{curCodeBlock, (uint32_t)CUROFFSET};
              ExecutionStatus status = runtime->stepFunction(newState);
              curCodeBlock->installBreakpointAtOffset(CUROFFSET);
              if (status == ExecutionStatus::EXCEPTION) {
                goto exception;
              }
              curCodeBlock = newState.codeBlock;
              ip = newState.codeBlock->getOffsetPtr(newState.offset);
              INIT_STATE_FOR_CODEBLOCK(curCodeBlock);
              // Single-stepping should handle call stack management for us.
              frameRegs = &runtime->getCurrentFrame().getFirstLocalRef();
            }
          } else if (ip->opCode == OpCode::Debugger) {
            // No breakpoint here and we've already run the debugger,
            // just continue on.
            // If the current instruction is no longer a debugger instruction,
            // we're just going to keep executing from the current IP.
            ip = NEXTINST(Debugger);
          }
          gcScope.flushToSmallCount(KEEP_HANDLES);
        }
        DISPATCH;
#else
        ip = NEXTINST(Debugger);
        DISPATCH;
#endif
      }

      CASE(AsyncBreakCheck) {
        if (LLVM_UNLIKELY(runtime->hasAsyncBreak())) {
#ifdef HERMES_ENABLE_DEBUGGER
          if (runtime->testAndClearDebuggerAsyncBreakRequest()) {
            if (runDebuggerUpdatingState(
                    Debugger::RunReason::AsyncBreak,
                    runtime,
                    curCodeBlock,
                    ip,
                    frameRegs) == ExecutionStatus::EXCEPTION)
              goto exception;
          }
#endif
          if (runtime->testAndClearTimeoutAsyncBreakRequest()) {
            if (runtime->notifyTimeout() == ExecutionStatus::EXCEPTION) {
              goto exception;
            }
          }
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);

        ip = NEXTINST(AsyncBreakCheck);
        DISPATCH;
      }

      CASE(ProfilePoint) {
#ifdef HERMESVM_PROFILER_BB
        auto pointIndex = ip->iProfilePoint.op1;
        SLOW_DEBUG(llvm::dbgs() << "ProfilePoint: " << pointIndex << "\n");
        runtime->getBasicBlockExecutionInfo().executeBlock(
            curCodeBlock, pointIndex);
#endif
        ip = NEXTINST(ProfilePoint);
        DISPATCH;
      }

      CASE(Unreachable) {
        llvm_unreachable("Hermes bug: unreachable instruction");
      }

      CASE(CreateClosure) {
        idVal = ip->iCreateClosure.op3;
        nextIP = NEXTINST(CreateClosure);
        goto createClosure;
      }
      CASE(CreateClosureLongIndex) {
        idVal = ip->iCreateClosureLongIndex.op3;
        nextIP = NEXTINST(CreateClosureLongIndex);
        goto createClosure;
      }
    createClosure : {
      auto *runtimeModule = curCodeBlock->getRuntimeModule();
      res = JSFunction::create(
          runtime,
          runtimeModule->getDomain(runtime),
          Handle<JSObject>::vmcast(&runtime->functionPrototype),
          Handle<Environment>::vmcast(&O2REG(CreateClosure)),
          runtimeModule->getCodeBlockMayAllocate(idVal));
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        goto exception;
      }
      O1REG(CreateClosure) = *res;
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CreateGeneratorClosure) {
        res = createGeneratorClosure(
            runtime,
            curCodeBlock->getRuntimeModule(),
            ip->iCreateClosure.op3,
            Handle<Environment>::vmcast(&O2REG(CreateGeneratorClosure)));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGeneratorClosure) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateGeneratorClosure);
        DISPATCH;
      }
      CASE(CreateGeneratorClosureLongIndex) {
        res = createGeneratorClosure(
            runtime,
            curCodeBlock->getRuntimeModule(),
            ip->iCreateClosureLongIndex.op3,
            Handle<Environment>::vmcast(
                &O2REG(CreateGeneratorClosureLongIndex)));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGeneratorClosureLongIndex) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateGeneratorClosureLongIndex);
        DISPATCH;
      }

      CASE(CreateGenerator) {
        runtime->storeCallerIP(ip);
        res = createGenerator_RJS(
            runtime,
            curCodeBlock->getRuntimeModule(),
            ip->iCreateGenerator.op3,
            Handle<Environment>::vmcast(&O2REG(CreateGenerator)),
            FRAME.getNativeArgs());
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGenerator) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateGenerator);
        DISPATCH;
      }
      CASE(CreateGeneratorLongIndex) {
        runtime->storeCallerIP(ip);
        res = createGenerator_RJS(
            runtime,
            curCodeBlock->getRuntimeModule(),
            ip->iCreateGenerator.op3,
            Handle<Environment>::vmcast(&O2REG(CreateGeneratorLongIndex)),
            FRAME.getNativeArgs());
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGenerator) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateGeneratorLongIndex);
        DISPATCH;
      }

      CASE(GetEnvironment) {
        // The currently executing function must exist, so get the environment.
        Environment *curEnv =
            FRAME.getCalleeClosureUnsafe()->getEnvironment(runtime);
        for (unsigned level = ip->iGetEnvironment.op2; level; --level) {
          assert(curEnv && "invalid environment relative level");
          curEnv = curEnv->getParentEnvironment(runtime);
        }
        O1REG(GetEnvironment) = HermesValue::encodeObjectValue(curEnv);
        ip = NEXTINST(GetEnvironment);
        DISPATCH;
      }

      CASE(CreateEnvironment) {
        tmpHandle = HermesValue::encodeObjectValue(
            FRAME.getCalleeClosureUnsafe()->getEnvironment(runtime));

        res = Environment::create(
            runtime,
            tmpHandle->getPointer()
                ? Handle<Environment>::vmcast(tmpHandle)
                : Handle<Environment>::vmcast_or_null(&runtime->nullPointer_),
            curCodeBlock->getEnvironmentSize());
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        O1REG(CreateEnvironment) = *res;
#ifdef HERMES_ENABLE_DEBUGGER
        FRAME.getDebugEnvironmentRef() = *res;
#endif
        tmpHandle = HermesValue::encodeUndefinedValue();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateEnvironment);
        DISPATCH;
      }

      CASE(StoreToEnvironment) {
        vmcast<Environment>(O1REG(StoreToEnvironment))
            ->slot(ip->iStoreToEnvironment.op2)
            .set(O3REG(StoreToEnvironment), &runtime->getHeap());
        ip = NEXTINST(StoreToEnvironment);
        DISPATCH;
      }
      CASE(StoreToEnvironmentL) {
        vmcast<Environment>(O1REG(StoreToEnvironmentL))
            ->slot(ip->iStoreToEnvironmentL.op2)
            .set(O3REG(StoreToEnvironmentL), &runtime->getHeap());
        ip = NEXTINST(StoreToEnvironmentL);
        DISPATCH;
      }

      CASE(StoreNPToEnvironment) {
        vmcast<Environment>(O1REG(StoreNPToEnvironment))
            ->slot(ip->iStoreNPToEnvironment.op2)
            .setNonPtr(O3REG(StoreNPToEnvironment));
        ip = NEXTINST(StoreNPToEnvironment);
        DISPATCH;
      }
      CASE(StoreNPToEnvironmentL) {
        vmcast<Environment>(O1REG(StoreNPToEnvironmentL))
            ->slot(ip->iStoreNPToEnvironmentL.op2)
            .setNonPtr(O3REG(StoreNPToEnvironmentL));
        ip = NEXTINST(StoreNPToEnvironmentL);
        DISPATCH;
      }

      CASE(LoadFromEnvironment) {
        O1REG(LoadFromEnvironment) =
            vmcast<Environment>(O2REG(LoadFromEnvironment))
                ->slot(ip->iLoadFromEnvironment.op3);
        ip = NEXTINST(LoadFromEnvironment);
        DISPATCH;
      }

      CASE(LoadFromEnvironmentL) {
        O1REG(LoadFromEnvironmentL) =
            vmcast<Environment>(O2REG(LoadFromEnvironmentL))
                ->slot(ip->iLoadFromEnvironmentL.op3);
        ip = NEXTINST(LoadFromEnvironmentL);
        DISPATCH;
      }

      CASE(GetGlobalObject) {
        O1REG(GetGlobalObject) = runtime->global_;
        ip = NEXTINST(GetGlobalObject);
        DISPATCH;
      }

      CASE(GetNewTarget) {
        O1REG(GetNewTarget) = FRAME.getNewTargetRef();
        ip = NEXTINST(GetNewTarget);
        DISPATCH;
      }

      CASE(DeclareGlobalVar) {
        DefinePropertyFlags dpf{};
        dpf.setWritable = 1;
        dpf.setConfigurable = 1;
        dpf.setEnumerable = 1;
        dpf.writable = 1;
        dpf.enumerable = 1;
        dpf.configurable = 0;

        if (JSObject::defineOwnProperty(
                runtime->getGlobal(),
                runtime,
                ID(ip->iDeclareGlobalVar.op1),
                dpf,
                Runtime::getUndefinedValue(),
                PropOpFlags().plusThrowOnError()) ==
            ExecutionStatus::EXCEPTION) {
          // If the property already exists, this should be a noop.
          // Instead of incurring the cost to check every time, do it
          // only if an exception is thrown, and swallow the exception
          // if it exists, since we didn't want to make the call,
          // anyway.  This most likely means the property is
          // non-configurable.
          NamedPropertyDescriptor desc;
          if (!JSObject::getOwnNamedDescriptor(
                  runtime->getGlobal(),
                  runtime,
                  ID(ip->iDeclareGlobalVar.op1),
                  desc)) {
            goto exception;
          } else {
            runtime->clearThrownValue();
          }
          // fall through
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(DeclareGlobalVar);
        DISPATCH;
      }

      CASE(TryGetByIdLong) {
        tryProp = true;
        idVal = ip->iTryGetByIdLong.op4;
        nextIP = NEXTINST(TryGetByIdLong);
        goto getById;
      }
      CASE(GetByIdLong) {
        tryProp = false;
        idVal = ip->iGetByIdLong.op4;
        nextIP = NEXTINST(GetByIdLong);
        goto getById;
      }
      CASE(GetByIdShort) {
        tryProp = false;
        idVal = ip->iGetByIdShort.op4;
        nextIP = NEXTINST(GetByIdShort);
        goto getById;
      }
      CASE(TryGetById) {
        tryProp = true;
        idVal = ip->iTryGetById.op4;
        nextIP = NEXTINST(TryGetById);
        goto getById;
      }
      CASE(GetById) {
        tryProp = false;
        idVal = ip->iGetById.op4;
        nextIP = NEXTINST(GetById);
      }
    getById : {
      ++NumGetById;
      // NOTE: it is safe to use OnREG(GetById) here because all instructions
      // have the same layout: opcode, registers, non-register operands, i.e.
      // they only differ in the width of the last "identifier" field.
      CallResult<HermesValue> propRes{ExecutionStatus::EXCEPTION};
      if (LLVM_LIKELY(O2REG(GetById).isObject())) {
        auto *obj = vmcast<JSObject>(O2REG(GetById));
        auto *clazz = obj->getClass(runtime);
        auto cacheIdx = ip->iGetById.op3;
        auto *cacheEntry = curCodeBlock->getReadCacheEntry(cacheIdx);

#ifdef HERMESVM_PROFILER_BB
        {
          HERMES_SLOW_ASSERT(
              gcScope.getHandleCountDbg() == KEEP_HANDLES &&
              "unaccounted handles were created");
          auto objHandle = runtime->makeHandle(obj);
          runtime->recordHiddenClass(
              curCodeBlock, ip, ID(idVal), clazz, cacheEntry->clazz);
          // obj may be moved by GC due to recordHiddenClass
          obj = objHandle.get();
          clazz = obj->getClass(runtime);
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
        // If we have a cache hit, reuse the cached offset and immediately
        // return the property.
        if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
          ++NumGetByIdCacheHits;
          O1REG(GetById) =
              JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(
                  obj, runtime, cacheEntry->slot);
          ip = nextIP;
          DISPATCH;
        }
        auto id = ID(idVal);
        NamedPropertyDescriptor desc;
        OptValue<bool> fastPathResult =
            JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc);
        if (LLVM_LIKELY(
                fastPathResult.hasValue() && fastPathResult.getValue()) &&
            !desc.flags.accessor) {
          ++NumGetByIdFastPaths;

          // cacheIdx == 0 indicates no caching so don't update the cache in
          // those cases.
          if (LLVM_LIKELY(!clazz->isDictionaryNoCache()) &&
              LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
#ifdef HERMES_SLOW_DEBUG
            if (cacheEntry->clazz && cacheEntry->clazz != clazz)
              ++NumGetByIdCacheEvicts;
#else
            (void)NumGetByIdCacheEvicts;
#endif
            // Cache the class, id and property slot.
            cacheEntry->clazz = clazz;
            cacheEntry->slot = desc.slot;
          }

          O1REG(GetById) = JSObject::getNamedSlotValue(obj, runtime, desc);
          ip = nextIP;
          DISPATCH;
        }

        // The cache may also be populated via the prototype of the object.
        // This value is only reliable if the fast path was a definite
        // not-found.
        if (fastPathResult.hasValue() && !fastPathResult.getValue()) {
          JSObject *parent = obj->getParent(runtime);
          // TODO: This isLazy check is because a lazy object is reported as
          // having no properties and therefore cannot contain the property.
          // This check does not belong here, it should be merged into
          // tryGetOwnNamedDescriptorFast().
          if (parent && cacheEntry->clazz == parent->getClass(runtime) &&
              LLVM_LIKELY(!obj->isLazy())) {
            ++NumGetByIdProtoHits;
            O1REG(GetById) =
                JSObject::getNamedSlotValue(parent, runtime, cacheEntry->slot);
            ip = nextIP;
            DISPATCH;
          }
        }

#ifdef HERMES_SLOW_DEBUG
        JSObject *propObj = JSObject::getNamedDescriptor(
            Handle<JSObject>::vmcast(&O2REG(GetById)), runtime, id, desc);
        if (propObj) {
          if (desc.flags.accessor)
            ++NumGetByIdAccessor;
          else if (propObj != vmcast<JSObject>(O2REG(GetById)))
            ++NumGetByIdProto;
        } else {
          ++NumGetByIdNotFound;
        }
#else
        (void)NumGetByIdAccessor;
        (void)NumGetByIdProto;
        (void)NumGetByIdNotFound;
#endif
        runtime->storeCallerIP(ip);
        propRes = JSObject::getNamed_RJS(
            Handle<JSObject>::vmcast(&O2REG(GetById)),
            runtime,
            id,
            !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist(),
            cacheIdx != hbc::PROPERTY_CACHING_DISABLED ? cacheEntry : nullptr);
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
      } else {
        ++NumGetByIdTransient;
        assert(!tryProp && "TryGetById can only be used on the global object");
        /* Slow path. */
        runtime->storeCallerIP(ip);
        propRes = Interpreter::getByIdTransient_RJS(
            runtime, Handle<>(&O2REG(GetById)), ID(idVal));
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
      }
      O1REG(GetById) = *propRes;
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(TryPutByIdLong) {
        tryProp = true;
        idVal = ip->iTryPutByIdLong.op4;
        nextIP = NEXTINST(TryPutByIdLong);
        goto putById;
      }
      CASE(PutByIdLong) {
        tryProp = false;
        idVal = ip->iPutByIdLong.op4;
        nextIP = NEXTINST(PutByIdLong);
        goto putById;
      }
      CASE(TryPutById) {
        tryProp = true;
        idVal = ip->iTryPutById.op4;
        nextIP = NEXTINST(TryPutById);
        goto putById;
      }
      CASE(PutById) {
        tryProp = false;
        idVal = ip->iPutById.op4;
        nextIP = NEXTINST(PutById);
      }
    putById : {
      ++NumPutById;
      if (LLVM_LIKELY(O1REG(PutById).isObject())) {
        auto *obj = vmcast<JSObject>(O1REG(PutById));
        auto *clazz = obj->getClass(runtime);
        auto cacheIdx = ip->iPutById.op3;
        auto *cacheEntry = curCodeBlock->getWriteCacheEntry(cacheIdx);

#ifdef HERMESVM_PROFILER_BB
        {
          HERMES_SLOW_ASSERT(
              gcScope.getHandleCountDbg() == KEEP_HANDLES &&
              "unaccounted handles were created");
          auto objHandle = runtime->makeHandle(obj);
          runtime->recordHiddenClass(
              curCodeBlock, ip, ID(idVal), clazz, cacheEntry->clazz);
          // obj may be moved by GC due to recordHiddenClass
          obj = objHandle.get();
          clazz = obj->getClass(runtime);
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
        // If we have a cache hit, reuse the cached offset and immediately
        // return the property.
        if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
          ++NumPutByIdCacheHits;
          JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(
              obj, runtime, cacheEntry->slot, O2REG(PutById));
          ip = nextIP;
          DISPATCH;
        }
        auto id = ID(idVal);
        NamedPropertyDescriptor desc;
        OptValue<bool> hasOwnProp =
            JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc);
        if (LLVM_LIKELY(hasOwnProp.hasValue() && hasOwnProp.getValue()) &&
            !desc.flags.accessor && desc.flags.writable &&
            !desc.flags.internalSetter) {
          ++NumPutByIdFastPaths;

          // cacheIdx == 0 indicates no caching so don't update the cache in
          // those cases.
          if (LLVM_LIKELY(!clazz->isDictionary()) &&
              LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
#ifdef HERMES_SLOW_DEBUG
            if (cacheEntry->clazz && cacheEntry->clazz != clazz)
              ++NumPutByIdCacheEvicts;
#else
            (void)NumPutByIdCacheEvicts;
#endif
            // Cache the class and property slot.
            cacheEntry->clazz = clazz;
            cacheEntry->slot = desc.slot;
          }

          JSObject::setNamedSlotValue(obj, runtime, desc.slot, O2REG(PutById));
          ip = nextIP;
          DISPATCH;
        }

        runtime->storeCallerIP(ip);
        auto putRes = JSObject::putNamed_RJS(
            Handle<JSObject>::vmcast(&O1REG(PutById)),
            runtime,
            id,
            Handle<>(&O2REG(PutById)),
            !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist());
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
      } else {
        ++NumPutByIdTransient;
        assert(!tryProp && "TryPutById can only be used on the global object");
        runtime->storeCallerIP(ip);
        auto retStatus = Interpreter::putByIdTransient_RJS(
            runtime,
            Handle<>(&O1REG(PutById)),
            ID(idVal),
            Handle<>(&O2REG(PutById)),
            strictMode);
        runtime->clearCallerIP();
        if (retStatus == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
      }
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(GetByVal) {
        CallResult<HermesValue> propRes{ExecutionStatus::EXCEPTION};
        if (LLVM_LIKELY(O2REG(GetByVal).isObject())) {
          runtime->storeCallerIP(ip);
          propRes = JSObject::getComputed_RJS(
              Handle<JSObject>::vmcast(&O2REG(GetByVal)),
              runtime,
              Handle<>(&O3REG(GetByVal)));
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        } else {
          // This is the "slow path".
          runtime->storeCallerIP(ip);
          propRes = Interpreter::getByValTransient_RJS(
              runtime, Handle<>(&O2REG(GetByVal)), Handle<>(&O3REG(GetByVal)));
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetByVal) = *propRes;
        ip = NEXTINST(GetByVal);
        DISPATCH;
      }

      CASE(PutByVal) {
        if (LLVM_LIKELY(O1REG(PutByVal).isObject())) {
          runtime->storeCallerIP(ip);
          auto putRes = JSObject::putComputed_RJS(
              Handle<JSObject>::vmcast(&O1REG(PutByVal)),
              runtime,
              Handle<>(&O2REG(PutByVal)),
              Handle<>(&O3REG(PutByVal)),
              defaultPropOpFlags);
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        } else {
          // This is the "slow path".
          runtime->storeCallerIP(ip);
          auto retStatus = Interpreter::putByValTransient_RJS(
              runtime,
              Handle<>(&O1REG(PutByVal)),
              Handle<>(&O2REG(PutByVal)),
              Handle<>(&O3REG(PutByVal)),
              strictMode);
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(retStatus == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(PutByVal);
        DISPATCH;
      }

      CASE(PutOwnByIndexL) {
        nextIP = NEXTINST(PutOwnByIndexL);
        idVal = ip->iPutOwnByIndexL.op3;
        goto putOwnByIndex;
      }
      CASE(PutOwnByIndex) {
        nextIP = NEXTINST(PutOwnByIndex);
        idVal = ip->iPutOwnByIndex.op3;
      }
    putOwnByIndex : {
      tmpHandle = HermesValue::encodeDoubleValue(idVal);
      (void)JSObject::defineOwnComputedPrimitive(
          Handle<JSObject>::vmcast(&O1REG(PutOwnByIndex)),
          runtime,
          tmpHandle,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          Handle<>(&O2REG(PutOwnByIndex)));
      gcScope.flushToSmallCount(KEEP_HANDLES);
      tmpHandle.clear();
      ip = nextIP;
      DISPATCH;
    }

      CASE(GetPNameList) {
        if (LLVM_UNLIKELY(
                handleGetPNameList(runtime, frameRegs, ip) ==
                ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(GetPNameList);
        DISPATCH;
      }

      CASE(GetNextPName) {
        {
          assert(
              vmisa<BigStorage>(O2REG(GetNextPName)) &&
              "GetNextPName's second op must be BigStorage");
          auto obj = Handle<JSObject>::vmcast(&O3REG(GetNextPName));
          auto arr = Handle<BigStorage>::vmcast(&O2REG(GetNextPName));
          uint32_t idx = O4REG(GetNextPName).getNumber();
          uint32_t size = O5REG(GetNextPName).getNumber();
          MutableHandle<JSObject> propObj{runtime};
          // Loop until we find a property which is present.
          while (idx < size) {
            tmpHandle = arr->at(idx);
            ComputedPropertyDescriptor desc;
            JSObject::getComputedPrimitiveDescriptor(
                obj, runtime, tmpHandle, propObj, desc);
            if (LLVM_LIKELY(propObj))
              break;
            ++idx;
          }
          if (idx < size) {
            // We must return the property as a string
            if (tmpHandle->isNumber()) {
              runtime->storeCallerIP(ip);
              auto status = toString_RJS(runtime, tmpHandle);
              runtime->clearCallerIP();
              assert(
                  status == ExecutionStatus::RETURNED &&
                  "toString on number cannot fail");
              tmpHandle = status->getHermesValue();
            }
            O1REG(GetNextPName) = tmpHandle.get();
            O4REG(GetNextPName) = HermesValue::encodeNumberValue(idx + 1);
          } else {
            O1REG(GetNextPName) = HermesValue::encodeUndefinedValue();
          }
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(GetNextPName);
        DISPATCH;
      }

      CASE(ToNumber) {
        if (LLVM_LIKELY(O2REG(ToNumber).isNumber())) {
          O1REG(ToNumber) = O2REG(ToNumber);
          ip = NEXTINST(ToNumber);
        } else {
          runtime->storeCallerIP(ip);
          res = toNumber_RJS(runtime, Handle<>(&O2REG(ToNumber)));
          runtime->clearCallerIP();
          if (res == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          O1REG(ToNumber) = res.getValue();
          ip = NEXTINST(ToNumber);
        }
        DISPATCH;
      }

      CASE(ToInt32) {
        runtime->storeCallerIP(ip);
        res = toInt32_RJS(runtime, Handle<>(&O2REG(ToInt32)));
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
          goto exception;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(ToInt32) = res.getValue();
        ip = NEXTINST(ToInt32);
        DISPATCH;
      }

      CASE(AddEmptyString) {
        if (LLVM_LIKELY(O2REG(AddEmptyString).isString())) {
          O1REG(AddEmptyString) = O2REG(AddEmptyString);
          ip = NEXTINST(AddEmptyString);
        } else {
          runtime->storeCallerIP(ip);
          res = toPrimitive_RJS(
              runtime, Handle<>(&O2REG(AddEmptyString)), PreferredType::NONE);
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
            goto exception;
          tmpHandle = res.getValue();
          runtime->storeCallerIP(ip);
          auto strRes = toString_RJS(runtime, tmpHandle);
          runtime->clearCallerIP();
          if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION))
            goto exception;
          tmpHandle.clear();
          gcScope.flushToSmallCount(KEEP_HANDLES);
          O1REG(AddEmptyString) = strRes->getHermesValue();
          ip = NEXTINST(AddEmptyString);
        }
        DISPATCH;
      }

      CASE(Jmp) {
        ip = IPADD(ip->iJmp.op1);
        DISPATCH;
      }
      CASE(JmpLong) {
        ip = IPADD(ip->iJmpLong.op1);
        DISPATCH;
      }
      CASE(JmpTrue) {
        if (toBoolean(O2REG(JmpTrue)))
          ip = IPADD(ip->iJmpTrue.op1);
        else
          ip = NEXTINST(JmpTrue);
        DISPATCH;
      }
      CASE(JmpTrueLong) {
        if (toBoolean(O2REG(JmpTrueLong)))
          ip = IPADD(ip->iJmpTrueLong.op1);
        else
          ip = NEXTINST(JmpTrueLong);
        DISPATCH;
      }
      CASE(JmpFalse) {
        if (!toBoolean(O2REG(JmpFalse)))
          ip = IPADD(ip->iJmpFalse.op1);
        else
          ip = NEXTINST(JmpFalse);
        DISPATCH;
      }
      CASE(JmpFalseLong) {
        if (!toBoolean(O2REG(JmpFalseLong)))
          ip = IPADD(ip->iJmpFalseLong.op1);
        else
          ip = NEXTINST(JmpFalseLong);
        DISPATCH;
      }
      CASE(JmpUndefined) {
        if (O2REG(JmpUndefined).isUndefined())
          ip = IPADD(ip->iJmpUndefined.op1);
        else
          ip = NEXTINST(JmpUndefined);
        DISPATCH;
      }
      CASE(JmpUndefinedLong) {
        if (O2REG(JmpUndefinedLong).isUndefined())
          ip = IPADD(ip->iJmpUndefinedLong.op1);
        else
          ip = NEXTINST(JmpUndefinedLong);
        DISPATCH;
      }
      CASE(Add) {
        if (LLVM_LIKELY(
                O2REG(Add).isNumber() &&
                O3REG(Add).isNumber())) { /* Fast-path. */
          CASE(AddN) {
            O1REG(Add) = HermesValue::encodeDoubleValue(
                O2REG(Add).getNumber() + O3REG(Add).getNumber());
            ip = NEXTINST(Add);
            DISPATCH;
          }
        }
        runtime->storeCallerIP(ip);
        res = addOp_RJS(runtime, Handle<>(&O2REG(Add)), Handle<>(&O3REG(Add)));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(Add) = res.getValue();
        ip = NEXTINST(Add);
        DISPATCH;
      }

      CASE(BitNot) {
        if (LLVM_LIKELY(O2REG(BitNot).isNumber())) { /* Fast-path. */
          O1REG(BitNot) = HermesValue::encodeDoubleValue(
              ~hermes::truncateToInt32(O2REG(BitNot).getNumber()));
          ip = NEXTINST(BitNot);
          DISPATCH;
        }
        runtime->storeCallerIP(ip);
        res = toInt32_RJS(runtime, Handle<>(&O2REG(BitNot)));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(BitNot) = HermesValue::encodeDoubleValue(
            ~static_cast<int32_t>(res->getNumber()));
        ip = NEXTINST(BitNot);
        DISPATCH;
      }

      CASE(GetArgumentsLength) {
        // If the arguments object hasn't been created yet.
        if (O2REG(GetArgumentsLength).isUndefined()) {
          O1REG(GetArgumentsLength) =
              HermesValue::encodeNumberValue(FRAME.getArgCount());
          ip = NEXTINST(GetArgumentsLength);
          DISPATCH;
        }
        // The arguments object has been created, so this is a regular property
        // get.
        assert(
            O2REG(GetArgumentsLength).isObject() &&
            "arguments lazy register is not an object");
        runtime->storeCallerIP(ip);
        res = JSObject::getNamed_RJS(
            Handle<JSObject>::vmcast(&O2REG(GetArgumentsLength)),
            runtime,
            Predefined::getSymbolID(Predefined::length));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetArgumentsLength) = *res;
        ip = NEXTINST(GetArgumentsLength);
        DISPATCH;
      }

      CASE(GetArgumentsPropByVal) {
        // If the arguments object hasn't been created yet and we have a
        // valid integer index, we use the fast path.
        if (O3REG(GetArgumentsPropByVal).isUndefined()) {
          // If this is an integer index.
          if (auto index = toArrayIndexFastPath(O2REG(GetArgumentsPropByVal))) {
            // Is this an existing argument?
            if (*index < FRAME.getArgCount()) {
              O1REG(GetArgumentsPropByVal) = FRAME.getArgRef(*index);
              ip = NEXTINST(GetArgumentsPropByVal);
              DISPATCH;
            }
          }
        }
        // Slow path.
        runtime->storeCallerIP(ip);
        res = getArgumentsPropByValSlowPath_RJS(
            runtime,
            &O3REG(GetArgumentsPropByVal),
            &O2REG(GetArgumentsPropByVal),
            FRAME.getCalleeClosureHandleUnsafe(),
            strictMode);
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetArgumentsPropByVal) = *res;
        ip = NEXTINST(GetArgumentsPropByVal);
        DISPATCH;
      }

      CASE(ReifyArguments) {
        // If the arguments object was already created, do nothing.
        if (!O1REG(ReifyArguments).isUndefined()) {
          assert(
              O1REG(ReifyArguments).isObject() &&
              "arguments lazy register is not an object");
          ip = NEXTINST(ReifyArguments);
          DISPATCH;
        }
        res = reifyArgumentsSlowPath(
            runtime, FRAME.getCalleeClosureHandleUnsafe(), strictMode);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(ReifyArguments) = *res;
        ip = NEXTINST(ReifyArguments);
        DISPATCH;
      }

      CASE(NewObject) {
        // Create a new object using the built-in constructor. Note that the
        // built-in constructor is empty, so we don't actually need to call
        // it.
        O1REG(NewObject) = JSObject::create(runtime).getHermesValue();
        assert(
            gcScope.getHandleCountDbg() == KEEP_HANDLES &&
            "Should not create handles.");
        ip = NEXTINST(NewObject);
        DISPATCH;
      }
      CASE(NewObjectWithParent) {
        O1REG(NewObjectWithParent) =
            JSObject::create(
                runtime,
                O2REG(NewObjectWithParent).isObject()
                    ? Handle<JSObject>::vmcast(&O2REG(NewObjectWithParent))
                    : O2REG(NewObjectWithParent).isNull()
                        ? Runtime::makeNullHandle<JSObject>()
                        : Handle<JSObject>::vmcast(&runtime->objectPrototype))
                .getHermesValue();
        assert(
            gcScope.getHandleCountDbg() == KEEP_HANDLES &&
            "Should not create handles.");
        ip = NEXTINST(NewObjectWithParent);
        DISPATCH;
      }

      CASE(NewObjectWithBuffer) {
        res = Interpreter::createObjectFromBuffer(
            runtime,
            curCodeBlock,
            ip->iNewObjectWithBuffer.op3,
            ip->iNewObjectWithBuffer.op4,
            ip->iNewObjectWithBuffer.op5);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewObjectWithBuffer) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewObjectWithBuffer);
        DISPATCH;
      }

      CASE(NewObjectWithBufferLong) {
        res = Interpreter::createObjectFromBuffer(
            runtime,
            curCodeBlock,
            ip->iNewObjectWithBufferLong.op3,
            ip->iNewObjectWithBufferLong.op4,
            ip->iNewObjectWithBufferLong.op5);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewObjectWithBufferLong) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewObjectWithBufferLong);
        DISPATCH;
      }

      CASE(NewArray) {
        // Create a new array using the built-in constructor. Note that the
        // built-in constructor is empty, so we don't actually need to call
        // it.
        auto createRes =
            JSArray::create(runtime, ip->iNewArray.op2, ip->iNewArray.op2);
        if (createRes == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        O1REG(NewArray) = createRes->getHermesValue();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewArray);
        DISPATCH;
      }

      CASE(NewArrayWithBuffer) {
        res = Interpreter::createArrayFromBuffer(
            runtime,
            curCodeBlock,
            ip->iNewArrayWithBuffer.op2,
            ip->iNewArrayWithBuffer.op3,
            ip->iNewArrayWithBuffer.op4);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewArrayWithBuffer) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(NewArrayWithBuffer);
        DISPATCH;
      }

      CASE(NewArrayWithBufferLong) {
        res = Interpreter::createArrayFromBuffer(
            runtime,
            curCodeBlock,
            ip->iNewArrayWithBufferLong.op2,
            ip->iNewArrayWithBufferLong.op3,
            ip->iNewArrayWithBufferLong.op4);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewArrayWithBufferLong) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(NewArrayWithBufferLong);
        DISPATCH;
      }

      CASE(CreateThis) {
        // Registers: output, prototype, closure.
        if (LLVM_UNLIKELY(!vmisa<Callable>(O3REG(CreateThis)))) {
          runtime->raiseTypeError("constructor is not callable");
          goto exception;
        }
        res = Callable::newObject(
            Handle<Callable>::vmcast(&O3REG(CreateThis)),
            runtime,
            Handle<JSObject>::vmcast(
                O2REG(CreateThis).isObject() ? &O2REG(CreateThis)
                                             : &runtime->objectPrototype));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(CreateThis) = *res;
        ip = NEXTINST(CreateThis);
        DISPATCH;
      }

      CASE(SelectObject) {
        // Registers: output, thisObject, constructorReturnValue.
        O1REG(SelectObject) = O3REG(SelectObject).isObject()
            ? O3REG(SelectObject)
            : O2REG(SelectObject);
        ip = NEXTINST(SelectObject);
        DISPATCH;
      }

      CASE(Eq)
      CASE(Neq) {
        runtime->storeCallerIP(ip);
        res = abstractEqualityTest_RJS(
            runtime, Handle<>(&O2REG(Eq)), Handle<>(&O3REG(Eq)));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(Eq) = ip->opCode == OpCode::Eq
            ? res.getValue()
            : HermesValue::encodeBoolValue(!res->getBool());
        ip = NEXTINST(Eq);
        DISPATCH;
      }
      CASE(StrictEq) {
        O1REG(StrictEq) = HermesValue::encodeBoolValue(
            strictEqualityTest(O2REG(StrictEq), O3REG(StrictEq)));
        ip = NEXTINST(StrictEq);
        DISPATCH;
      }
      CASE(StrictNeq) {
        O1REG(StrictNeq) = HermesValue::encodeBoolValue(
            !strictEqualityTest(O2REG(StrictNeq), O3REG(StrictNeq)));
        ip = NEXTINST(StrictNeq);
        DISPATCH;
      }
      CASE(Not) {
        O1REG(Not) = HermesValue::encodeBoolValue(!toBoolean(O2REG(Not)));
        ip = NEXTINST(Not);
        DISPATCH;
      }
      CASE(Negate) {
        if (LLVM_LIKELY(O2REG(Negate).isNumber())) {
          O1REG(Negate) =
              HermesValue::encodeDoubleValue(-O2REG(Negate).getNumber());
        } else {
          runtime->storeCallerIP(ip);
          res = toNumber_RJS(runtime, Handle<>(&O2REG(Negate)));
          runtime->clearCallerIP();
          if (res == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          O1REG(Negate) = HermesValue::encodeDoubleValue(-res->getNumber());
        }
        ip = NEXTINST(Negate);
        DISPATCH;
      }
      CASE(TypeOf) {
        O1REG(TypeOf) = typeOf(runtime, Handle<>(&O2REG(TypeOf)));
        ip = NEXTINST(TypeOf);
        DISPATCH;
      }
      CASE(Mod) {
        // We use fmod here for simplicity. Theoretically fmod behaves slightly
        // differently than the ECMAScript Spec. fmod applies round-towards-zero
        // for the remainder when it's not representable by a double; while the
        // spec requires round-to-nearest. As an example, 5 % 0.7 will give
        // 0.10000000000000031 using fmod, but using the rounding style
        // described
        // by the spec, the output should really be 0.10000000000000053.
        // Such difference can be ignored in practice.
        if (LLVM_LIKELY(O2REG(Mod).isNumber() && O3REG(Mod).isNumber())) {
          /* Fast-path. */
          O1REG(Mod) = HermesValue::encodeDoubleValue(
              std::fmod(O2REG(Mod).getNumber(), O3REG(Mod).getNumber()));
          ip = NEXTINST(Mod);
          DISPATCH;
        }
        runtime->storeCallerIP(ip);
        res = toNumber_RJS(runtime, Handle<>(&O2REG(Mod)));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION)
          goto exception;
        double left = res->getDouble();
        runtime->storeCallerIP(ip);
        res = toNumber_RJS(runtime, Handle<>(&O3REG(Mod)));
        runtime->clearCallerIP();
        if (res == ExecutionStatus::EXCEPTION)
          goto exception;
        O1REG(Mod) =
            HermesValue::encodeDoubleValue(std::fmod(left, res->getDouble()));
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(Mod);
        DISPATCH;
      }
      CASE(InstanceOf) {
        runtime->storeCallerIP(ip);
        auto result = instanceOfOperator_RJS(
            runtime,
            Handle<>(&O2REG(InstanceOf)),
            Handle<>(&O3REG(InstanceOf)));
        runtime->clearCallerIP();
        if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(InstanceOf) = HermesValue::encodeBoolValue(*result);
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(InstanceOf);
        DISPATCH;
      }
      CASE(IsIn) {
        {
          if (LLVM_UNLIKELY(!O3REG(IsIn).isObject())) {
            runtime->raiseTypeError("right operand of 'in' is not an object");
            goto exception;
          }
          MutableHandle<JSObject> inObject{runtime};
          ComputedPropertyDescriptor desc;
          if (JSObject::getComputedDescriptor(
                  Handle<JSObject>::vmcast(&O3REG(IsIn)),
                  runtime,
                  Handle<>(&O2REG(IsIn)),
                  inObject,
                  desc) == ExecutionStatus::EXCEPTION) {
            goto exception;
          }

          O1REG(IsIn) = HermesValue::encodeBoolValue(!!inObject);
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(IsIn);
        DISPATCH;
      }

      CASE(PutNewOwnByIdShort) {
        nextIP = NEXTINST(PutNewOwnByIdShort);
        idVal = ip->iPutNewOwnByIdShort.op3;
        goto putOwnById;
      }
      CASE(PutNewOwnNEByIdLong)
      CASE(PutNewOwnByIdLong) {
        nextIP = NEXTINST(PutNewOwnByIdLong);
        idVal = ip->iPutNewOwnByIdLong.op3;
        goto putOwnById;
      }
      CASE(PutNewOwnNEById)
      CASE(PutNewOwnById) {
        nextIP = NEXTINST(PutNewOwnById);
        idVal = ip->iPutNewOwnById.op3;
      }
    putOwnById : {
      assert(
          O1REG(PutNewOwnById).isObject() &&
          "Object argument of PutNewOwnById must be an object");
      if (LLVM_UNLIKELY(
              JSObject::defineNewOwnProperty(
                  Handle<JSObject>::vmcast(&O1REG(PutNewOwnById)),
                  runtime,
                  ID(idVal),
                  ip->opCode <= OpCode::PutNewOwnByIdLong
                      ? PropertyFlags::defaultNewNamedPropertyFlags()
                      : PropertyFlags::nonEnumerablePropertyFlags(),
                  Handle<>(&O2REG(PutNewOwnById))) ==
              ExecutionStatus::EXCEPTION)) {
        goto exception;
      }
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(DelByIdLong) {
        idVal = ip->iDelByIdLong.op3;
        nextIP = NEXTINST(DelByIdLong);
        goto DelById;
      }

      CASE(DelById) {
        idVal = ip->iDelById.op3;
        nextIP = NEXTINST(DelById);
      }
    DelById : {
      if (LLVM_LIKELY(O2REG(DelById).isObject())) {
        auto status = JSObject::deleteNamed(
            Handle<JSObject>::vmcast(&O2REG(DelById)),
            runtime,
            ID(idVal),
            defaultPropOpFlags);
        if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(DelById) = HermesValue::encodeBoolValue(status.getValue());
      } else {
        // This is the "slow path".
        res = toObject(runtime, Handle<>(&O2REG(DelById)));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          // If an exception is thrown, likely we are trying to convert
          // undefined/null to an object. Passing over the name of the property
          // so that we could emit more meaningful error messages.
          (void)amendPropAccessErrorMsgWithPropName(
              runtime, Handle<>(&O2REG(DelById)), "delete", ID(idVal));
          goto exception;
        }
        tmpHandle = res.getValue();
        auto status = JSObject::deleteNamed(
            Handle<JSObject>::vmcast(tmpHandle),
            runtime,
            ID(idVal),
            defaultPropOpFlags);
        if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(DelById) = HermesValue::encodeBoolValue(status.getValue());
        tmpHandle.clear();
      }
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(DelByVal) {
        if (LLVM_LIKELY(O2REG(DelByVal).isObject())) {
          auto status = JSObject::deleteComputed(
              Handle<JSObject>::vmcast(&O2REG(DelByVal)),
              runtime,
              Handle<>(&O3REG(DelByVal)),
              defaultPropOpFlags);
          if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          O1REG(DelByVal) = HermesValue::encodeBoolValue(status.getValue());
        } else {
          // This is the "slow path".
          res = toObject(runtime, Handle<>(&O2REG(DelByVal)));
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          tmpHandle = res.getValue();
          auto status = JSObject::deleteComputed(
              Handle<JSObject>::vmcast(tmpHandle),
              runtime,
              Handle<>(&O3REG(DelByVal)),
              defaultPropOpFlags);
          if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          O1REG(DelByVal) = HermesValue::encodeBoolValue(status.getValue());
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(DelByVal);
        DISPATCH;
      }
      CASE(CreateRegExp) {
        {
          // Create the RegExp object.
          res = JSRegExp::create(
              runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
          if (res == ExecutionStatus::EXCEPTION) {
            goto exception;
          }
          auto re = runtime->makeHandle<JSRegExp>(*res);
          // Initialize the regexp.
          auto pattern =
              runtime->makeHandle(curCodeBlock->getRuntimeModule()
                                      ->getStringPrimFromStringIDMayAllocate(
                                          ip->iCreateRegExp.op2));
          auto flags =
              runtime->makeHandle(curCodeBlock->getRuntimeModule()
                                      ->getStringPrimFromStringIDMayAllocate(
                                          ip->iCreateRegExp.op3));
          auto bytecode =
              curCodeBlock->getRuntimeModule()->getRegExpBytecodeFromRegExpID(
                  ip->iCreateRegExp.op4);
          if (LLVM_UNLIKELY(
                  JSRegExp::initialize(re, runtime, pattern, flags, bytecode) ==
                  ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          // Done, return the new object.
          O1REG(CreateRegExp) = re.getHermesValue();
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateRegExp);
        DISPATCH;
      }

      CASE(SwitchImm) {
        if (LLVM_LIKELY(O1REG(SwitchImm).isNumber())) {
          double numVal = O1REG(SwitchImm).getNumber();
          uint32_t uintVal = (uint32_t)numVal;
          if (LLVM_LIKELY(numVal == uintVal) && // Only integers.
              LLVM_LIKELY(uintVal >= ip->iSwitchImm.op4) && // Bounds checking.
              LLVM_LIKELY(uintVal <= ip->iSwitchImm.op5)) // Bounds checking.
          {
            // Calculate the offset into the bytecode where the jump table for
            // this SwitchImm starts.
            const uint8_t *tablestart = (const uint8_t *)llvm::alignAddr(
                (const uint8_t *)ip + ip->iSwitchImm.op2, sizeof(uint32_t));

            // Read the offset from the table.
            const uint32_t *loc =
                (const uint32_t *)tablestart + uintVal - ip->iSwitchImm.op4;

            ip = IPADD(*loc);
            DISPATCH;
          }
        }
        // Wrong type or out of range, jump to default.
        ip = IPADD(ip->iSwitchImm.op3);
        DISPATCH;
      }
      LOAD_CONST(
          LoadConstUInt8,
          HermesValue::encodeDoubleValue(ip->iLoadConstUInt8.op2));
      LOAD_CONST(
          LoadConstInt, HermesValue::encodeDoubleValue(ip->iLoadConstInt.op2));
      LOAD_CONST(
          LoadConstDouble,
          HermesValue::encodeDoubleValue(ip->iLoadConstDouble.op2));
      LOAD_CONST(
          LoadConstString,
          HermesValue::encodeStringValue(
              curCodeBlock->getRuntimeModule()
                  ->getStringPrimFromStringIDMayAllocate(
                      ip->iLoadConstString.op2)));
      LOAD_CONST(
          LoadConstStringLongIndex,
          HermesValue::encodeStringValue(
              curCodeBlock->getRuntimeModule()
                  ->getStringPrimFromStringIDMayAllocate(
                      ip->iLoadConstStringLongIndex.op2)));
      LOAD_CONST(LoadConstUndefined, HermesValue::encodeUndefinedValue());
      LOAD_CONST(LoadConstNull, HermesValue::encodeNullValue());
      LOAD_CONST(LoadConstTrue, HermesValue::encodeBoolValue(true));
      LOAD_CONST(LoadConstFalse, HermesValue::encodeBoolValue(false));
      LOAD_CONST(LoadConstZero, HermesValue::encodeDoubleValue(0));
      BINOP(Sub, doSub);
      BINOP(Mul, doMult);
      BINOP(Div, doDiv);
      BITWISEBINOP(BitAnd, &);
      BITWISEBINOP(BitOr, |);
      BITWISEBINOP(BitXor, ^);
      // For LShift, we need to use toUInt32 first because lshift on negative
      // numbers is undefined behavior in theory.
      SHIFTOP(LShift, <<, toUInt32_RJS, uint32_t, int32_t);
      SHIFTOP(RShift, >>, toInt32_RJS, int32_t, int32_t);
      SHIFTOP(URshift, >>, toUInt32_RJS, uint32_t, uint32_t);
      CONDOP(Less, <, lessOp_RJS);
      CONDOP(LessEq, <=, lessEqualOp_RJS);
      CONDOP(Greater, >, greaterOp_RJS);
      CONDOP(GreaterEq, >=, greaterEqualOp_RJS);
      JCOND(Less, <, lessOp_RJS);
      JCOND(LessEqual, <=, lessEqualOp_RJS);
      JCOND(Greater, >, greaterOp_RJS);
      JCOND(GreaterEqual, >=, greaterEqualOp_RJS);

      JCOND_STRICT_EQ_IMPL(
          JStrictEqual, , IPADD(ip->iJStrictEqual.op1), NEXTINST(JStrictEqual));
      JCOND_STRICT_EQ_IMPL(
          JStrictEqual,
          Long,
          IPADD(ip->iJStrictEqualLong.op1),
          NEXTINST(JStrictEqualLong));
      JCOND_STRICT_EQ_IMPL(
          JStrictNotEqual,
          ,
          NEXTINST(JStrictNotEqual),
          IPADD(ip->iJStrictNotEqual.op1));
      JCOND_STRICT_EQ_IMPL(
          JStrictNotEqual,
          Long,
          NEXTINST(JStrictNotEqualLong),
          IPADD(ip->iJStrictNotEqualLong.op1));

      JCOND_EQ_IMPL(JEqual, , IPADD(ip->iJEqual.op1), NEXTINST(JEqual));
      JCOND_EQ_IMPL(
          JEqual, Long, IPADD(ip->iJEqualLong.op1), NEXTINST(JEqualLong));
      JCOND_EQ_IMPL(
          JNotEqual, , NEXTINST(JNotEqual), IPADD(ip->iJNotEqual.op1));
      JCOND_EQ_IMPL(
          JNotEqual,
          Long,
          NEXTINST(JNotEqualLong),
          IPADD(ip->iJNotEqualLong.op1));

      CASE_OUTOFLINE(PutOwnByVal);
      CASE_OUTOFLINE(PutOwnGetterSetterByVal);
      CASE_OUTOFLINE(DirectEval);

      CASE(_last) {
        llvm_unreachable("Invalid opcode _last");
      }
    }

    llvm_unreachable("unreachable");

  // We arrive here if we couldn't allocate the registers for the current frame.
  stackOverflow:
    runtime->raiseStackOverflow(Runtime::StackOverflowKind::JSRegisterStack);

  // We arrive here when we raised an exception in a callee, but we don't want
  // the callee to be able to handle it.
  handleExceptionInParent:
    // Restore the caller code block and IP.
    curCodeBlock = FRAME.getSavedCodeBlock();
    ip = FRAME.getSavedIP();

    // Pop to the previous frame where technically the error happened.
    frameRegs =
        &runtime->restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

    // If we are coming from native code, return.
    if (!curCodeBlock)
      return ExecutionStatus::EXCEPTION;

// Return because of recursive calling structure
#ifdef HERMESVM_PROFILER_EXTERN
    return ExecutionStatus::EXCEPTION;
#endif
  // Handle the exception.
  exception:
    UPDATE_OPCODE_TIME_SPENT;
    assert(
        !runtime->thrownValue_.isEmpty() &&
        "thrownValue unavailable at exception");

    bool catchable = true;
    // If this is an Error object that was thrown internally, it didn't have
    // access to the current codeblock and IP, so collect the stack trace here.
    if (auto *jsError = dyn_vmcast<JSError>(runtime->thrownValue_)) {
      catchable = jsError->catchable();
      if (!jsError->getStackTrace()) {
        // Temporarily clear the thrown value for following operations.
        auto errorHandle =
            runtime->makeHandle(vmcast<JSError>(runtime->thrownValue_));
        runtime->clearThrownValue();

        JSError::recordStackTrace(
            errorHandle, runtime, false, curCodeBlock, ip);

        // Restore the thrown value.
        runtime->setThrownValue(errorHandle.getHermesValue());
      }
    }

    gcScope.flushToSmallCount(KEEP_HANDLES);
    tmpHandle.clear();

#ifdef HERMES_ENABLE_DEBUGGER
    if (SingleStep) {
      // If we're single stepping, don't bother with any more checks,
      // and simply signal that we should continue execution with an exception.
      state.codeBlock = curCodeBlock;
      state.offset = CUROFFSET;
      return ExecutionStatus::EXCEPTION;
    }

    using PauseOnThrowMode = facebook::hermes::debugger::PauseOnThrowMode;
    auto mode = runtime->debugger_.getPauseOnThrowMode();
    if (mode != PauseOnThrowMode::None) {
      if (!runtime->debugger_.isDebugging()) {
        // Determine whether the PauseOnThrowMode requires us to stop here.
        bool caught =
            runtime->debugger_
                .findCatchTarget(InterpreterState(curCodeBlock, CUROFFSET))
                .hasValue();
        bool shouldStop = mode == PauseOnThrowMode::All ||
            (mode == PauseOnThrowMode::Uncaught && !caught);
        if (shouldStop) {
          // When runDebugger is invoked after an exception,
          // stepping should never happen internally.
          // Any step is a step to an exception handler, which we do
          // directly here in the interpreter.
          // Thus, the result state should be the same as the input state.
          InterpreterState tmpState{curCodeBlock, (uint32_t)CUROFFSET};
          ExecutionStatus resultStatus = runtime->debugger_.runDebugger(
              Debugger::RunReason::Exception, tmpState);
          (void)resultStatus;
          assert(
              tmpState == InterpreterState(curCodeBlock, CUROFFSET) &&
              "not allowed to step internally in a pauseOnThrow");
          gcScope.flushToSmallCount(KEEP_HANDLES);
        }
      }
    }
#endif

    int32_t handlerOffset = 0;

    // If the exception is not catchable, skip found catch blocks.
    while (((handlerOffset = curCodeBlock->findCatchTargetOffset(CUROFFSET)) ==
            -1) ||
           !catchable) {
      PROFILER_EXIT_FUNCTION(curCodeBlock);

      // Restore the code block and IP.
      curCodeBlock = FRAME.getSavedCodeBlock();
      ip = FRAME.getSavedIP();

      // Pop a stack frame.
      frameRegs =
          &runtime->restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

      SLOW_DEBUG(
          dbgs() << "function exit with exception: restored stackLevel="
                 << runtime->getStackLevel() << "\n");

      // Are we returning to native code?
      if (!curCodeBlock) {
        SLOW_DEBUG(
            dbgs()
            << "function exit with exception: returning to native code\n");
        return ExecutionStatus::EXCEPTION;
      }

      assert(
          isCallType(ip->opCode) &&
          "return address is not Call-type instruction");

// Return because of recursive calling structure
#ifdef HERMESVM_PROFILER_EXTERN
      return ExecutionStatus::EXCEPTION;
#endif
    }

    INIT_STATE_FOR_CODEBLOCK(curCodeBlock);

    ip = IPADD(handlerOffset - CUROFFSET);
  }
}

} // namespace vm
} // namespace hermes
