/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "vm"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/Runtime.h"

#include "hermes/Inst/InstDecode.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/Support/Statistic.h"
#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/HandleRootOwner-inline.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSGenerator.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Profiler.h"
#include "hermes/VM/Profiler/CodeCoverageProfiler.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/WeakRoot-inline.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

#include "Interpreter-internal.h"

using llvh::dbgs;
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
    NumGetByIdDict,
    "NumGetByIdDict: Number of property 'read by id' of dictionaries");
HERMES_SLOW_STATISTIC(
    NumGetByIdSlow,
    "NumGetByIdSlow: Number of property 'read by id' slow path");

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
// function that recursively calls the interpreter. See Profiler.{h,cpp} for how
// these symbols are subsequently patched with JS function names.
#define INTERP_WRAPPER(name)                                                \
  __attribute__((__noinline__)) static llvh::CallResult<llvh::HermesValue>  \
  name(hermes::vm::Runtime &runtime, hermes::vm::CodeBlock *newCodeBlock) { \
    return runtime.interpretFunctionImpl(newCodeBlock);                     \
  }
PROFILER_SYMBOLS(INTERP_WRAPPER)
#endif

namespace hermes {
namespace vm {

#if defined(HERMESVM_PROFILER_EXTERN)
typedef CallResult<HermesValue> (*WrapperFunc)(Runtime &, CodeBlock *);
#define LIST_ITEM(name) name,
static const WrapperFunc interpWrappers[] = {PROFILER_SYMBOLS(LIST_ITEM)};
#endif

/// Initialize the state of some internal variables based on the current
/// code block.
#define INIT_STATE_FOR_CODEBLOCK(codeBlock)                      \
  do {                                                           \
    strictMode = (codeBlock)->isStrictMode();                    \
    defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(strictMode);      \
    if (EnableCrashTrace) {                                      \
      auto *bc = (codeBlock)->getRuntimeModule()->getBytecode(); \
      bytecodeFileStart = bc->getRawBuffer().data();             \
      auto hash = bc->getSourceHash();                           \
      runtime.crashTrace_.recordModule(                          \
          bc->getSegmentID(),                                    \
          (codeBlock)->getRuntimeModule()->getSourceURL(),       \
          llvh::StringRef((const char *)&hash, sizeof(hash)));   \
    }                                                            \
  } while (0)

CallResult<PseudoHandle<JSGenerator>> Interpreter::createGenerator_RJS(
    Runtime &runtime,
    RuntimeModule *runtimeModule,
    unsigned funcIndex,
    Handle<Environment> envHandle,
    NativeArgs args) {
  auto gifRes = GeneratorInnerFunction::create(
      runtime,
      runtimeModule->getDomain(runtime),
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      envHandle,
      runtimeModule->getCodeBlockMayAllocate(funcIndex),
      args);
  if (LLVM_UNLIKELY(gifRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto generatorFunction = runtime.makeHandle(vmcast<JSGeneratorFunction>(
      runtime.getCurrentFrame().getCalleeClosureUnsafe()));

  auto prototypeProp = JSObject::getNamed_RJS(
      generatorFunction,
      runtime,
      Predefined::getSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(prototypeProp == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> prototype = vmisa<JSObject>(prototypeProp->get())
      ? runtime.makeHandle<JSObject>(prototypeProp->get())
      : Handle<JSObject>::vmcast(&runtime.generatorPrototype);

  return JSGenerator::create(runtime, *gifRes, prototype);
}

CallResult<Handle<Arguments>> Interpreter::reifyArgumentsSlowPath(
    Runtime &runtime,
    Handle<Callable> curFunction,
    bool strictMode) {
  auto frame = runtime.getCurrentFrame();
  uint32_t argCount = frame.getArgCount();
  // Define each JavaScript argument.
  auto argRes = Arguments::create(runtime, argCount, curFunction, strictMode);
  if (LLVM_UNLIKELY(argRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<Arguments> args = *argRes;

  for (uint32_t argIndex = 0; argIndex < argCount; ++argIndex) {
    SmallHermesValue shv =
        SmallHermesValue::encodeHermesValue(frame.getArgRef(argIndex), runtime);
    Arguments::unsafeSetExistingElementAt(*args, runtime, argIndex, shv);
  }

  // The returned value should already be set from the create call.
  return args;
}

CallResult<PseudoHandle<>> Interpreter::getArgumentsPropByValSlowPath_RJS(
    Runtime &runtime,
    PinnedHermesValue *lazyReg,
    PinnedHermesValue *valueReg,
    Handle<Callable> curFunction,
    bool strictMode) {
  auto frame = runtime.getCurrentFrame();

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
    auto strPrim = runtime.makeHandle(std::move(*strRes));

    // Check if the string is a valid argument index.
    if (auto index = toArrayIndex(runtime, strPrim)) {
      if (*index < frame.getArgCount()) {
        return createPseudoHandle(frame.getArgRef(*index));
      }

      auto objectPrototype = Handle<JSObject>::vmcast(&runtime.objectPrototype);

      // OK, they are requesting an index that either doesn't exist or is
      // somewhere up in the prototype chain. Since we want to avoid reifying,
      // check which it is:
      MutableHandle<JSObject> inObject{runtime};
      MutableHandle<SymbolID> inNameTmpStorage{runtime};
      ComputedPropertyDescriptor desc;
      JSObject::getComputedPrimitiveDescriptor(
          objectPrototype, runtime, strPrim, inObject, inNameTmpStorage, desc);

      // If we couldn't find the property, just return 'undefined'.
      if (!inObject)
        return createPseudoHandle(HermesValue::encodeUndefinedValue());

      // If the property isn't an accessor, we can just return it without
      // reifying.
      if (!desc.flags.accessor) {
        return JSObject::getComputedSlotValue(
            createPseudoHandle(inObject.get()),
            runtime,
            inNameTmpStorage,
            desc);
      }
    }

    // Are they requesting "arguments.length"?
    if (runtime.symbolEqualsToStringPrim(
            Predefined::getSymbolID(Predefined::length), *strPrim)) {
      return createPseudoHandle(
          HermesValue::encodeDoubleValue(frame.getArgCount()));
    }
  }

  // Looking for an accessor or a property that needs reification.
  auto argRes = reifyArgumentsSlowPath(runtime, curFunction, strictMode);
  if (argRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Update the register with the reified value.
  *lazyReg = argRes->getHermesValue();

  // For simplicity, call ourselves again.
  return getArgumentsPropByValSlowPath_RJS(
      runtime, lazyReg, valueReg, curFunction, strictMode);
}

CallResult<PseudoHandle<>> Interpreter::handleCallSlowPath(
    Runtime &runtime,
    PinnedHermesValue *callTarget) {
  if (auto *native = dyn_vmcast<NativeFunction>(*callTarget)) {
    ++NumNativeFunctionCalls;
    // Call the native function directly
    return NativeFunction::_nativeCall(native, runtime);
  } else if (auto *bound = dyn_vmcast<BoundFunction>(*callTarget)) {
    ++NumBoundFunctionCalls;
    // Call the bound function.
    return BoundFunction::_boundCall(bound, runtime.getCurrentIP(), runtime);
  } else {
    return runtime.raiseTypeErrorForValue(
        Handle<>(callTarget), " is not a function");
  }
}

inline PseudoHandle<> Interpreter::tryGetPrimitiveOwnPropertyById(
    Runtime &runtime,
    Handle<> base,
    SymbolID id) {
  if (base->isString() && id == Predefined::getSymbolID(Predefined::length)) {
    return createPseudoHandle(
        HermesValue::encodeNumberValue(base->getString()->getStringLength()));
  }
  return createPseudoHandle(HermesValue::encodeEmptyValue());
}

CallResult<PseudoHandle<>> Interpreter::getByIdTransient_RJS(
    Runtime &runtime,
    Handle<> base,
    SymbolID id) {
  // This is similar to what ES5.1 8.7.1 special [[Get]] internal
  // method did, but that section doesn't exist in ES9 anymore.
  // Instead, the [[Get]] Receiver argument serves a similar purpose.

  // Fast path: try to get primitive own property directly first.
  PseudoHandle<> valOpt = tryGetPrimitiveOwnPropertyById(runtime, base, id);
  if (!valOpt->isEmpty()) {
    return valOpt;
  }

  // get the property descriptor from primitive prototype without
  // boxing with vm::toObject().  This is where any properties will
  // be.
  CallResult<Handle<JSObject>> primitivePrototypeResult =
      getPrimitivePrototype(runtime, base);
  if (primitivePrototypeResult == ExecutionStatus::EXCEPTION) {
    // If an exception is thrown, likely we are trying to read property on
    // undefined/null. Passing over the name of the property
    // so that we could emit more meaningful error messages.
    return amendPropAccessErrorMsgWithPropName(runtime, base, "read", id);
  }

  return JSObject::getNamedWithReceiver_RJS(
      *primitivePrototypeResult, runtime, id, base);
}

PseudoHandle<> Interpreter::getByValTransientFast(
    Runtime &runtime,
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
      return createPseudoHandle(
          runtime
              .getCharacterString(base->getString()->at(arrayIndex.getValue()))
              .getHermesValue());
    }
  }
  return createPseudoHandle(HermesValue::encodeEmptyValue());
}

CallResult<PseudoHandle<>> Interpreter::getByValTransient_RJS(
    Runtime &runtime,
    Handle<> base,
    Handle<> name) {
  // This is similar to what ES5.1 8.7.1 special [[Get]] internal
  // method did, but that section doesn't exist in ES9 anymore.
  // Instead, the [[Get]] Receiver argument serves a similar purpose.

  // Optimization: check fast path first.
  PseudoHandle<> fastRes = getByValTransientFast(runtime, base, name);
  if (!fastRes->isEmpty()) {
    return fastRes;
  }

  auto res = toObject(runtime, base);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  return JSObject::getComputedWithReceiver_RJS(
      runtime.makeHandle<JSObject>(res.getValue()), runtime, name, base);
}

static ExecutionStatus
transientObjectPutErrorMessage(Runtime &runtime, Handle<> base, SymbolID id) {
  // Emit an error message that looks like:
  // "Cannot create property '%{id}' on ${typeof base} '${String(base)}'".
  StringView propName = runtime.getIdentifierTable().getStringView(runtime, id);
  Handle<StringPrimitive> baseType =
      runtime.makeHandle(vmcast<StringPrimitive>(typeOf(runtime, base)));
  StringView baseTypeAsString =
      StringPrimitive::createStringView(runtime, baseType);
  MutableHandle<StringPrimitive> valueAsString{runtime};
  if (base->isSymbol()) {
    // Special workaround for Symbol which can't be stringified.
    auto str = symbolDescriptiveString(runtime, Handle<SymbolID>::vmcast(base));
    if (str != ExecutionStatus::EXCEPTION) {
      valueAsString = *str;
    } else {
      runtime.clearThrownValue();
      valueAsString = StringPrimitive::createNoThrow(
          runtime, "<<Exception occurred getting the value>>");
    }
  } else {
    auto str = toString_RJS(runtime, base);
    assert(
        str != ExecutionStatus::EXCEPTION &&
        "Primitives should be convertible to string without exceptions");
    valueAsString = std::move(*str);
  }
  StringView valueAsStringPrintable =
      StringPrimitive::createStringView(runtime, valueAsString);

  SmallU16String<32> tmp1;
  SmallU16String<32> tmp2;
  return runtime.raiseTypeError(
      TwineChar16("Cannot create property '") + propName + "' on " +
      baseTypeAsString.getUTF16Ref(tmp1) + " '" +
      valueAsStringPrintable.getUTF16Ref(tmp2) + "'");
}

ExecutionStatus Interpreter::putByIdTransient_RJS(
    Runtime &runtime,
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

  auto O = runtime.makeHandle<JSObject>(res.getValue());

  NamedPropertyDescriptor desc;
  JSObject *propObj = JSObject::getNamedDescriptorUnsafe(O, runtime, id, desc);

  // Is this a missing property, or a data property defined in the prototype
  // chain? In both cases we would need to create an own property on the
  // transient object, which is prohibited.
  if (!propObj ||
      (propObj != O.get() &&
       (!desc.flags.accessor && !desc.flags.proxyObject))) {
    if (strictMode) {
      return transientObjectPutErrorMessage(runtime, base, id);
    }
    return ExecutionStatus::RETURNED;
  }

  // Modifying an own data property in a transient object is prohibited.
  if (!desc.flags.accessor && !desc.flags.proxyObject) {
    if (strictMode) {
      return runtime.raiseTypeError(
          "Cannot modify a property in a transient object");
    }
    return ExecutionStatus::RETURNED;
  }

  if (desc.flags.accessor) {
    // This is an accessor.
    auto *accessor = vmcast<PropertyAccessor>(
        JSObject::getNamedSlotValueUnsafe(propObj, runtime, desc)
            .getObject(runtime));

    // It needs to have a setter.
    if (!accessor->setter) {
      if (strictMode) {
        return runtime.raiseTypeError("Cannot modify a read-only accessor");
      }
      return ExecutionStatus::RETURNED;
    }

    CallResult<PseudoHandle<>> setRes =
        accessor->setter.getNonNull(runtime)->executeCall1(
            runtime.makeHandle(accessor->setter), runtime, base, *value);
    if (setRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    assert(desc.flags.proxyObject && "descriptor flags are impossible");
    CallResult<bool> setRes = JSProxy::setNamed(
        runtime.makeHandle(propObj), runtime, id, value, base);
    if (setRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*setRes && strictMode) {
      return runtime.raiseTypeError("transient proxy set returned false");
    }
  }
  return ExecutionStatus::RETURNED;
}

ExecutionStatus Interpreter::putByValTransient_RJS(
    Runtime &runtime,
    Handle<> base,
    Handle<> name,
    Handle<> value,
    bool strictMode) {
  auto idRes = valueToSymbolID(runtime, name);
  if (idRes == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  return putByIdTransient_RJS(runtime, base, **idRes, value, strictMode);
}

static Handle<HiddenClass> getHiddenClassForBuffer(
    Runtime &runtime,
    CodeBlock *curCodeBlock,
    unsigned numLiterals,
    unsigned keyBufferIndex) {
  RuntimeModule *runtimeModule = curCodeBlock->getRuntimeModule();
  if (auto clazzOpt = runtimeModule->findCachedLiteralHiddenClass(
          runtime, keyBufferIndex, numLiterals))
    return *clazzOpt;

  MutableHandle<> tmpHandleKey{runtime};
  MutableHandle<HiddenClass> clazz =
      runtime.makeMutableHandle(runtime.getHiddenClassForPrototypeRaw(
          vmcast<JSObject>(runtime.objectPrototype),
          JSObject::numOverlapSlots<JSObject>()));

  GCScopeMarkerRAII marker{runtime};
  auto keyGen =
      curCodeBlock->getObjectBufferKeyIter(keyBufferIndex, numLiterals);
  while (keyGen.hasNext()) {
    auto key = keyGen.get(runtime);
    SymbolID sym = [&] {
      if (key.isSymbol())
        return ID(key.getSymbol().unsafeGetIndex());

      assert(key.isNumber() && "Key must be symbol or number");
      tmpHandleKey = key;
      // Note that since this handle has been created, the associated symbol
      // will be automatically kept alive until we flush the marker.
      // valueToSymbolID cannot fail because the key is known to be uint32.
      Handle<SymbolID> symHandle = *valueToSymbolID(runtime, tmpHandleKey);
      return *symHandle;
    }();
    auto addResult = HiddenClass::addProperty(
        clazz, runtime, sym, PropertyFlags::defaultNewNamedPropertyFlags());
    clazz = addResult->first;
    marker.flush();
  }

  if (LLVM_LIKELY(!clazz->isDictionary())) {
    assert(
        numLiterals == clazz->getNumProperties() &&
        "numLiterals should match hidden class property count.");
    assert(
        clazz->getNumProperties() < 256 &&
        "cached hidden class should have property count less than 256");
    runtimeModule->tryCacheLiteralHiddenClass(runtime, keyBufferIndex, *clazz);
  }

  return {clazz};
}

CallResult<PseudoHandle<>> Interpreter::createObjectFromBuffer(
    Runtime &runtime,
    CodeBlock *curCodeBlock,
    unsigned numLiterals,
    unsigned keyBufferIndex,
    unsigned valBufferIndex) {
  // Create a new object using the built-in constructor or cached hidden class.
  // Note that the built-in constructor is empty, so we don't actually need to
  // call it.
  auto clazz = getHiddenClassForBuffer(
      runtime, curCodeBlock, numLiterals, keyBufferIndex);
  auto obj = runtime.makeHandle(JSObject::create(runtime, clazz));

  auto valGen =
      curCodeBlock->getObjectBufferValueIter(valBufferIndex, numLiterals);

#ifndef NDEBUG
  auto keyGen =
      curCodeBlock->getObjectBufferKeyIter(keyBufferIndex, numLiterals);
#endif

  uint32_t propIndex = 0;
  // keyGen should always have the same amount of elements as valGen
  while (valGen.hasNext()) {
#ifndef NDEBUG
    {
      GCScopeMarkerRAII marker{runtime};
      // keyGen points to an element in the key buffer, which means it will
      // only ever generate a Number or a Symbol. This means it will never
      // allocate memory, and it is safe to not use a Handle.
      SymbolID stringIdResult{};
      auto key = keyGen.get(runtime);
      if (key.isSymbol()) {
        stringIdResult = ID(key.getSymbol().unsafeGetIndex());
      } else {
        auto keyHandle =
            runtime.makeHandle(HermesValue::encodeDoubleValue(key.getNumber()));
        auto idRes = valueToSymbolID(runtime, keyHandle);
        assert(
            idRes != ExecutionStatus::EXCEPTION &&
            "valueToIdentifier() failed for uint32_t value");
        stringIdResult = **idRes;
      }
      NamedPropertyDescriptor desc;
      auto pos = HiddenClass::findProperty(
          clazz,
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
    // returned from obj.get().
    auto val = valGen.get(runtime);
    auto shv = SmallHermesValue::encodeHermesValue(val, runtime);
    // We made this object, it's not a Proxy.
    JSObject::setNamedSlotValueUnsafe(obj.get(), runtime, propIndex, shv);
    ++propIndex;
  }

  return createPseudoHandle(HermesValue::encodeObjectValue(*obj));
}

CallResult<PseudoHandle<>> Interpreter::createArrayFromBuffer(
    Runtime &runtime,
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
  auto arr = *arrRes;
  JSArray::setStorageEndIndex(arr, runtime, numElements);

  auto iter = curCodeBlock->getArrayBufferIter(bufferIndex, numLiterals);
  JSArray::size_type i = 0;
  while (iter.hasNext()) {
    // NOTE: we must get the value in a separate step to guarantee ordering.
    const auto value =
        SmallHermesValue::encodeHermesValue(iter.get(runtime), runtime);
    JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, value);
  }

  return createPseudoHandle(HermesValue::encodeObjectValue(*arr));
}

#ifndef NDEBUG

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, DumpHermesValue dhv) {
  OS << dhv.hv;
  // If it is a string, dump the contents, truncated to 8 characters.
  if (dhv.hv.isString()) {
    SmallU16String<32> str;
    dhv.hv.getString()->appendUTF16String(str);
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

void dumpCallArguments(
    llvh::raw_ostream &OS,
    Runtime &runtime,
    StackFramePtr calleeFrame) {
  OS << "arguments:\n";
  OS << "  " << 0 << " " << DumpHermesValue(calleeFrame.getThisArgRef())
     << "\n";
  for (unsigned i = 0; i < calleeFrame.getArgCount(); ++i) {
    OS << "  " << (i + 1) << " " << DumpHermesValue(calleeFrame.getArgRef(i))
       << "\n";
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

  dbgs() << llvh::format_decimal((const uint8_t *)ip - curCodeBlock->begin(), 4)
         << " OpCode::" << getOpCodeString(decoded.meta.opCode);

  for (unsigned i = 0; i < decoded.meta.numOperands; ++i) {
    auto operandType = decoded.meta.operandType[i];
    auto value = decoded.operandValue[i];

    dbgs() << (i == 0 ? " " : ", ");
    dumpOperand(dbgs(), operandType, value);

    if (operandType == OperandType::Reg8 || operandType == OperandType::Reg32) {
      // Print the register value, if source.
      if (i != 0 || decoded.meta.numOperands == 1)
        dbgs() << "="
               << DumpHermesValue(REG(static_cast<uint32_t>(value.integer)));
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

  // To avoid needing a large lookup table or switchcase, the following packs
  // information about the size of each call opcode into a uint32_t. Each call
  // type is represented with two bits, representing how much larger it is than
  // the smallest call instruction.
  // If we used 64 bits, we could fit the actual size of each call, without
  // needing the offset, and this may be necessary if new call instructions are
  // added in the future. For now however, due to limitations on loading large
  // immediates in ARM, it is significantly more efficient to use a uint32_t
  // than a uint64_t.
  constexpr auto firstCall = std::min({
#define DEFINE_RET_TARGET(name) static_cast<uint8_t>(OpCode::name),
#include "hermes/BCGen/HBC/BytecodeList.def"
  });
  constexpr auto lastCall = std::max({
#define DEFINE_RET_TARGET(name) static_cast<uint8_t>(OpCode::name),
#include "hermes/BCGen/HBC/BytecodeList.def"
  });
  constexpr auto minSize = std::min({
#define DEFINE_RET_TARGET(name) sizeof(inst::name##Inst),
#include "hermes/BCGen/HBC/BytecodeList.def"
  });
  constexpr auto maxSize = std::max({
#define DEFINE_RET_TARGET(name) sizeof(inst::name##Inst),
#include "hermes/BCGen/HBC/BytecodeList.def"
  });

  constexpr uint32_t W = 2;
  constexpr uint32_t mask = (1 << W) - 1;

  static_assert(llvh::isUInt<W>(maxSize - minSize), "Size range too large.");
  static_assert((lastCall - firstCall + 1) * W <= 32, "Too many call opcodes.");

  constexpr uint32_t callSizes = 0
#define DEFINE_RET_TARGET(name)             \
  |                                         \
      ((sizeof(inst::name##Inst) - minSize) \
       << (((uint8_t)OpCode::name - firstCall) * W))
#include "hermes/BCGen/HBC/BytecodeList.def"
      ;
#undef DEFINE_RET_TARGET

  const uint8_t offset = static_cast<uint8_t>(ip->opCode) - firstCall;
  return IPADD(((callSizes >> (offset * W)) & mask) + minSize);
}

CallResult<HermesValue> Runtime::interpretFunctionImpl(
    CodeBlock *newCodeBlock) {
  newCodeBlock->lazyCompile(*this);

#if defined(HERMES_MEMORY_INSTRUMENTATION) || !defined(NDEBUG)
  // We always call getCurrentIP() in a debug build as this has the effect
  // of asserting the IP is correctly set (not invalidated) at this point.
  // This allows us to leverage our whole test-suite to find missing cases
  // of CAPTURE_IP* macros in the interpreter loop.
  const inst::Inst *ip = getCurrentIP();
  (void)ip;
#endif
#ifdef HERMES_MEMORY_INSTRUMENTATION
  if (ip) {
    const CodeBlock *codeBlock;
    std::tie(codeBlock, ip) = getCurrentInterpreterLocation(ip);
    // All functions end in a Ret so we must match this with a pushCallStack()
    // before executing.
    if (codeBlock) {
      // Push a call entry at the last location we were executing bytecode.
      // This will correctly attribute things like eval().
      pushCallStack(codeBlock, ip);
    } else {
      // Push a call entry at the entry at the top of interpreted code.
      pushCallStack(newCodeBlock, (const Inst *)newCodeBlock->begin());
    }
  } else {
    // Push a call entry at the entry at the top of interpreted code.
    pushCallStack(newCodeBlock, (const Inst *)newCodeBlock->begin());
  }
#endif

  InterpreterState state{newCodeBlock, 0};
  if (HERMESVM_CRASH_TRACE &&
      (getVMExperimentFlags() & experiments::CrashTrace)) {
    return Interpreter::interpretFunction<false, true>(*this, state);
  } else {
    return Interpreter::interpretFunction<false, false>(*this, state);
  }
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
  if (HERMESVM_CRASH_TRACE &&
      (getVMExperimentFlags() & experiments::CrashTrace))
    return Interpreter::interpretFunction<true, true>(*this, state).getStatus();
  else
    return Interpreter::interpretFunction<true, false>(*this, state)
        .getStatus();
}
#endif

template <bool SingleStep, bool EnableCrashTrace>
CallResult<HermesValue> Interpreter::interpretFunction(
    Runtime &runtime,
    InterpreterState &state) {
  // The interpreter is re-entrant and also saves/restores its IP via the
  // runtime whenever a call out is made (see the CAPTURE_IP_* macros). As such,
  // failure to preserve the IP across calls to interpreterFunction() disrupt
  // interpreter calls further up the C++ callstack. The RAII utility class
  // below makes sure we always do this correctly.
  //
  // TODO: The IPs stored in the C++ callstack via this holder will generally be
  // the same as in the JS stack frames via the Saved IP field. We can probably
  // get rid of one of these redundant stores. Doing this isn't completely
  // trivial as there are currently cases where we re-enter the interpreter
  // without calling Runtime::saveCallerIPInStackFrame(), and there are features
  // (I think mostly the debugger + stack traces) which implicitly rely on
  // this behavior. At least their tests break if this behavior is not
  // preserved.
  struct IPSaver {
    IPSaver(Runtime &runtime)
        : ip_(runtime.getCurrentIP()), runtime_(runtime) {}

    ~IPSaver() {
      runtime_.setCurrentIP(ip_);
    }

   private:
    const Inst *ip_;
    Runtime &runtime_;
  };
  IPSaver ipSaver(runtime);

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
  // Points to the first local register in the current frame.
  // This eliminates the indirect load from Runtime and the -1 offset.
  PinnedHermesValue *frameRegs;
  // Strictness of current function.
  bool strictMode;
  // Default flags when accessing properties.
  PropOpFlags defaultPropOpFlags;

// These CAPTURE_IP* macros should wrap around any major calls out of the
// interpreter loop. They stash and retrieve the IP via the current Runtime
// allowing the IP to be externally observed and even altered to change the flow
// of execution. Explicitly saving AND restoring the IP from the Runtime in this
// way means the C++ compiler will keep IP in a register within the rest of the
// interpreter loop.
//
// When assertions are enabled we take the extra step of "invalidating" the IP
// between captures so we can detect if it's erroneously accessed.
//
#ifdef NDEBUG

#define CAPTURE_IP(expr)    \
  runtime.setCurrentIP(ip); \
  (void)(expr);             \
  ip = runtime.getCurrentIP();

// Used when we want to declare a new variable and assign the expression to it.
#define CAPTURE_IP_ASSIGN(decl, expr) \
  runtime.setCurrentIP(ip);           \
  decl = (expr);                      \
  ip = runtime.getCurrentIP();

#else // !NDEBUG

#define CAPTURE_IP(expr)       \
  runtime.setCurrentIP(ip);    \
  (void)(expr);                \
  ip = runtime.getCurrentIP(); \
  runtime.invalidateCurrentIP();

// Used when we want to declare a new variable and assign the expression to it.
#define CAPTURE_IP_ASSIGN(decl, expr) \
  runtime.setCurrentIP(ip);           \
  decl = (expr);                      \
  ip = runtime.getCurrentIP();        \
  runtime.invalidateCurrentIP();

#endif // NDEBUG

/// \def DONT_CAPTURE_IP(expr)
/// \param expr A call expression to a function external to the interpreter. The
///   expression should not make any allocations and the IP should be set
///   immediately following this macro.
#define DONT_CAPTURE_IP(expr)      \
  do {                             \
    NoAllocScope noAlloc(runtime); \
    (void)expr;                    \
  } while (false)

// When performing a tail call, we need to set the runtime IP and leave it set.
#define CAPTURE_IP_SET() runtime.setCurrentIP(ip)

  LLVM_DEBUG(dbgs() << "interpretFunction() called\n");

  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }

  GCScope gcScope(runtime);
  // Avoid allocating a handle dynamically by reusing this one.
  MutableHandle<> tmpHandle(runtime);
  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  CallResult<PseudoHandle<>> resPH{ExecutionStatus::EXCEPTION};
  CallResult<Handle<Arguments>> resArgs{ExecutionStatus::EXCEPTION};
  CallResult<bool> boolRes{ExecutionStatus::EXCEPTION};
  // Start of the bytecode file, used to calculate IP offset in crash traces.
  const uint8_t *bytecodeFileStart;

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
  runtime.getDebugger().willEnterCodeBlock(curCodeBlock);
#endif

  runtime.getCodeCoverageProfiler().markExecuted(curCodeBlock);

  if (!SingleStep) {
    auto newFrame = runtime.setCurrentFrameToTopOfStack();
    runtime.saveCallerIPInStackFrame();
#ifndef NDEBUG
    runtime.invalidateCurrentIP();
#endif

    // Point frameRegs to the first register in the new frame. Note that at this
    // moment technically it points above the top of the stack, but we are never
    // going to access it.
    frameRegs = &newFrame.getFirstLocalRef();

#ifndef NDEBUG
    LLVM_DEBUG(
        dbgs() << "function entry: stackLevel=" << runtime.getStackLevel()
               << ", argCount=" << runtime.getCurrentFrame().getArgCount()
               << ", frameSize=" << curCodeBlock->getFrameSize() << "\n");

    LLVM_DEBUG(
        dbgs() << " callee "
               << DumpHermesValue(
                      runtime.getCurrentFrame().getCalleeClosureOrCBRef())
               << "\n");
    LLVM_DEBUG(
        dbgs() << "   this "
               << DumpHermesValue(runtime.getCurrentFrame().getThisArgRef())
               << "\n");
    for (uint32_t i = 0; i != runtime.getCurrentFrame()->getArgCount(); ++i) {
      LLVM_DEBUG(
          dbgs() << "   " << llvh::format_decimal(i, 4) << " "
                 << DumpHermesValue(runtime.getCurrentFrame().getArgRef(i))
                 << "\n");
    }
#endif

    // Allocate the registers for the new frame.
    if (LLVM_UNLIKELY(!runtime.checkAndAllocStack(
            curCodeBlock->getFrameSize() +
                StackFrameLayout::CalleeExtraRegistersAtStart,
            HermesValue::encodeUndefinedValue())))
      goto stackOverflow;

    ip = (Inst const *)curCodeBlock->begin();

    // Check for invalid invocation.
    if (LLVM_UNLIKELY(curCodeBlock->getHeaderFlags().isCallProhibited(
            newFrame.isConstructorCall()))) {
      if (!newFrame.isConstructorCall()) {
        CAPTURE_IP(
            runtime.raiseTypeError("Class constructor invoked without new"));
      } else {
        CAPTURE_IP(runtime.raiseTypeError("Function is not a constructor"));
      }
      goto handleExceptionInParent;
    }
  } else {
    // Point frameRegs to the first register in the frame.
    frameRegs = &runtime.getCurrentFrame().getFirstLocalRef();
    ip = (Inst const *)(curCodeBlock->begin() + state.offset);
  }

  assert((const uint8_t *)ip < curCodeBlock->end() && "CodeBlock is empty");

  INIT_STATE_FOR_CODEBLOCK(curCodeBlock);

#define BEFORE_OP_CODE                                                       \
  {                                                                          \
    UPDATE_OPCODE_TIME_SPENT;                                                \
    HERMES_SLOW_ASSERT(                                                      \
        curCodeBlock->contains(ip) && "curCodeBlock must contain ip");       \
    HERMES_SLOW_ASSERT((printDebugInfo(curCodeBlock, frameRegs, ip), true)); \
    HERMES_SLOW_ASSERT(                                                      \
        gcScope.getHandleCountDbg() == KEEP_HANDLES &&                       \
        "unaccounted handles were created");                                 \
    HERMES_SLOW_ASSERT(tmpHandle->isUndefined() && "tmpHandle not cleared"); \
    RECORD_OPCODE_START_TIME;                                                \
    INC_OPCODE_COUNT;                                                        \
    if (EnableCrashTrace) {                                                  \
      runtime.crashTrace_.recordInst(                                        \
          (uint32_t)((const uint8_t *)ip - bytecodeFileStart), ip->opCode);  \
    }                                                                        \
  }

#ifdef HERMESVM_INDIRECT_THREADING
  static void *opcodeDispatch[] = {
#define DEFINE_OPCODE(name) &&case_##name,
#include "hermes/BCGen/HBC/BytecodeList.def"
      &&case__last};

#define CASE(name) case_##name:
// For indirect threading, there is no way to specify a default, leave it as
// an empty label.
#define DEFAULT_CASE
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
#define DEFAULT_CASE default:
#define DISPATCH                                \
  if (SingleStep) {                             \
    state.codeBlock = curCodeBlock;             \
    state.offset = CUROFFSET;                   \
    return HermesValue::encodeUndefinedValue(); \
  }                                             \
  continue

#endif // HERMESVM_INDIRECT_THREADING

#define RUN_DEBUGGER_ASYNC_BREAK(flags)                                      \
  do {                                                                       \
    CAPTURE_IP_ASSIGN(                                                       \
        auto dRes,                                                           \
        runDebuggerUpdatingState(                                            \
            (uint8_t)(flags) &                                               \
                    (uint8_t)Runtime::AsyncBreakReasonBits::DebuggerExplicit \
                ? Debugger::RunReason::AsyncBreakExplicit                    \
                : Debugger::RunReason::AsyncBreakImplicit,                   \
            runtime,                                                         \
            curCodeBlock,                                                    \
            ip,                                                              \
            frameRegs));                                                     \
    if (dRes == ExecutionStatus::EXCEPTION)                                  \
      goto exception;                                                        \
  } while (0)

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
      HermesValue::RawType callNewTarget;

/// Handle an opcode \p name with an out-of-line implementation in a function
///   ExecutionStatus caseName(
///       Runtime &,
///       PinnedHermesValue *frameRegs,
///       Inst *ip)
#define CASE_OUTOFLINE(name)                                         \
  CASE(name) {                                                       \
    CAPTURE_IP_ASSIGN(auto res, case##name(runtime, frameRegs, ip)); \
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {          \
      goto exception;                                                \
    }                                                                \
    gcScope.flushToSmallCount(KEEP_HANDLES);                         \
    ip = NEXTINST(name);                                             \
    DISPATCH;                                                        \
  }

/// Implement a binary arithmetic instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction. The fast path case will have a
///     "n" appended to the name.
#define BINOP(name)                                                      \
  CASE(name) {                                                           \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) { \
      /* Fast-path. */                                                   \
      CASE(name##N) {                                                    \
        O1REG(name) = HermesValue::encodeDoubleValue(                    \
            do##name(O2REG(name).getNumber(), O3REG(name).getNumber())); \
        ip = NEXTINST(name);                                             \
        DISPATCH;                                                        \
      }                                                                  \
    }                                                                    \
    CAPTURE_IP(                                                          \
        res = doOperSlowPath<do##name>(                                  \
            runtime, Handle<>(&O2REG(name)), Handle<>(&O3REG(name))));   \
    if (res == ExecutionStatus::EXCEPTION)                               \
      goto exception;                                                    \
    O1REG(name) = *res;                                                  \
    gcScope.flushToSmallCount(KEEP_HANDLES);                             \
    ip = NEXTINST(name);                                                 \
    DISPATCH;                                                            \
  }

#define INCDECOP(name)                                                        \
  CASE(name) {                                                                \
    if (LLVM_LIKELY(O2REG(name).isNumber())) {                                \
      O1REG(name) =                                                           \
          HermesValue::encodeDoubleValue(do##name(O2REG(name).getNumber()));  \
      gcScope.flushToSmallCount(KEEP_HANDLES);                                \
      ip = NEXTINST(name);                                                    \
      DISPATCH;                                                               \
    }                                                                         \
    CAPTURE_IP(                                                               \
        res =                                                                 \
            doIncDecOperSlowPath<do##name>(runtime, Handle<>(&O2REG(name)))); \
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {                   \
      goto exception;                                                         \
    }                                                                         \
    O1REG(name) = *res;                                                       \
    gcScope.flushToSmallCount(KEEP_HANDLES);                                  \
    ip = NEXTINST(name);                                                      \
    DISPATCH;                                                                 \
  }

/// Implement a shift instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction.
#define SHIFTOP(name)                                                          \
  CASE(name) {                                                                 \
    if (LLVM_LIKELY(                                                           \
            O2REG(name).isNumber() &&                                          \
            O3REG(name).isNumber())) { /* Fast-path. */                        \
      auto lnum = hermes::truncateToInt32(O2REG(name).getNumber());            \
      uint32_t rnum = hermes::truncateToInt32(O3REG(name).getNumber()) & 0x1f; \
      O1REG(name) = HermesValue::encodeDoubleValue(do##name(lnum, rnum));      \
      ip = NEXTINST(name);                                                     \
      DISPATCH;                                                                \
    }                                                                          \
    CAPTURE_IP(                                                                \
        res = doShiftOperSlowPath<do##name>(                                   \
            runtime, Handle<>(&O2REG(name)), Handle<>(&O3REG(name))));         \
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {                    \
      goto exception;                                                          \
    }                                                                          \
    O1REG(name) = *res;                                                        \
    gcScope.flushToSmallCount(KEEP_HANDLES);                                   \
    ip = NEXTINST(name);                                                       \
    DISPATCH;                                                                  \
  }

/// Implement a binary bitwise instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction.
#define BITWISEBINOP(name)                                               \
  CASE(name) {                                                           \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) { \
      /* Fast-path. */                                                   \
      O1REG(name) = HermesValue::encodeDoubleValue(do##name(             \
          hermes::truncateToInt32(O2REG(name).getNumber()),              \
          hermes::truncateToInt32(O3REG(name).getNumber())));            \
      ip = NEXTINST(name);                                               \
      DISPATCH;                                                          \
    }                                                                    \
    CAPTURE_IP(                                                          \
        res = doBitOperSlowPath<do##name>(                               \
            runtime, Handle<>(&O2REG(name)), Handle<>(&O3REG(name))));   \
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {              \
      goto exception;                                                    \
    }                                                                    \
    O1REG(name) = *res;                                                  \
    gcScope.flushToSmallCount(KEEP_HANDLES);                             \
    ip = NEXTINST(name);                                                 \
    DISPATCH;                                                            \
  }

/// Implement a comparison instruction.
/// \param name the name of the instruction.
/// \param oper the C++ operator to use to actually perform the fast arithmetic
///     comparison.
/// \param operFuncName  function to call for the slow-path comparison.
#define CONDOP(name, oper, operFuncName)                                 \
  CASE(name) {                                                           \
    if (LLVM_LIKELY(O2REG(name).isNumber() && O3REG(name).isNumber())) { \
      /* Fast-path. */                                                   \
      O1REG(name) = HermesValue::encodeBoolValue(                        \
          O2REG(name).getNumber() oper O3REG(name).getNumber());         \
      ip = NEXTINST(name);                                               \
      DISPATCH;                                                          \
    }                                                                    \
    CAPTURE_IP(                                                          \
        boolRes = operFuncName(                                          \
            runtime, Handle<>(&O2REG(name)), Handle<>(&O3REG(name))));   \
    if (boolRes == ExecutionStatus::EXCEPTION)                           \
      goto exception;                                                    \
    gcScope.flushToSmallCount(KEEP_HANDLES);                             \
    O1REG(name) = HermesValue::encodeBoolValue(boolRes.getValue());      \
    ip = NEXTINST(name);                                                 \
    DISPATCH;                                                            \
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
    CAPTURE_IP(                                                           \
        boolRes = operFuncName(                                           \
            runtime,                                                      \
            Handle<>(&O2REG(name##suffix)),                               \
            Handle<>(&O3REG(name##suffix))));                             \
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
    CAPTURE_IP_ASSIGN(                                   \
        auto eqRes,                                      \
        abstractEqualityTest_RJS(                        \
            runtime,                                     \
            Handle<>(&O2REG(name##suffix)),              \
            Handle<>(&O3REG(name##suffix))));            \
    if (eqRes == ExecutionStatus::EXCEPTION) {           \
      goto exception;                                    \
    }                                                    \
    gcScope.flushToSmallCount(KEEP_HANDLES);             \
    if (*eqRes) {                                        \
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

#define LOAD_CONST_CAPTURE_IP(name, value) \
  CASE(name) {                             \
    CAPTURE_IP(O1REG(name) = value);       \
    ip = NEXTINST(name);                   \
    DISPATCH;                              \
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
          O1REG(CoerceThisNS) = runtime.global_;
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
          O1REG(LoadThisNS) = runtime.global_;
        } else {
          tmpHandle = FRAME.getThisArgRef();
          nextIP = NEXTINST(LoadThisNS);
          goto coerceThisSlowPath;
        }
        ip = NEXTINST(LoadThisNS);
        DISPATCH;
      }
    coerceThisSlowPath : {
      CAPTURE_IP(res = toObject(runtime, tmpHandle));
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
        StackFramePtr fr{runtime.stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call1);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call2) {
        callArgCount = 2;
        nextIP = NEXTINST(Call2);
        StackFramePtr fr{runtime.stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call2);
        fr.getArgRefUnsafe(0) = O4REG(Call2);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call3) {
        callArgCount = 3;
        nextIP = NEXTINST(Call3);
        StackFramePtr fr{runtime.stackPointer_};
        fr.getArgRefUnsafe(-1) = O3REG(Call3);
        fr.getArgRefUnsafe(0) = O4REG(Call3);
        fr.getArgRefUnsafe(1) = O5REG(Call3);
        callNewTarget = HermesValue::encodeUndefinedValue().getRaw();
        goto doCall;
      }

      CASE(Call4) {
        callArgCount = 4;
        nextIP = NEXTINST(Call4);
        StackFramePtr fr{runtime.stackPointer_};
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
        goto doCall;
      }

    doCall : {
#ifdef HERMES_ENABLE_DEBUGGER
      // Check for an async debugger request.
      if (uint8_t asyncFlags =
              runtime.testAndClearDebuggerAsyncBreakRequest()) {
        RUN_DEBUGGER_ASYNC_BREAK(asyncFlags);
        gcScope.flushToSmallCount(KEEP_HANDLES);
        DISPATCH;
      }
#endif

      // Subtract 1 from callArgCount as 'this' is considered an argument in the
      // instruction, but not in the frame.
      auto newFrame = StackFramePtr::initFrame(
          runtime.stackPointer_,
          FRAME,
          ip,
          curCodeBlock,
          callArgCount - 1,
          O2REG(Call),
          HermesValue::fromRaw(callNewTarget));
      (void)newFrame;

      SLOW_DEBUG(dumpCallArguments(dbgs(), runtime, newFrame));

      if (auto *func = dyn_vmcast<JSFunction>(O2REG(Call))) {
        assert(!SingleStep && "can't single-step a call");

#ifdef HERMES_MEMORY_INSTRUMENTATION
        runtime.pushCallStack(curCodeBlock, ip);
#endif

        CodeBlock *calleeBlock = func->getCodeBlock(runtime);
        CAPTURE_IP(calleeBlock->lazyCompile(runtime));
#if defined(HERMESVM_PROFILER_EXTERN)
        CAPTURE_IP(res = runtime.interpretFunction(calleeBlock));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(Call) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = nextIP;
        DISPATCH;
#else
        curCodeBlock = calleeBlock;
        CAPTURE_IP_SET();
        goto tailCall;
#endif
      }
      CAPTURE_IP(
          resPH = Interpreter::handleCallSlowPath(runtime, &O2REG(Call)));
      if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
        goto exception;
      }
      O1REG(Call) = std::move(resPH->get());
      SLOW_DEBUG(
          dbgs() << "native return value r" << (unsigned)ip->iCall.op1 << "="
                 << DumpHermesValue(O1REG(Call)) << "\n");
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CallDirect)
      CASE(CallDirectLongIndex) {
#ifdef HERMES_ENABLE_DEBUGGER
        // Check for an async debugger request.
        if (uint8_t asyncFlags =
                runtime.testAndClearDebuggerAsyncBreakRequest()) {
          RUN_DEBUGGER_ASYNC_BREAK(asyncFlags);
          gcScope.flushToSmallCount(KEEP_HANDLES);
          DISPATCH;
        }
#endif

        CAPTURE_IP_ASSIGN(
            CodeBlock * calleeBlock,
            ip->opCode == OpCode::CallDirect
                ? curCodeBlock->getRuntimeModule()->getCodeBlockMayAllocate(
                      ip->iCallDirect.op3)
                : curCodeBlock->getRuntimeModule()->getCodeBlockMayAllocate(
                      ip->iCallDirectLongIndex.op3));

        auto newFrame = StackFramePtr::initFrame(
            runtime.stackPointer_,
            FRAME,
            ip,
            curCodeBlock,
            (uint32_t)ip->iCallDirect.op2 - 1,
            HermesValue::encodeNativePointer(calleeBlock),
            HermesValue::encodeUndefinedValue());
        (void)newFrame;

        LLVM_DEBUG(dumpCallArguments(dbgs(), runtime, newFrame));

        assert(!SingleStep && "can't single-step a call");

        CAPTURE_IP(calleeBlock->lazyCompile(runtime));
#if defined(HERMESVM_PROFILER_EXTERN)
        CAPTURE_IP(res = runtime.interpretFunction(calleeBlock));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CallDirect) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = ip->opCode == OpCode::CallDirect ? NEXTINST(CallDirect)
                                              : NEXTINST(CallDirectLongIndex);
        DISPATCH;
#else
        curCodeBlock = calleeBlock;
        CAPTURE_IP_SET();
        goto tailCall;
#endif
      }

      CASE(GetBuiltinClosure) {
        uint8_t methodIndex = ip->iCallBuiltin.op2;
        Callable *closure = runtime.getBuiltinCallable(methodIndex);
        O1REG(GetBuiltinClosure) = HermesValue::encodeObjectValue(closure);
        ip = NEXTINST(GetBuiltinClosure);
        DISPATCH;
      }

      CASE(CallBuiltin) {
        CAPTURE_IP_ASSIGN(
            auto cres,
            implCallBuiltin(
                runtime, frameRegs, curCodeBlock, ip->iCallBuiltin.op3));
        if (LLVM_UNLIKELY(cres == ExecutionStatus::EXCEPTION))
          goto exception;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CallBuiltin);
        DISPATCH;
      }
      CASE(CallBuiltinLong) {
        CAPTURE_IP_ASSIGN(
            auto cres,
            implCallBuiltin(
                runtime, frameRegs, curCodeBlock, ip->iCallBuiltinLong.op3));
        if (LLVM_UNLIKELY(cres == ExecutionStatus::EXCEPTION))
          goto exception;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CallBuiltinLong);
        DISPATCH;
      }

      CASE(CompleteGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime.getCurrentFrame().getCalleeClosureUnsafe());
        innerFn->setState(GeneratorInnerFunction::State::Completed);
        ip = NEXTINST(CompleteGenerator);
        DISPATCH;
      }

      CASE(SaveGenerator) {
        DONT_CAPTURE_IP(
            saveGenerator(runtime, frameRegs, IPADD(ip->iSaveGenerator.op1)));
        ip = NEXTINST(SaveGenerator);
        DISPATCH;
      }
      CASE(SaveGeneratorLong) {
        DONT_CAPTURE_IP(saveGenerator(
            runtime, frameRegs, IPADD(ip->iSaveGeneratorLong.op1)));
        ip = NEXTINST(SaveGeneratorLong);
        DISPATCH;
      }

      CASE(StartGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime.getCurrentFrame().getCalleeClosureUnsafe());
        if (innerFn->getState() ==
            GeneratorInnerFunction::State::SuspendedStart) {
          nextIP = NEXTINST(StartGenerator);
        } else {
          nextIP = innerFn->getNextIP(runtime);
          innerFn->restoreStack(runtime);
        }
        innerFn->setState(GeneratorInnerFunction::State::Executing);
        ip = nextIP;
        DISPATCH;
      }

      CASE(ResumeGenerator) {
        auto *innerFn = vmcast<GeneratorInnerFunction>(
            runtime.getCurrentFrame().getCalleeClosureUnsafe());
        O1REG(ResumeGenerator) = innerFn->getResult().unboxToHV(runtime);
        O2REG(ResumeGenerator) = HermesValue::encodeBoolValue(
            innerFn->getAction() == GeneratorInnerFunction::Action::Return);
        innerFn->clearResult(runtime);
        if (innerFn->getAction() == GeneratorInnerFunction::Action::Throw) {
          runtime.setThrownValue(O1REG(ResumeGenerator));
          goto exception;
        }
        ip = NEXTINST(ResumeGenerator);
        DISPATCH;
      }

      CASE(Ret) {
#ifdef HERMES_ENABLE_DEBUGGER
        // Check for an async debugger request.
        if (uint8_t asyncFlags =
                runtime.testAndClearDebuggerAsyncBreakRequest()) {
          RUN_DEBUGGER_ASYNC_BREAK(asyncFlags);
          gcScope.flushToSmallCount(KEEP_HANDLES);
          DISPATCH;
        }
#endif

        PROFILER_EXIT_FUNCTION(curCodeBlock);

#ifdef HERMES_MEMORY_INSTRUMENTATION
        runtime.popCallStack();
#endif

        // Store the return value.
        res = O1REG(Ret);

        ip = FRAME.getSavedIP();
        curCodeBlock = FRAME.getSavedCodeBlock();

        frameRegs =
            &runtime.restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

        SLOW_DEBUG(
            dbgs() << "function exit: restored stackLevel="
                   << runtime.getStackLevel() << "\n");

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
        assert(!runtime.thrownValue_.isEmpty() && "Invalid thrown value");
        assert(
            !isUncatchableError(runtime.thrownValue_) &&
            "Uncatchable thrown value was caught");
        O1REG(Catch) = runtime.thrownValue_;
        runtime.clearThrownValue();
#ifdef HERMES_ENABLE_DEBUGGER
        // Signal to the debugger that we're done unwinding an exception,
        // and we can resume normal debugging flow.
        runtime.debugger_.finishedUnwindingException();
#endif
        ip = NEXTINST(Catch);
        DISPATCH;
      }

      CASE(Throw) {
        runtime.thrownValue_ = O1REG(Throw);
        SLOW_DEBUG(
            dbgs() << "Exception thrown: "
                   << DumpHermesValue(runtime.thrownValue_) << "\n");
        goto exception;
      }

      CASE(ThrowIfEmpty) {
        if (LLVM_UNLIKELY(O2REG(ThrowIfEmpty).isEmpty())) {
          SLOW_DEBUG(dbgs() << "Throwing ReferenceError for empty variable");
          CAPTURE_IP(runtime.raiseReferenceError(
              "accessing an uninitialized variable"));
          goto exception;
        }
        O1REG(ThrowIfEmpty) = O2REG(ThrowIfEmpty);
        ip = NEXTINST(ThrowIfEmpty);
        DISPATCH;
      }

      CASE(Debugger) {
        SLOW_DEBUG(dbgs() << "debugger statement executed\n");
#ifdef HERMES_ENABLE_DEBUGGER
        {
          if (!runtime.debugger_.isDebugging()) {
            // Only run the debugger if we're not already debugging.
            // Don't want to call it again and mess with its state.
            CAPTURE_IP_ASSIGN(
                auto res,
                runDebuggerUpdatingState(
                    Debugger::RunReason::Opcode,
                    runtime,
                    curCodeBlock,
                    ip,
                    frameRegs));
            if (res == ExecutionStatus::EXCEPTION) {
              // If one of the internal steps threw,
              // then handle that here by jumping to where we're supposed to go.
              // If we're in mid-step, the breakpoint at the catch point
              // will have been set by the debugger.
              // We don't want to execute this instruction because it's already
              // thrown.
              goto exception;
            }
          }
          auto breakpointOpt = runtime.debugger_.getBreakpointLocation(ip);
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
              CAPTURE_IP_ASSIGN(
                  ExecutionStatus status, runtime.stepFunction(newState));
              curCodeBlock->installBreakpointAtOffset(CUROFFSET);
              if (status == ExecutionStatus::EXCEPTION) {
                goto exception;
              }
              curCodeBlock = newState.codeBlock;
              ip = newState.codeBlock->getOffsetPtr(newState.offset);
              INIT_STATE_FOR_CODEBLOCK(curCodeBlock);
              // Single-stepping should handle call stack management for us.
              frameRegs = &runtime.getCurrentFrame().getFirstLocalRef();
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
        if (LLVM_UNLIKELY(runtime.hasAsyncBreak())) {
#ifdef HERMES_ENABLE_DEBUGGER
          if (uint8_t asyncFlags =
                  runtime.testAndClearDebuggerAsyncBreakRequest()) {
            RUN_DEBUGGER_ASYNC_BREAK(asyncFlags);
          }
#endif
          if (runtime.testAndClearTimeoutAsyncBreakRequest()) {
            CAPTURE_IP_ASSIGN(auto nRes, runtime.notifyTimeout());
            if (nRes == ExecutionStatus::EXCEPTION) {
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
        SLOW_DEBUG(llvh::dbgs() << "ProfilePoint: " << pointIndex << "\n");
        CAPTURE_IP(runtime.getBasicBlockExecutionInfo().executeBlock(
            curCodeBlock, pointIndex));
#endif
        ip = NEXTINST(ProfilePoint);
        DISPATCH;
      }

      // Use a macro here to avoid clang-format issues with a literal default:
      // label.
      DEFAULT_CASE
      CASE(Unreachable) {
        hermes_fatal("Unreachable instruction encountered");
        // The fatal call doesn't return, no need to set the IP differently and
        // dispatch.
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
      CAPTURE_IP(
          O1REG(CreateClosure) =
              JSFunction::create(
                  runtime,
                  runtimeModule->getDomain(runtime),
                  Handle<JSObject>::vmcast(&runtime.functionPrototype),
                  Handle<Environment>::vmcast(&O2REG(CreateClosure)),
                  runtimeModule->getCodeBlockMayAllocate(idVal))
                  .getHermesValue());
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CreateAsyncClosure) {
        idVal = ip->iCreateAsyncClosure.op3;
        nextIP = NEXTINST(CreateAsyncClosure);
        goto createAsyncClosure;
      }
      CASE(CreateAsyncClosureLongIndex) {
        idVal = ip->iCreateAsyncClosureLongIndex.op3;
        nextIP = NEXTINST(CreateAsyncClosureLongIndex);
        goto createAsyncClosure;
      }
    createAsyncClosure : {
      auto *runtimeModule = curCodeBlock->getRuntimeModule();
      CAPTURE_IP_ASSIGN(
          O1REG(CreateAsyncClosure),
          JSAsyncFunction::create(
              runtime,
              runtimeModule->getDomain(runtime),
              Handle<JSObject>::vmcast(&runtime.asyncFunctionPrototype),
              Handle<Environment>::vmcast(&O2REG(CreateAsyncClosure)),
              runtimeModule->getCodeBlockMayAllocate(idVal))
              .getHermesValue());
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CreateGeneratorClosure) {
        idVal = ip->iCreateGeneratorClosure.op3;
        nextIP = NEXTINST(CreateGeneratorClosure);
        goto createGeneratorClosure;
      }
      CASE(CreateGeneratorClosureLongIndex) {
        idVal = ip->iCreateGeneratorClosureLongIndex.op3;
        nextIP = NEXTINST(CreateGeneratorClosureLongIndex);
        goto createGeneratorClosure;
      }
    createGeneratorClosure : {
      auto *runtimeModule = curCodeBlock->getRuntimeModule();
      CAPTURE_IP_ASSIGN(
          O1REG(CreateGeneratorClosure),
          JSGeneratorFunction::create(
              runtime,
              runtimeModule->getDomain(runtime),
              Handle<JSObject>::vmcast(&runtime.generatorFunctionPrototype),
              Handle<Environment>::vmcast(&O2REG(CreateGeneratorClosure)),
              runtimeModule->getCodeBlockMayAllocate(idVal))
              .getHermesValue());
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(CreateGenerator) {
        CAPTURE_IP_ASSIGN(
            auto res,
            createGenerator_RJS(
                runtime,
                curCodeBlock->getRuntimeModule(),
                ip->iCreateGenerator.op3,
                Handle<Environment>::vmcast(&O2REG(CreateGenerator)),
                FRAME.getNativeArgs()));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGenerator) = res->getHermesValue();
        res->invalidate();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(CreateGenerator);
        DISPATCH;
      }
      CASE(CreateGeneratorLongIndex) {
        CAPTURE_IP_ASSIGN(
            auto res,
            createGenerator_RJS(
                runtime,
                curCodeBlock->getRuntimeModule(),
                ip->iCreateGeneratorLongIndex.op3,
                Handle<Environment>::vmcast(&O2REG(CreateGeneratorLongIndex)),
                FRAME.getNativeArgs()));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(CreateGeneratorLongIndex) = res->getHermesValue();
        res->invalidate();
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
        tmpHandle = HermesValue::encodeObjectValueUnsafe(
            FRAME.getCalleeClosureUnsafe()->getEnvironment(runtime));

        CAPTURE_IP(
            res = Environment::create(
                runtime,
                Handle<Environment>::vmcast_or_null(tmpHandle),
                curCodeBlock->getEnvironmentSize()));
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
            .set(O3REG(StoreToEnvironment), runtime.getHeap());
        ip = NEXTINST(StoreToEnvironment);
        DISPATCH;
      }
      CASE(StoreToEnvironmentL) {
        vmcast<Environment>(O1REG(StoreToEnvironmentL))
            ->slot(ip->iStoreToEnvironmentL.op2)
            .set(O3REG(StoreToEnvironmentL), runtime.getHeap());
        ip = NEXTINST(StoreToEnvironmentL);
        DISPATCH;
      }

      CASE(StoreNPToEnvironment) {
        vmcast<Environment>(O1REG(StoreNPToEnvironment))
            ->slot(ip->iStoreNPToEnvironment.op2)
            .setNonPtr(O3REG(StoreNPToEnvironment), runtime.getHeap());
        ip = NEXTINST(StoreNPToEnvironment);
        DISPATCH;
      }
      CASE(StoreNPToEnvironmentL) {
        vmcast<Environment>(O1REG(StoreNPToEnvironmentL))
            ->slot(ip->iStoreNPToEnvironmentL.op2)
            .setNonPtr(O3REG(StoreNPToEnvironmentL), runtime.getHeap());
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
        O1REG(GetGlobalObject) = runtime.global_;
        ip = NEXTINST(GetGlobalObject);
        DISPATCH;
      }

      CASE(GetNewTarget) {
        O1REG(GetNewTarget) = FRAME.getNewTargetRef();
        ip = NEXTINST(GetNewTarget);
        DISPATCH;
      }

      CASE(DeclareGlobalVar) {
        DefinePropertyFlags dpf =
            DefinePropertyFlags::getDefaultNewPropertyFlags();
        dpf.configurable = 0;
        // Do not overwrite existing globals with undefined.
        dpf.setValue = 0;

        CAPTURE_IP_ASSIGN(
            auto res,
            JSObject::defineOwnProperty(
                runtime.getGlobal(),
                runtime,
                ID(ip->iDeclareGlobalVar.op1),
                dpf,
                Runtime::getUndefinedValue(),
                PropOpFlags().plusThrowOnError()));
        if (res == ExecutionStatus::EXCEPTION) {
          assert(
              !runtime.getGlobal()->isProxyObject() &&
              "global can't be a proxy object");
          // If the property already exists, this should be a noop.
          // Instead of incurring the cost to check every time, do it
          // only if an exception is thrown, and swallow the exception
          // if it exists, since we didn't want to make the call,
          // anyway.  This most likely means the property is
          // non-configurable.
          NamedPropertyDescriptor desc;
          CAPTURE_IP_ASSIGN(
              auto res,
              JSObject::getOwnNamedDescriptor(
                  runtime.getGlobal(),
                  runtime,
                  ID(ip->iDeclareGlobalVar.op1),
                  desc));
          if (!res) {
            goto exception;
          } else {
            runtime.clearThrownValue();
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
      if (LLVM_LIKELY(O2REG(GetById).isObject())) {
        auto *obj = vmcast<JSObject>(O2REG(GetById));
        auto cacheIdx = ip->iGetById.op3;
        auto *cacheEntry = curCodeBlock->getReadCacheEntry(cacheIdx);

#ifdef HERMESVM_PROFILER_BB
        {
          HERMES_SLOW_ASSERT(
              gcScope.getHandleCountDbg() == KEEP_HANDLES &&
              "unaccounted handles were created");
          auto objHandle = runtime.makeHandle(obj);
          auto cacheHCPtr = vmcast_or_null<HiddenClass>(static_cast<GCCell *>(
              cacheEntry->clazz.get(runtime, runtime.getHeap())));
          CAPTURE_IP(runtime.recordHiddenClass(
              curCodeBlock, ip, ID(idVal), obj->getClass(runtime), cacheHCPtr));
          // obj may be moved by GC due to recordHiddenClass
          obj = objHandle.get();
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
        CompressedPointer clazzPtr{obj->getClassGCPtr()};
#ifndef NDEBUG
        if (vmcast<HiddenClass>(clazzPtr.getNonNull(runtime))->isDictionary())
          ++NumGetByIdDict;
#else
        (void)NumGetByIdDict;
#endif

        // If we have a cache hit, reuse the cached offset and immediately
        // return the property.
        if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
          ++NumGetByIdCacheHits;
          CAPTURE_IP(
              O1REG(GetById) =
                  JSObject::getNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
                      obj, runtime, cacheEntry->slot)
                      .unboxToHV(runtime));
          ip = nextIP;
          DISPATCH;
        }
        auto id = ID(idVal);
        NamedPropertyDescriptor desc;
        CAPTURE_IP_ASSIGN(
            OptValue<bool> fastPathResult,
            JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc));
        if (LLVM_LIKELY(
                fastPathResult.hasValue() && fastPathResult.getValue()) &&
            !desc.flags.accessor) {
          ++NumGetByIdFastPaths;

          // cacheIdx == 0 indicates no caching so don't update the cache in
          // those cases.
          HiddenClass *clazz =
              vmcast<HiddenClass>(clazzPtr.getNonNull(runtime));
          if (LLVM_LIKELY(!clazz->isDictionaryNoCache()) &&
              LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
#ifdef HERMES_SLOW_DEBUG
            if (cacheEntry->clazz && cacheEntry->clazz != clazzPtr)
              ++NumGetByIdCacheEvicts;
#else
            (void)NumGetByIdCacheEvicts;
#endif
            // Cache the class, id and property slot.
            cacheEntry->clazz = clazzPtr;
            cacheEntry->slot = desc.slot;
          }

          assert(
              !obj->isProxyObject() &&
              "tryGetOwnNamedDescriptorFast returned true on Proxy");
          CAPTURE_IP(
              O1REG(GetById) =
                  JSObject::getNamedSlotValueUnsafe(obj, runtime, desc)
                      .unboxToHV(runtime));
          ip = nextIP;
          DISPATCH;
        }

        // The cache may also be populated via the prototype of the object.
        // This value is only reliable if the fast path was a definite
        // not-found.
        if (fastPathResult.hasValue() && !fastPathResult.getValue() &&
            LLVM_LIKELY(!obj->isProxyObject())) {
          CAPTURE_IP_ASSIGN(JSObject * parent, obj->getParent(runtime));
          // TODO: This isLazy check is because a lazy object is reported as
          // having no properties and therefore cannot contain the property.
          // This check does not belong here, it should be merged into
          // tryGetOwnNamedDescriptorFast().
          if (parent && cacheEntry->clazz == parent->getClassGCPtr() &&
              LLVM_LIKELY(!obj->isLazy())) {
            ++NumGetByIdProtoHits;
            // We've already checked that this isn't a Proxy.
            CAPTURE_IP(
                O1REG(GetById) = JSObject::getNamedSlotValueUnsafe(
                                     parent, runtime, cacheEntry->slot)
                                     .unboxToHV(runtime));
            ip = nextIP;
            DISPATCH;
          }
        }

#ifdef HERMES_SLOW_DEBUG
        // Call to getNamedDescriptorUnsafe is safe because `id` is kept alive
        // by the IdentifierTable.
        CAPTURE_IP_ASSIGN(
            JSObject * propObj,
            JSObject::getNamedDescriptorUnsafe(
                Handle<JSObject>::vmcast(&O2REG(GetById)), runtime, id, desc));
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
#ifdef HERMES_SLOW_DEBUG
        auto *savedClass = cacheIdx != hbc::PROPERTY_CACHING_DISABLED
            ? cacheEntry->clazz.get(runtime, runtime.getHeap())
            : nullptr;
#endif
        ++NumGetByIdSlow;
        CAPTURE_IP(
            resPH = JSObject::getNamed_RJS(
                Handle<JSObject>::vmcast(&O2REG(GetById)),
                runtime,
                id,
                !tryProp ? defaultPropOpFlags
                         : defaultPropOpFlags.plusMustExist(),
                cacheIdx != hbc::PROPERTY_CACHING_DISABLED ? cacheEntry
                                                           : nullptr));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
#ifdef HERMES_SLOW_DEBUG
        if (cacheIdx != hbc::PROPERTY_CACHING_DISABLED && savedClass &&
            cacheEntry->clazz.get(runtime, runtime.getHeap()) != savedClass) {
          ++NumGetByIdCacheEvicts;
        }
#endif
      } else {
        ++NumGetByIdTransient;
        assert(!tryProp && "TryGetById can only be used on the global object");
        /* Slow path. */
        CAPTURE_IP(
            resPH = Interpreter::getByIdTransient_RJS(
                runtime, Handle<>(&O2REG(GetById)), ID(idVal)));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
      }
      O1REG(GetById) = resPH->get();
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
        CAPTURE_IP_ASSIGN(
            SmallHermesValue shv,
            SmallHermesValue::encodeHermesValue(O2REG(PutById), runtime));
        auto *obj = vmcast<JSObject>(O1REG(PutById));
        auto cacheIdx = ip->iPutById.op3;
        auto *cacheEntry = curCodeBlock->getWriteCacheEntry(cacheIdx);

#ifdef HERMESVM_PROFILER_BB
        {
          HERMES_SLOW_ASSERT(
              gcScope.getHandleCountDbg() == KEEP_HANDLES &&
              "unaccounted handles were created");
          auto shvHandle = runtime.makeHandle(shv.toHV(runtime));
          auto objHandle = runtime.makeHandle(obj);
          auto cacheHCPtr = vmcast_or_null<HiddenClass>(static_cast<GCCell *>(
              cacheEntry->clazz.get(runtime, runtime.getHeap())));
          CAPTURE_IP(runtime.recordHiddenClass(
              curCodeBlock, ip, ID(idVal), obj->getClass(runtime), cacheHCPtr));
          // shv/obj may be invalidated by recordHiddenClass
          if (shv.isPointer())
            shv.unsafeUpdatePointer(
                static_cast<GCCell *>(shvHandle->getPointer()), runtime);
          obj = objHandle.get();
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
        CompressedPointer clazzPtr{obj->getClassGCPtr()};
        // If we have a cache hit, reuse the cached offset and immediately
        // return the property.
        if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
          ++NumPutByIdCacheHits;
          CAPTURE_IP(
              JSObject::setNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
                  obj, runtime, cacheEntry->slot, shv));
          ip = nextIP;
          DISPATCH;
        }
        auto id = ID(idVal);
        NamedPropertyDescriptor desc;
        CAPTURE_IP_ASSIGN(
            OptValue<bool> hasOwnProp,
            JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc));
        if (LLVM_LIKELY(hasOwnProp.hasValue() && hasOwnProp.getValue()) &&
            !desc.flags.accessor && desc.flags.writable &&
            !desc.flags.internalSetter) {
          ++NumPutByIdFastPaths;

          // cacheIdx == 0 indicates no caching so don't update the cache in
          // those cases.
          HiddenClass *clazz =
              vmcast<HiddenClass>(clazzPtr.getNonNull(runtime));
          if (LLVM_LIKELY(!clazz->isDictionary()) &&
              LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
#ifdef HERMES_SLOW_DEBUG
            if (cacheEntry->clazz && cacheEntry->clazz != clazzPtr)
              ++NumPutByIdCacheEvicts;
#else
            (void)NumPutByIdCacheEvicts;
#endif
            // Cache the class and property slot.
            cacheEntry->clazz = clazzPtr;
            cacheEntry->slot = desc.slot;
          }

          // This must be valid because an own property was already found.
          CAPTURE_IP(
              JSObject::setNamedSlotValueUnsafe(obj, runtime, desc.slot, shv));
          ip = nextIP;
          DISPATCH;
        }

        CAPTURE_IP_ASSIGN(
            auto putRes,
            JSObject::putNamed_RJS(
                Handle<JSObject>::vmcast(&O1REG(PutById)),
                runtime,
                id,
                Handle<>(&O2REG(PutById)),
                !tryProp ? defaultPropOpFlags
                         : defaultPropOpFlags.plusMustExist()));
        if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
      } else {
        ++NumPutByIdTransient;
        assert(!tryProp && "TryPutById can only be used on the global object");
        CAPTURE_IP_ASSIGN(
            auto retStatus,
            Interpreter::putByIdTransient_RJS(
                runtime,
                Handle<>(&O1REG(PutById)),
                ID(idVal),
                Handle<>(&O2REG(PutById)),
                strictMode));
        if (retStatus == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
      }
      gcScope.flushToSmallCount(KEEP_HANDLES);
      ip = nextIP;
      DISPATCH;
    }

      CASE(GetByVal) {
        if (LLVM_LIKELY(O2REG(GetByVal).isObject())) {
          CAPTURE_IP(
              resPH = JSObject::getComputed_RJS(
                  Handle<JSObject>::vmcast(&O2REG(GetByVal)),
                  runtime,
                  Handle<>(&O3REG(GetByVal))));
          if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        } else {
          // This is the "slow path".
          CAPTURE_IP(
              resPH = Interpreter::getByValTransient_RJS(
                  runtime,
                  Handle<>(&O2REG(GetByVal)),
                  Handle<>(&O3REG(GetByVal))));
          if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetByVal) = resPH->get();
        ip = NEXTINST(GetByVal);
        DISPATCH;
      }

      CASE(PutByVal) {
        if (LLVM_LIKELY(O1REG(PutByVal).isObject())) {
          CAPTURE_IP_ASSIGN(
              auto putRes,
              JSObject::putComputed_RJS(
                  Handle<JSObject>::vmcast(&O1REG(PutByVal)),
                  runtime,
                  Handle<>(&O2REG(PutByVal)),
                  Handle<>(&O3REG(PutByVal)),
                  defaultPropOpFlags));
          if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
        } else {
          // This is the "slow path".
          CAPTURE_IP_ASSIGN(
              auto retStatus,
              Interpreter::putByValTransient_RJS(
                  runtime,
                  Handle<>(&O1REG(PutByVal)),
                  Handle<>(&O2REG(PutByVal)),
                  Handle<>(&O3REG(PutByVal)),
                  strictMode));
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
      CAPTURE_IP(JSObject::defineOwnComputedPrimitive(
          Handle<JSObject>::vmcast(&O1REG(PutOwnByIndex)),
          runtime,
          tmpHandle,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          Handle<>(&O2REG(PutOwnByIndex))));
      gcScope.flushToSmallCount(KEEP_HANDLES);
      tmpHandle.clear();
      ip = nextIP;
      DISPATCH;
    }

      CASE_OUTOFLINE(GetPNameList);

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
          MutableHandle<SymbolID> tmpPropNameStorage{runtime};
          // Loop until we find a property which is present.
          while (idx < size) {
            tmpHandle = arr->at(runtime, idx);
            ComputedPropertyDescriptor desc;
            CAPTURE_IP_ASSIGN(
                ExecutionStatus status,
                JSObject::getComputedPrimitiveDescriptor(
                    obj,
                    runtime,
                    tmpHandle,
                    propObj,
                    tmpPropNameStorage,
                    desc));
            if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
              goto exception;
            }
            if (LLVM_LIKELY(propObj))
              break;
            ++idx;
          }
          if (idx < size) {
            // We must return the property as a string
            if (tmpHandle->isNumber()) {
              CAPTURE_IP_ASSIGN(auto status, toString_RJS(runtime, tmpHandle));
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
          CAPTURE_IP(res = toNumber_RJS(runtime, Handle<>(&O2REG(ToNumber))));
          if (res == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          O1REG(ToNumber) = res.getValue();
          ip = NEXTINST(ToNumber);
        }
        DISPATCH;
      }

      CASE(ToNumeric) {
        if (LLVM_LIKELY(O2REG(ToNumeric).isNumber())) {
          O1REG(ToNumeric) = O2REG(ToNumeric);
          ip = NEXTINST(ToNumeric);
        } else {
          CAPTURE_IP(res = toNumeric_RJS(runtime, Handle<>(&O2REG(ToNumeric))));
          if (res == ExecutionStatus::EXCEPTION)
            goto exception;
          gcScope.flushToSmallCount(KEEP_HANDLES);
          O1REG(ToNumeric) = res.getValue();
          ip = NEXTINST(ToNumeric);
        }
        DISPATCH;
      }

      CASE(ToInt32) {
        CAPTURE_IP(res = toInt32_RJS(runtime, Handle<>(&O2REG(ToInt32))));
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
          CAPTURE_IP(
              res = toPrimitive_RJS(
                  runtime,
                  Handle<>(&O2REG(AddEmptyString)),
                  PreferredType::NONE));
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
            goto exception;
          tmpHandle = res.getValue();
          CAPTURE_IP_ASSIGN(auto strRes, toString_RJS(runtime, tmpHandle));
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
      INCDECOP(Inc)
      INCDECOP(Dec)
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
        CAPTURE_IP(
            res = addOp_RJS(
                runtime, Handle<>(&O2REG(Add)), Handle<>(&O3REG(Add))));
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
        CAPTURE_IP(res = toNumeric_RJS(runtime, Handle<>(&O2REG(BitNot))));
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        if (res->isBigInt()) {
          CAPTURE_IP_ASSIGN(auto bigint, runtime.makeHandle(res->getBigInt()));
          CAPTURE_IP(res = BigIntPrimitive::unaryNOT(runtime, bigint));
          if (res == ExecutionStatus::EXCEPTION) {
            goto exception;
          }
          O1REG(Negate) = HermesValue::encodeBigIntValue(res->getBigInt());
        } else {
          CAPTURE_IP(res = toInt32_RJS(runtime, Handle<>(&O2REG(BitNot))));
          if (res == ExecutionStatus::EXCEPTION) {
            goto exception;
          }
          O1REG(BitNot) = HermesValue::encodeDoubleValue(
              ~static_cast<int32_t>(res->getNumber()));
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
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
        CAPTURE_IP(
            resPH = JSObject::getNamed_RJS(
                Handle<JSObject>::vmcast(&O2REG(GetArgumentsLength)),
                runtime,
                Predefined::getSymbolID(Predefined::length)));
        if (resPH == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetArgumentsLength) = resPH->get();
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
        CAPTURE_IP_ASSIGN(
            auto res,
            getArgumentsPropByValSlowPath_RJS(
                runtime,
                &O3REG(GetArgumentsPropByVal),
                &O2REG(GetArgumentsPropByVal),
                FRAME.getCalleeClosureHandleUnsafe(),
                strictMode));
        if (res == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(GetArgumentsPropByVal) = res->getHermesValue();
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
        CAPTURE_IP(
            resArgs = reifyArgumentsSlowPath(
                runtime, FRAME.getCalleeClosureHandleUnsafe(), strictMode));
        if (LLVM_UNLIKELY(resArgs == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(ReifyArguments) = resArgs->getHermesValue();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(ReifyArguments);
        DISPATCH;
      }

      CASE(NewObject) {
        // Create a new object using the built-in constructor. Note that the
        // built-in constructor is empty, so we don't actually need to call
        // it.
        CAPTURE_IP(
            O1REG(NewObject) = JSObject::create(runtime).getHermesValue());
        assert(
            gcScope.getHandleCountDbg() == KEEP_HANDLES &&
            "Should not create handles.");
        ip = NEXTINST(NewObject);
        DISPATCH;
      }
      CASE(NewObjectWithParent) {
        CAPTURE_IP(
            O1REG(NewObjectWithParent) =
                JSObject::create(
                    runtime,
                    O2REG(NewObjectWithParent).isObject()
                        ? Handle<JSObject>::vmcast(&O2REG(NewObjectWithParent))
                        : O2REG(NewObjectWithParent).isNull()
                        ? Runtime::makeNullHandle<JSObject>()
                        : Handle<JSObject>::vmcast(&runtime.objectPrototype))
                    .getHermesValue());
        assert(
            gcScope.getHandleCountDbg() == KEEP_HANDLES &&
            "Should not create handles.");
        ip = NEXTINST(NewObjectWithParent);
        DISPATCH;
      }

      CASE(NewObjectWithBuffer) {
        CAPTURE_IP(
            resPH = Interpreter::createObjectFromBuffer(
                runtime,
                curCodeBlock,
                ip->iNewObjectWithBuffer.op3,
                ip->iNewObjectWithBuffer.op4,
                ip->iNewObjectWithBuffer.op5));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewObjectWithBuffer) = resPH->get();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewObjectWithBuffer);
        DISPATCH;
      }

      CASE(NewObjectWithBufferLong) {
        CAPTURE_IP(
            resPH = Interpreter::createObjectFromBuffer(
                runtime,
                curCodeBlock,
                ip->iNewObjectWithBufferLong.op3,
                ip->iNewObjectWithBufferLong.op4,
                ip->iNewObjectWithBufferLong.op5));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewObjectWithBufferLong) = resPH->get();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewObjectWithBufferLong);
        DISPATCH;
      }

      CASE(NewArray) {
        // Create a new array using the built-in constructor. Note that the
        // built-in constructor is empty, so we don't actually need to call
        // it.
        {
          CAPTURE_IP_ASSIGN(
              auto createRes,
              JSArray::create(runtime, ip->iNewArray.op2, ip->iNewArray.op2));
          if (createRes == ExecutionStatus::EXCEPTION) {
            goto exception;
          }
          O1REG(NewArray) = createRes->getHermesValue();
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(NewArray);
        DISPATCH;
      }

      CASE(NewArrayWithBuffer) {
        CAPTURE_IP(
            resPH = Interpreter::createArrayFromBuffer(
                runtime,
                curCodeBlock,
                ip->iNewArrayWithBuffer.op2,
                ip->iNewArrayWithBuffer.op3,
                ip->iNewArrayWithBuffer.op4));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewArrayWithBuffer) = resPH->get();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(NewArrayWithBuffer);
        DISPATCH;
      }

      CASE(NewArrayWithBufferLong) {
        CAPTURE_IP(
            resPH = Interpreter::createArrayFromBuffer(
                runtime,
                curCodeBlock,
                ip->iNewArrayWithBufferLong.op2,
                ip->iNewArrayWithBufferLong.op3,
                ip->iNewArrayWithBufferLong.op4));
        if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(NewArrayWithBufferLong) = resPH->get();
        gcScope.flushToSmallCount(KEEP_HANDLES);
        tmpHandle.clear();
        ip = NEXTINST(NewArrayWithBufferLong);
        DISPATCH;
      }

      CASE(CreateThis) {
        // Registers: output, prototype, closure.
        if (LLVM_UNLIKELY(!vmisa<Callable>(O3REG(CreateThis)))) {
          CAPTURE_IP(runtime.raiseTypeError("constructor is not callable"));
          goto exception;
        }
        CAPTURE_IP_ASSIGN(
            auto res,
            Callable::newObject(
                Handle<Callable>::vmcast(&O3REG(CreateThis)),
                runtime,
                Handle<JSObject>::vmcast(
                    O2REG(CreateThis).isObject() ? &O2REG(CreateThis)
                                                 : &runtime.objectPrototype)));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(CreateThis) = res->getHermesValue();
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
        CAPTURE_IP_ASSIGN(
            auto eqRes,
            abstractEqualityTest_RJS(
                runtime, Handle<>(&O2REG(Eq)), Handle<>(&O3REG(Eq))));
        if (eqRes == ExecutionStatus::EXCEPTION) {
          goto exception;
        }
        gcScope.flushToSmallCount(KEEP_HANDLES);
        O1REG(Eq) = HermesValue::encodeBoolValue(
            ip->opCode == OpCode::Eq ? *eqRes : !*eqRes);
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
          CAPTURE_IP(res = toNumeric_RJS(runtime, Handle<>(&O2REG(Negate))));
          if (res == ExecutionStatus::EXCEPTION)
            goto exception;
          if (res->isNumber()) {
            O1REG(Negate) = HermesValue::encodeDoubleValue(-res->getNumber());
          } else {
            assert(res->isBigInt() && "should be bigint");
            CAPTURE_IP_ASSIGN(
                auto bigint, runtime.makeHandle(res->getBigInt()));
            CAPTURE_IP(res = BigIntPrimitive::unaryMinus(runtime, bigint));
            if (res == ExecutionStatus::EXCEPTION) {
              goto exception;
            }
            O1REG(Negate) = HermesValue::encodeBigIntValue(res->getBigInt());
          }
          gcScope.flushToSmallCount(KEEP_HANDLES);
        }
        ip = NEXTINST(Negate);
        DISPATCH;
      }
      CASE(TypeOf) {
        CAPTURE_IP(O1REG(TypeOf) = typeOf(runtime, Handle<>(&O2REG(TypeOf))));
        ip = NEXTINST(TypeOf);
        DISPATCH;
      }
      CASE(Mod) {
        if (LLVM_LIKELY(O2REG(Mod).isNumber() && O3REG(Mod).isNumber())) {
          /* Fast-path. */
          O1REG(Mod) = HermesValue::encodeDoubleValue(
              doMod(O2REG(Mod).getNumber(), O3REG(Mod).getNumber()));
          ip = NEXTINST(Mod);
          DISPATCH;
        }
        CAPTURE_IP(
            res = doOperSlowPath<doMod>(
                runtime, Handle<>(&O2REG(Mod)), Handle<>(&O3REG(Mod))));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(Mod) = *res;
        gcScope.flushToSmallCount(KEEP_HANDLES);
        ip = NEXTINST(Mod);
        DISPATCH;
      }
      CASE(InstanceOf) {
        CAPTURE_IP_ASSIGN(
            auto result,
            instanceOfOperator_RJS(
                runtime,
                Handle<>(&O2REG(InstanceOf)),
                Handle<>(&O3REG(InstanceOf))));
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
            CAPTURE_IP(runtime.raiseTypeError(
                "right operand of 'in' is not an object"));
            goto exception;
          }
          CAPTURE_IP_ASSIGN(
              auto cr,
              JSObject::hasComputed(
                  Handle<JSObject>::vmcast(&O3REG(IsIn)),
                  runtime,
                  Handle<>(&O2REG(IsIn))));
          if (cr == ExecutionStatus::EXCEPTION) {
            goto exception;
          }
          O1REG(IsIn) = HermesValue::encodeBoolValue(*cr);
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
      CAPTURE_IP_ASSIGN(
          auto res,
          JSObject::defineNewOwnProperty(
              Handle<JSObject>::vmcast(&O1REG(PutNewOwnById)),
              runtime,
              ID(idVal),
              ip->opCode <= OpCode::PutNewOwnByIdLong
                  ? PropertyFlags::defaultNewNamedPropertyFlags()
                  : PropertyFlags::nonEnumerablePropertyFlags(),
              Handle<>(&O2REG(PutNewOwnById))));
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
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
        CAPTURE_IP_ASSIGN(
            auto status,
            JSObject::deleteNamed(
                Handle<JSObject>::vmcast(&O2REG(DelById)),
                runtime,
                ID(idVal),
                defaultPropOpFlags));
        if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
          goto exception;
        }
        O1REG(DelById) = HermesValue::encodeBoolValue(status.getValue());
      } else {
        // This is the "slow path".
        CAPTURE_IP(res = toObject(runtime, Handle<>(&O2REG(DelById))));
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          // If an exception is thrown, likely we are trying to convert
          // undefined/null to an object. Passing over the name of the property
          // so that we could emit more meaningful error messages.
          CAPTURE_IP(amendPropAccessErrorMsgWithPropName(
              runtime, Handle<>(&O2REG(DelById)), "delete", ID(idVal)));
          goto exception;
        }
        tmpHandle = res.getValue();
        CAPTURE_IP_ASSIGN(
            auto status,
            JSObject::deleteNamed(
                Handle<JSObject>::vmcast(tmpHandle),
                runtime,
                ID(idVal),
                defaultPropOpFlags));
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
          CAPTURE_IP_ASSIGN(
              auto status,
              JSObject::deleteComputed(
                  Handle<JSObject>::vmcast(&O2REG(DelByVal)),
                  runtime,
                  Handle<>(&O3REG(DelByVal)),
                  defaultPropOpFlags));
          if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          O1REG(DelByVal) = HermesValue::encodeBoolValue(status.getValue());
        } else {
          // This is the "slow path".
          CAPTURE_IP(res = toObject(runtime, Handle<>(&O2REG(DelByVal))));
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
            goto exception;
          }
          tmpHandle = res.getValue();
          CAPTURE_IP_ASSIGN(
              auto status,
              JSObject::deleteComputed(
                  Handle<JSObject>::vmcast(tmpHandle),
                  runtime,
                  Handle<>(&O3REG(DelByVal)),
                  defaultPropOpFlags));
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
          CAPTURE_IP(
              O1REG(CreateRegExp) = JSRegExp::create(runtime).getHermesValue());
          auto re = Handle<JSRegExp>::vmcast(&O1REG(CreateRegExp));
          // Initialize the regexp.
          CAPTURE_IP_ASSIGN(
              auto pattern,
              runtime.makeHandle(curCodeBlock->getRuntimeModule()
                                     ->getStringPrimFromStringIDMayAllocate(
                                         ip->iCreateRegExp.op2)));
          CAPTURE_IP_ASSIGN(
              auto flags,
              runtime.makeHandle(curCodeBlock->getRuntimeModule()
                                     ->getStringPrimFromStringIDMayAllocate(
                                         ip->iCreateRegExp.op3)));
          CAPTURE_IP_ASSIGN(
              auto bytecode,
              curCodeBlock->getRuntimeModule()->getRegExpBytecodeFromRegExpID(
                  ip->iCreateRegExp.op4));
          CAPTURE_IP(
              JSRegExp::initialize(re, runtime, pattern, flags, bytecode));
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
            const uint8_t *tablestart = (const uint8_t *)llvh::alignAddr(
                (const uint8_t *)ip + ip->iSwitchImm.op2, sizeof(uint32_t));

            // Read the offset from the table.
            // Must be signed to account for backwards branching.
            const int32_t *loc =
                (const int32_t *)tablestart + uintVal - ip->iSwitchImm.op4;

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
      // LoadConstBigInt will always allocate a new (heap) object (unlike, e.g.,
      // LoadConstString). This could become an issue for code that uses lots of
      // BigInt literals in loops, in which case the VM may need to cache the
      // objects.
      LOAD_CONST_CAPTURE_IP(
          LoadConstBigInt,
          runtime.ignoreAllocationFailure(BigIntPrimitive::fromBytes(
              runtime,
              curCodeBlock->getRuntimeModule()->getBigIntBytesFromBigIntId(
                  ip->iLoadConstBigInt.op2))));
      LOAD_CONST_CAPTURE_IP(
          LoadConstBigIntLongIndex,
          runtime.ignoreAllocationFailure(BigIntPrimitive::fromBytes(
              runtime,
              curCodeBlock->getRuntimeModule()->getBigIntBytesFromBigIntId(
                  ip->iLoadConstBigIntLongIndex.op2))));
      LOAD_CONST_CAPTURE_IP(
          LoadConstString,
          HermesValue::encodeStringValue(
              curCodeBlock->getRuntimeModule()
                  ->getStringPrimFromStringIDMayAllocate(
                      ip->iLoadConstString.op2)));
      LOAD_CONST_CAPTURE_IP(
          LoadConstStringLongIndex,
          HermesValue::encodeStringValue(
              curCodeBlock->getRuntimeModule()
                  ->getStringPrimFromStringIDMayAllocate(
                      ip->iLoadConstStringLongIndex.op2)));
      LOAD_CONST(LoadConstEmpty, HermesValue::encodeEmptyValue());
      LOAD_CONST(LoadConstUndefined, HermesValue::encodeUndefinedValue());
      LOAD_CONST(LoadConstNull, HermesValue::encodeNullValue());
      LOAD_CONST(LoadConstTrue, HermesValue::encodeBoolValue(true));
      LOAD_CONST(LoadConstFalse, HermesValue::encodeBoolValue(false));
      LOAD_CONST(LoadConstZero, HermesValue::encodeDoubleValue(0));
      BINOP(Sub);
      BINOP(Mul);
      BINOP(Div);
      // Can't do BINOP(Mod) as there's no ModN opcode.
      BITWISEBINOP(BitAnd);
      BITWISEBINOP(BitOr);
      BITWISEBINOP(BitXor);
      SHIFTOP(LShift);
      SHIFTOP(RShift);
      SHIFTOP(URshift);
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

      CASE_OUTOFLINE(IteratorBegin);
      CASE_OUTOFLINE(IteratorNext);
      CASE(IteratorClose) {
        if (LLVM_UNLIKELY(O1REG(IteratorClose).isObject())) {
          // The iterator must be closed if it's still an object.
          // That means it was never an index and is not done iterating (a state
          // which is indicated by `undefined`).
          CAPTURE_IP_ASSIGN(
              auto res,
              iteratorClose(
                  runtime,
                  Handle<JSObject>::vmcast(&O1REG(IteratorClose)),
                  Runtime::getEmptyValue()));
          if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
            if (ip->iIteratorClose.op2 &&
                !isUncatchableError(runtime.thrownValue_)) {
              // Ignore inner exception.
              runtime.clearThrownValue();
            } else {
              goto exception;
            }
          }
          gcScope.flushToSmallCount(KEEP_HANDLES);
        }
        ip = NEXTINST(IteratorClose);
        DISPATCH;
      }

#ifdef HERMES_RUN_WASM
      // Asm.js/Wasm Intrinsics
      CASE(Add32) {
        O1REG(Add32) = HermesValue::encodeDoubleValue((
            int32_t)(int64_t)(O2REG(Add32).getNumber() + O3REG(Add32).getNumber()));
        ip = NEXTINST(Add32);
        DISPATCH;
      }
      CASE(Sub32) {
        O1REG(Sub32) = HermesValue::encodeDoubleValue((
            int32_t)(int64_t)(O2REG(Sub32).getNumber() - O3REG(Sub32).getNumber()));
        ip = NEXTINST(Sub32);
        DISPATCH;
      }
      CASE(Mul32) {
        // Signedness matters for multiplication, but low 32 bits are the same
        // regardless of signedness.
        const uint32_t arg0 = (uint32_t)(int32_t)(O2REG(Mul32).getNumber());
        const uint32_t arg1 = (uint32_t)(int32_t)(O3REG(Mul32).getNumber());
        O1REG(Mul32) = HermesValue::encodeDoubleValue((int32_t)(arg0 * arg1));
        ip = NEXTINST(Mul32);
        DISPATCH;
      }
      CASE(Divi32) {
        const int32_t arg0 = (int32_t)(O2REG(Divi32).getNumber());
        const int32_t arg1 = (int32_t)(O3REG(Divi32).getNumber());
        O1REG(Divi32) = HermesValue::encodeDoubleValue(arg0 / arg1);
        ip = NEXTINST(Divi32);
        DISPATCH;
      }
      CASE(Divu32) {
        const uint32_t arg0 = (uint32_t)(int32_t)(O2REG(Divu32).getNumber());
        const uint32_t arg1 = (uint32_t)(int32_t)(O3REG(Divu32).getNumber());
        O1REG(Divu32) = HermesValue::encodeDoubleValue((int32_t)(arg0 / arg1));
        ip = NEXTINST(Divu32);
        DISPATCH;
      }

      CASE(Loadi8) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadi8));
        int8_t *basePtr = reinterpret_cast<int8_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadi8).getNumber());
        O1REG(Loadi8) = HermesValue::encodeNumberValue(basePtr[addr]);
        ip = NEXTINST(Loadi8);
        DISPATCH;
      }
      CASE(Loadu8) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadu8));
        uint8_t *basePtr = reinterpret_cast<uint8_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadu8).getNumber());
        O1REG(Loadu8) = HermesValue::encodeNumberValue(basePtr[addr]);
        ip = NEXTINST(Loadu8);
        DISPATCH;
      }
      CASE(Loadi16) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadi16));
        int16_t *basePtr = reinterpret_cast<int16_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadi16).getNumber());
        O1REG(Loadi16) = HermesValue::encodeNumberValue(basePtr[addr >> 1]);
        ip = NEXTINST(Loadi16);
        DISPATCH;
      }
      CASE(Loadu16) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadu16));
        uint16_t *basePtr = reinterpret_cast<uint16_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadu16).getNumber());
        O1REG(Loadu16) = HermesValue::encodeNumberValue(basePtr[addr >> 1]);
        ip = NEXTINST(Loadu16);
        DISPATCH;
      }
      CASE(Loadi32) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadi32));
        int32_t *basePtr = reinterpret_cast<int32_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadi32).getNumber());
        O1REG(Loadi32) = HermesValue::encodeNumberValue(basePtr[addr >> 2]);
        ip = NEXTINST(Loadi32);
        DISPATCH;
      }
      CASE(Loadu32) {
        auto *mem = vmcast<JSTypedArrayBase>(O2REG(Loadu32));
        uint32_t *basePtr = reinterpret_cast<uint32_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O3REG(Loadu32).getNumber());
        O1REG(Loadu32) =
            HermesValue::encodeNumberValue((int32_t)(basePtr[addr >> 2]));
        ip = NEXTINST(Loadu32);
        DISPATCH;
      }

      CASE(Store8) {
        auto *mem = vmcast<JSTypedArrayBase>(O1REG(Store8));
        int8_t *basePtr = reinterpret_cast<int8_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O2REG(Store8).getNumber());
        basePtr[addr] = (int8_t)(int32_t)(O3REG(Store8).getNumber());
        ip = NEXTINST(Store8);
        DISPATCH;
      }
      CASE(Store16) {
        auto *mem = vmcast<JSTypedArrayBase>(O1REG(Store16));
        int16_t *basePtr = reinterpret_cast<int16_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O2REG(Store16).getNumber());
        basePtr[addr >> 1] = (int16_t)(int32_t)(O3REG(Store16).getNumber());
        ip = NEXTINST(Store16);
        DISPATCH;
      }
      CASE(Store32) {
        auto *mem = vmcast<JSTypedArrayBase>(O1REG(Store32));
        int32_t *basePtr = reinterpret_cast<int32_t *>(mem->begin(runtime));
        const uint32_t addr = (uint32_t)(int32_t)(O2REG(Store32).getNumber());
        basePtr[addr >> 2] = (int32_t)(O3REG(Store32).getNumber());
        // A nop for now.
        ip = NEXTINST(Store32);
        DISPATCH;
      }
#endif

      CASE(_last) {
        hermes_fatal("Invalid opcode _last");
      }
    }

    hermes_fatal(
        "All opcodes should dispatch to the next and not fallthrough "
        "to here");

  // We arrive here if we couldn't allocate the registers for the current frame.
  stackOverflow:
    CAPTURE_IP(runtime.raiseStackOverflow(
        Runtime::StackOverflowKind::JSRegisterStack));

  // We arrive here when we raised an exception in a callee, but we don't want
  // the callee to be able to handle it.
  handleExceptionInParent:
    // Restore the caller code block and IP.
    curCodeBlock = FRAME.getSavedCodeBlock();
    ip = FRAME.getSavedIP();

    // Pop to the previous frame where technically the error happened.
    frameRegs = &runtime.restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

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
        !runtime.thrownValue_.isEmpty() &&
        "thrownValue unavailable at exception");

    bool catchable = true;
    // If this is an Error object that was thrown internally, it didn't have
    // access to the current codeblock and IP, so collect the stack trace here.
    if (auto *jsError = dyn_vmcast<JSError>(runtime.thrownValue_)) {
      catchable = jsError->catchable();
      if (!jsError->getStackTrace()) {
        // Temporarily clear the thrown value for following operations.
        CAPTURE_IP_ASSIGN(
            auto errorHandle,
            runtime.makeHandle(vmcast<JSError>(runtime.thrownValue_)));
        runtime.clearThrownValue();

        CAPTURE_IP(JSError::recordStackTrace(
            errorHandle, runtime, false, curCodeBlock, ip));

        // Restore the thrown value.
        runtime.setThrownValue(errorHandle.getHermesValue());
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
    auto mode = runtime.debugger_.getPauseOnThrowMode();
    if (mode != PauseOnThrowMode::None) {
      if (!runtime.debugger_.isDebugging()) {
        // Determine whether the PauseOnThrowMode requires us to stop here.
        bool caught =
            runtime.debugger_
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
          CAPTURE_IP_ASSIGN(
              ExecutionStatus resultStatus,
              runtime.debugger_.runDebugger(
                  Debugger::RunReason::Exception, tmpState));
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

#ifdef HERMES_MEMORY_INSTRUMENTATION
      runtime.popCallStack();
#endif

      // Restore the code block and IP.
      curCodeBlock = FRAME.getSavedCodeBlock();
      ip = FRAME.getSavedIP();

      // Pop a stack frame.
      frameRegs =
          &runtime.restoreStackAndPreviousFrame(FRAME).getFirstLocalRef();

      SLOW_DEBUG(
          dbgs() << "function exit with exception: restored stackLevel="
                 << runtime.getStackLevel() << "\n");

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

#undef DEBUG_TYPE
