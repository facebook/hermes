/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Callable.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class Environment

VTable Environment::vt{CellKind::EnvironmentKind, 0};

void EnvironmentBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Environment *>(cell);
  mb.addField("@parentEnvironment", &self->parentEnvironment_);
  mb.addArray<Metadata::ArrayData::ArrayType::HermesValue>(
      self->getSlots(), &self->size_, sizeof(GCHermesValue));
}

//===----------------------------------------------------------------------===//
// class Callable

void CallableBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const Callable *>(cell);
  mb.addField("@environment", &self->environment_);
}

CallResult<HermesValue> Callable::_newObjectImpl(
    Handle<Callable> /*selfHandle*/,
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  return JSObject::create(runtime, parentHandle).getHermesValue();
}

void Callable::defineLazyProperties(Handle<Callable> fn, Runtime *runtime) {
  // lazy functions can be Bound or JS Functions.
  if (auto jsFun = Handle<JSFunction>::dyn_vmcast(runtime, fn)) {
    const CodeBlock *codeBlock = jsFun->getCodeBlock();
    // create empty object for prototype.
    auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
    auto prototypeObjectHandle =
        toHandle(runtime, JSObject::create(runtime, objProto));

    auto cr = Callable::defineNameLengthAndPrototype(
        fn,
        runtime,
        codeBlock->getName(),
        codeBlock->getParamCount() - 1,
        prototypeObjectHandle,
        Callable::WritablePrototype::Yes,
        codeBlock->isStrictMode());
    assert(
        cr != ExecutionStatus::EXCEPTION && "failed to define length and name");
    (void)cr;
  } else if (vmisa<BoundFunction>(fn.get())) {
    Handle<BoundFunction> boundfn = Handle<BoundFunction>::vmcast(fn);
    Handle<Callable> target = runtime->makeHandle(boundfn->getTarget());
    unsigned int argsWithThis = boundfn->getArgCountWithThis();

    auto res = BoundFunction::initializeLengthAndName(
        boundfn, runtime, target, argsWithThis == 0 ? 0 : argsWithThis - 1);
    assert(
        res != ExecutionStatus::EXCEPTION &&
        "failed to define length and name of bound function");
    (void)res;
  } else {
    // no other kind of function can be lazy currently
    assert(false && "invalid lazy function");
  }
}

ExecutionStatus Callable::defineNameLengthAndPrototype(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    SymbolID name,
    unsigned paramCount,
    Handle<JSObject> prototypeObjectHandle,
    WritablePrototype writablePrototype,
    bool strictMode) {
  PropertyFlags pf;
  pf.clear();
  pf.enumerable = 0;
  pf.writable = 0;
  pf.configurable = 1;

  GCScope scope{runtime, "defineNameLengthAndPrototype"};

  using P = Predefined;
/// Adds a property to the object in \p OBJ_HANDLE.  \p SYMBOL provides its name
/// as a \c Predefined enum value, and its value is  rooted in \p HANDLE.  If
/// property definition fails, the exceptional execution status will be
/// propogated to the outer function.
#define DEFINE_PROP(OBJ_HANDLE, SYMBOL, HANDLE)                \
  do {                                                         \
    auto status = JSObject::defineNewOwnProperty(              \
        OBJ_HANDLE,                                            \
        runtime,                                               \
        runtime->getPredefinedSymbolID(SYMBOL),                \
        pf,                                                    \
        HANDLE);                                               \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
  } while (false)

  // Define the name.
  auto nameHandle = name.isValid()
      ? runtime->makeHandle(runtime->getStringPrimFromSymbolID(name))
      : runtime->getPredefinedStringHandle(Predefined::emptyString);

  DEFINE_PROP(selfHandle, P::name, nameHandle);

  // Length is the number of formal arguments.
  auto lengthHandle =
      runtime->makeHandle(HermesValue::encodeDoubleValue(paramCount));
  DEFINE_PROP(selfHandle, P::length, lengthHandle);

  if (strictMode) {
    // Define .callee and .arguments properties: throw always in strict mode.
    auto accessor =
        Handle<PropertyAccessor>::vmcast(&runtime->throwTypeErrorAccessor);

    pf.clear();
    pf.enumerable = 0;
    pf.configurable = 0;
    pf.accessor = 1;

    DEFINE_PROP(selfHandle, P::caller, accessor);
    DEFINE_PROP(selfHandle, P::arguments, accessor);
  }

  if (prototypeObjectHandle) {
    // Set its 'prototype' property.
    pf.clear();
    pf.enumerable = 0;
    /// System constructors have read-only prototypes.
    pf.writable = (uint8_t)writablePrototype;
    pf.configurable = 0;
    DEFINE_PROP(selfHandle, P::prototype, prototypeObjectHandle);

    // Set the 'constructor' property in the prototype object.
    pf.clear();
    pf.enumerable = 0;
    pf.writable = 1;
    pf.configurable = 1;
    DEFINE_PROP(prototypeObjectHandle, P::constructor, selfHandle);
  }

  return ExecutionStatus::RETURNED;

#undef DEFINE_PROP
}

/// Execute this function with no arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<HermesValue> Callable::executeCall0(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> thisArgHandle,
    bool construct) {
  ScopedNativeCallFrame newFrame{runtime,
                                 0,
                                 selfHandle.getHermesValue(),
                                 construct
                                     ? selfHandle.getHermesValue()
                                     : HermesValue::encodeUndefinedValue(),
                                 *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  return call(selfHandle, runtime);
}

/// Execute this function with one argument. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<HermesValue> Callable::executeCall1(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    bool construct) {
  ScopedNativeCallFrame newFrame{runtime,
                                 1,
                                 selfHandle.getHermesValue(),
                                 construct
                                     ? selfHandle.getHermesValue()
                                     : HermesValue::encodeUndefinedValue(),
                                 *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  newFrame->getArgRef(0) = param1;
  return call(selfHandle, runtime);
}

/// Execute this function with two arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<HermesValue> Callable::executeCall2(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    bool construct) {
  ScopedNativeCallFrame newFrame{runtime,
                                 2,
                                 selfHandle.getHermesValue(),
                                 construct
                                     ? selfHandle.getHermesValue()
                                     : HermesValue::encodeUndefinedValue(),
                                 *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  return call(selfHandle, runtime);
}

/// Execute this function with three arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<HermesValue> Callable::executeCall3(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    HermesValue param3,
    bool construct) {
  ScopedNativeCallFrame newFrame{runtime,
                                 3,
                                 selfHandle.getHermesValue(),
                                 construct
                                     ? selfHandle.getHermesValue()
                                     : HermesValue::encodeUndefinedValue(),
                                 *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  newFrame->getArgRef(2) = param3;
  return call(selfHandle, runtime);
}

/// Execute this function with four arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<HermesValue> Callable::executeCall4(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    HermesValue param3,
    HermesValue param4,
    bool construct) {
  ScopedNativeCallFrame newFrame{runtime,
                                 4,
                                 selfHandle.getHermesValue(),
                                 construct
                                     ? selfHandle.getHermesValue()
                                     : HermesValue::encodeUndefinedValue(),
                                 *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  newFrame->getArgRef(2) = param3;
  newFrame->getArgRef(3) = param4;
  return call(selfHandle, runtime);
}

CallResult<HermesValue> Callable::executeConstruct0(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  auto thisVal = Callable::createThisForConstruct(selfHandle, runtime);
  if (LLVM_UNLIKELY(thisVal == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto thisValHandle = runtime->makeHandle<JSObject>(*thisVal);
  auto result = executeCall0(selfHandle, runtime, thisValHandle, true);
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return result->isObject() ? result : thisValHandle.getHermesValue();
}

CallResult<HermesValue> Callable::executeConstruct1(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<> param1) {
  auto thisVal = Callable::createThisForConstruct(selfHandle, runtime);
  if (LLVM_UNLIKELY(thisVal == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto thisValHandle = runtime->makeHandle<JSObject>(*thisVal);
  auto result = executeCall1(selfHandle, runtime, thisValHandle, *param1, true);
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return result->isObject() ? result : thisValHandle.getHermesValue();
}

CallResult<HermesValue> Callable::createThisForConstruct(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  auto prototypeProp = JSObject::getNamed(
      selfHandle,
      runtime,
      runtime->getPredefinedSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(prototypeProp == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> prototype = vmisa<JSObject>(*prototypeProp)
      ? runtime->makeHandle<JSObject>(*prototypeProp)
      : Handle<JSObject>::vmcast(&runtime->objectPrototype);
  return Callable::newObject(selfHandle, runtime, prototype);
}

CallResult<double> Callable::extractOwnLengthProperty(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  NamedPropertyDescriptor desc;
  if (!JSObject::getOwnNamedDescriptor(
          selfHandle,
          runtime,
          runtime->getPredefinedSymbolID(Predefined::length),
          desc)) {
    return 0.0;
  }

  auto propRes =
      JSObject::getNamedPropertyValue(selfHandle, runtime, selfHandle, desc);
  if (propRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  if (!propRes->isNumber()) {
    return 0.0;
  }

  auto intRes = toInteger(runtime, runtime->makeHandle(propRes.getValue()));
  if (intRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return intRes->getNumber();
}

//===----------------------------------------------------------------------===//
// class BoundFunction

CallableVTable BoundFunction::vt{
    {
        VTable(CellKind::BoundFunctionKind, sizeof(BoundFunction)),
        BoundFunction::_getOwnIndexedRangeImpl,
        BoundFunction::_haveOwnIndexedImpl,
        BoundFunction::_getOwnIndexedPropertyFlagsImpl,
        BoundFunction::_getOwnIndexedImpl,
        BoundFunction::_setOwnIndexedImpl,
        BoundFunction::_deleteOwnIndexedImpl,
        BoundFunction::_checkAllOwnIndexedImpl,
    },
    BoundFunction::_newObjectImpl,
    BoundFunction::_callImpl};

void BoundFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  CallableBuildMeta(cell, mb);
  const auto *self = static_cast<const BoundFunction *>(cell);
  mb.addField("@target", &self->target_);
  mb.addField("@argStorage", &self->argStorage_);
}

CallResult<HermesValue> BoundFunction::create(
    Runtime *runtime,
    Handle<Callable> target,
    unsigned argCountWithThis,
    const PinnedHermesValue *argsWithThis) {
  unsigned argCount = argCountWithThis > 0 ? argCountWithThis - 1 : 0;

  auto arrRes = ArrayStorage::create(runtime, argCount + 1);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto argStorageHandle = runtime->makeHandle<ArrayStorage>(*arrRes);

  void *mem = runtime->alloc(sizeof(BoundFunction));
  auto selfHandle = runtime->makeHandle(new (mem) BoundFunction(
      runtime,
      runtime->functionPrototypeRawPtr,
      runtime->getHiddenClassForPrototypeRaw(runtime->functionPrototypeRawPtr),
      target,
      argStorageHandle));

  // Copy the arguments. If we don't have any, we must at least initialize
  // 'this' to 'undefined'.
  MutableHandle<ArrayStorage> handle(runtime, selfHandle->argStorage_);

  // In case the storage was trimmed, make sure it has enough capacity.
  ArrayStorage::ensureCapacity(handle, runtime, argCount + 1);

  if (argCountWithThis) {
    for (unsigned i = 0; i != argCountWithThis; ++i) {
      ArrayStorage::push_back(
          handle, runtime, runtime->makeHandle(argsWithThis[i]));
    }
  } else {
    // Don't need to worry about resizing since it was created with a capacity
    // of at least 1.
    ArrayStorage::push_back(handle, runtime, runtime->getUndefinedValue());
  }
  // Update the storage pointer in case push_back() needed to reallocate.
  selfHandle->argStorage_.set(*handle, &runtime->getHeap());

  if (target->isLazy()) {
    // If the target is lazy we can make the bound function lazy.
    // If the target is NOT lazy, it might have getter/setters on length that
    // throws and we also need to throw.
    selfHandle->flags_.lazyObject = 1;
  } else {
    if (initializeLengthAndName(selfHandle, runtime, target, argCount) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return selfHandle.getHermesValue();
}

ExecutionStatus BoundFunction::initializeLengthAndName(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<Callable> target,
    unsigned argCount) {
  if (LLVM_UNLIKELY(target->isLazy())) {
    Callable::initializeLazyObject(runtime, target);
  }

  // Extract target.length.
  auto targetLength = Callable::extractOwnLengthProperty(target, runtime);
  if (targetLength == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  // Define .length
  PropertyFlags pf{};
  pf.enumerable = 0;
  pf.writable = 0;
  pf.configurable = 1;

  // Length is the number of formal arguments.
  auto length = runtime->makeHandle(HermesValue::encodeNumberValue(
      argCount >= *targetLength ? 0.0 : *targetLength - argCount));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              runtime->getPredefinedSymbolID(Predefined::length),
              pf,
              length) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Set the name by prepending "bound ".
  auto propRes = JSObject::getNamed(
      target, runtime, runtime->getPredefinedSymbolID(Predefined::name));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto nameHandle = propRes->isString()
      ? runtime->makeHandle<StringPrimitive>(*propRes)
      : runtime->getPredefinedStringHandle(Predefined::emptyString);
  auto nameView = StringPrimitive::createStringView(runtime, nameHandle);
  llvm::SmallU16String<32> boundName{"bound "};
  boundName.append(nameView.begin(), nameView.end());
  auto strRes = StringPrimitive::create(runtime, boundName);
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  DefinePropertyFlags dpf{};
  dpf.setWritable = 1;
  dpf.writable = 0;
  dpf.setEnumerable = 1;
  dpf.enumerable = 0;
  dpf.setConfigurable = 1;
  dpf.configurable = 1;
  dpf.setValue = 1;

  if (LLVM_UNLIKELY(
          JSObject::defineOwnProperty(
              selfHandle,
              runtime,
              runtime->getPredefinedSymbolID(Predefined::name),
              dpf,
              runtime->makeHandle(*strRes)) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Define .callee and .arguments properties: throw always in bound functions.
  auto accessor =
      Handle<PropertyAccessor>::vmcast(&runtime->throwTypeErrorAccessor);

  pf.clear();
  pf.enumerable = 0;
  pf.configurable = 0;
  pf.accessor = 1;

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              runtime->getPredefinedSymbolID(Predefined::caller),
              pf,
              accessor) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              runtime->getPredefinedSymbolID(Predefined::arguments),
              pf,
              accessor) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> BoundFunction::_newObjectImpl(
    Handle<Callable> selfHandle,
    Runtime *runtime,
    Handle<JSObject>) {
  auto *self = vmcast<BoundFunction>(*selfHandle);

  // If it is a chain of bound functions, skip directly to the end.
  while (auto *targetAsBound = dyn_vmcast<BoundFunction>(self->getTarget()))
    self = targetAsBound;

  auto targetHandle = runtime->makeHandle(self->getTarget());

  // We must duplicate the [[Construct]] functionality here.

  // Obtain "target.prototype".
  auto propRes = JSObject::getNamed(
      targetHandle,
      runtime,
      runtime->getPredefinedSymbolID(Predefined::prototype));
  if (propRes == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  auto prototype = runtime->makeHandle(*propRes);

  // If target.prototype is an object, use it, otherwise use the standard
  // object prototype.
  return targetHandle->getVT()->newObject(
      targetHandle,
      runtime,
      prototype->isObject()
          ? Handle<JSObject>::vmcast(prototype)
          : Handle<JSObject>::vmcast(&runtime->objectPrototype));
}

CallResult<HermesValue> BoundFunction::_boundCall(
    BoundFunction *self,
    Runtime *runtime) {
  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime->raiseStackOverflow();
  }

  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  StackFramePtr originalCalleeFrame = StackFramePtr(runtime->getStackPointer());
  // Save the original newTarget since we will overwrite it.
  HermesValue originalNewTarget = originalCalleeFrame.getNewTargetRef();
  // Save the original arg count since we will lose it.
  auto originalArgCount = originalCalleeFrame.getArgCount();
  // Keep track of the total arg count.
  auto totalArgCount = originalArgCount;

  auto callerFrame = runtime->getCurrentFrame();
  // We must preserve the "thisArg" passed to us by the caller because it is in
  // a register that is not supposed to be modified by a call. Copy it to the
  // scratch register in the caller's frame.
  // Note that since there is only one scratch reg, we must process all chained
  // bound calls in one go (which is more efficient anyway).
  callerFrame.getScratchRef() = originalCalleeFrame.getThisArgRef();

  // Pop the stack down to the first argument, erasing the call frame - we don't
  // need the call frame since we will build a new one.
  runtime->popToSavedStackPointer(&originalCalleeFrame->getArgRefUnsafe(0));

  // Loop, copying the bound arguments of all chained bound functions.
  for (;;) {
    auto boundArgCount = self->getArgCountWithThis() - 1;
    totalArgCount += boundArgCount;

    // Check if we have enough stack for the arguments and the frame metadata.
    if (LLVM_UNLIKELY(!runtime->checkAvailableStack(
            StackFrameLayout::callerOutgoingRegisters(boundArgCount)))) {
      // Oops, we ran out of stack in the middle of calling a bound function.
      // Restore everything and bail.

      // We can't "pop" the stack pointer to an arbitrary value, which may be
      // higher than the current pointer. So, first we pop everything that we
      // may have pushed, then allocate the correct amount to get back to the
      // initial state.
      runtime->popToSavedStackPointer(&originalCalleeFrame->getArgRefUnsafe(0));
      runtime->allocUninitializedStack(StackFrameLayout::ThisArg + 1);
      assert(
          runtime->getStackPointer() == originalCalleeFrame.ptr() &&
          "Stack wasn't restored properly");

      runtime->raiseStackOverflow();
      res = ExecutionStatus::EXCEPTION;
      goto bail;
    }

    // Allocate space only for the arguments for now.
    auto *stack = runtime->allocUninitializedStack(boundArgCount);

    // Copy the bound arguments (but not the bound "this").
    if (StackFrameLayout::StackIncrement == -1) {
      std::uninitialized_copy_n(
          self->getArgsWithThis() + 1, boundArgCount, stack);
    } else {
      std::uninitialized_copy_n(
          self->getArgsWithThis() + 1,
          boundArgCount,
          llvm::make_reverse_iterator(stack + 1));
    }

    // Loop while the target is another bound function.
    auto *targetAsBound = dyn_vmcast<BoundFunction>(self->getTarget());
    if (!targetAsBound)
      break;
    self = targetAsBound;
  }

  // Block scope for non-trivial variables to avoid complaints from "goto".
  {
    // Allocate space for "thisArg" and the frame metdata following the outgoing
    // registers. Note that we already checked earlier that we have enough
    // stack.
    static_assert(
        StackFrameLayout::CallerExtraRegistersAtEnd ==
            StackFrameLayout::ThisArg,
        "Stack frame layout changed without updating _boundCall");
    auto *stack =
        runtime->allocUninitializedStack(StackFrameLayout::ThisArg + 1);

    // Initialize the new frame metadata.
    auto newCalleeFrame = StackFramePtr::initFrame(
        stack,
        runtime->getCurrentFrame(),
        nullptr,
        nullptr,
        totalArgCount,
        HermesValue::encodeObjectValue(self->getTarget()),
        originalNewTarget);
    // Initialize "thisArg". When constructing we must use the original 'this',
    // not the bound one.
    newCalleeFrame.getThisArgRef() = !originalNewTarget.isUndefined()
        ? callerFrame.getScratchRef()
        : self->getArgsWithThis()[0];

    res =
        Callable::call(newCalleeFrame.getCalleeClosureHandleUnsafe(), runtime);

    assert(
        runtime->getCurrentFrame() == callerFrame &&
        "caller frame not restored");

    // Restore the original stack level.
    runtime->popToSavedStackPointer(originalCalleeFrame.ptr());
  }

bail:
  // We must restore the original call frame. There is no need to restore
  // all the fields to their previous values, just the registers which are not
  // supposed to be modified by a call.
  StackFramePtr::initFrame(
      originalCalleeFrame.ptr(),
      StackFramePtr{},
      nullptr,
      nullptr,
      0,
      nullptr,
      false);

  // Restore "thisArg" and clear the scratch register to avoid a leak.
  originalCalleeFrame.getThisArgRef() = callerFrame.getScratchRef();
  callerFrame.getScratchRef() = HermesValue::encodeUndefinedValue();

  return res;
}

CallResult<HermesValue> BoundFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  return _boundCall(vmcast<BoundFunction>(selfHandle.get()), runtime);
}

//===----------------------------------------------------------------------===//
// class NativeFunction

CallableVTable NativeFunction::vt{
    {
        VTable(CellKind::NativeFunctionKind, sizeof(NativeFunction)),
        NativeFunction::_getOwnIndexedRangeImpl,
        NativeFunction::_haveOwnIndexedImpl,
        NativeFunction::_getOwnIndexedPropertyFlagsImpl,
        NativeFunction::_getOwnIndexedImpl,
        NativeFunction::_setOwnIndexedImpl,
        NativeFunction::_deleteOwnIndexedImpl,
        NativeFunction::_checkAllOwnIndexedImpl,
    },
    NativeFunction::_newObjectImpl,
    NativeFunction::_callImpl};

void NativeFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  CallableBuildMeta(cell, mb);
}

Handle<NativeFunction> NativeFunction::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle,
    void *context,
    NativeFunctionPtr functionPtr,
    SymbolID name,
    unsigned paramCount,
    Handle<JSObject> prototypeObjectHandle) {
  void *mem = runtime->alloc(sizeof(NativeFunction));
  auto selfHandle = runtime->makeHandle(new (mem) NativeFunction(
      runtime,
      &vt.base.base,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(*parentHandle),
      context,
      functionPtr));

  auto st = defineNameLengthAndPrototype(
      selfHandle,
      runtime,
      name,
      paramCount,
      prototypeObjectHandle,
      Callable::WritablePrototype::Yes,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  return selfHandle;
}

Handle<NativeFunction> NativeFunction::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle,
    Handle<Environment> parentEnvHandle,
    void *context,
    NativeFunctionPtr functionPtr,
    SymbolID name,
    unsigned paramCount,
    Handle<JSObject> prototypeObjectHandle) {
  void *mem = runtime->alloc(sizeof(NativeFunction));
  auto selfHandle = runtime->makeHandle(new (mem) NativeFunction(
      runtime,
      &vt.base.base,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(*parentHandle),
      parentEnvHandle,
      context,
      functionPtr));

  auto st = defineNameLengthAndPrototype(
      selfHandle,
      runtime,
      name,
      paramCount,
      prototypeObjectHandle,
      Callable::WritablePrototype::Yes,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  return selfHandle;
}

CallResult<HermesValue> NativeFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  assert(
      runtime->getThrownValue().isEmpty() &&
      "pending exception when calling native function");

  auto result = _nativeCall(vmcast<NativeFunction>(selfHandle.get()), runtime);

  // If there was an exception thrown during the call.
  // TODO: T30015280
  if (LLVM_UNLIKELY(!runtime->getThrownValue().isEmpty()))
    return ExecutionStatus::EXCEPTION;

  return result.getValue();
}

CallResult<HermesValue> NativeFunction::_newObjectImpl(
    Handle<Callable>,
    Runtime *runtime,
    Handle<JSObject>) {
  return runtime->raiseTypeError(
      "This function cannot be used as a constructor.");
}

//===----------------------------------------------------------------------===//
// class NativeConstructor

const CallableVTable NativeConstructor::vt{
    {
        VTable(CellKind::NativeConstructorKind, sizeof(NativeConstructor)),
        NativeConstructor::_getOwnIndexedRangeImpl,
        NativeConstructor::_haveOwnIndexedImpl,
        NativeConstructor::_getOwnIndexedPropertyFlagsImpl,
        NativeConstructor::_getOwnIndexedImpl,
        NativeConstructor::_setOwnIndexedImpl,
        NativeConstructor::_deleteOwnIndexedImpl,
        NativeConstructor::_checkAllOwnIndexedImpl,
    },
    NativeConstructor::_newObjectImpl,
    NativeConstructor::_callImpl};

void NativeConstructorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  NativeFunctionBuildMeta(cell, mb);
}

#ifndef NDEBUG
CallResult<HermesValue> NativeConstructor::_callImpl(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  StackFramePtr newFrame{runtime->getStackPointer()};

  if (newFrame.isConstructorCall()) {
    auto consHandle = Handle<NativeConstructor>::vmcast(selfHandle);
    assert(
        consHandle->targetKind_ ==
            vmcast<JSObject>(newFrame.getThisArgRef())->getKind() &&
        "call(construct=true) called without the correct 'this' value");
  }
  return NativeFunction::_callImpl(selfHandle, runtime);
}
#endif

//===----------------------------------------------------------------------===//
// class JSFunction

CallableVTable JSFunction::vt{{
                                  VTable(
                                      CellKind::FunctionKind,
                                      sizeof(JSFunction),
                                      JSFunction::_finalizeImpl),
                                  JSFunction::_getOwnIndexedRangeImpl,
                                  JSFunction::_haveOwnIndexedImpl,
                                  JSFunction::_getOwnIndexedPropertyFlagsImpl,
                                  JSFunction::_getOwnIndexedImpl,
                                  JSFunction::_setOwnIndexedImpl,
                                  JSFunction::_deleteOwnIndexedImpl,
                                  JSFunction::_checkAllOwnIndexedImpl,
                              },
                              JSFunction::_newObjectImpl,
                              JSFunction::_callImpl};

void FunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  CallableBuildMeta(cell, mb);
}

CallResult<HermesValue> JSFunction::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle,
    Handle<Environment> envHandle,
    CodeBlock *codeBlock) {
  void *mem =
      runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(sizeof(JSFunction));
  auto selfHandle = runtime->makeHandle(new (mem) JSFunction(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(*parentHandle),
      envHandle,
      codeBlock));
  selfHandle->flags_.lazyObject = 1;
  return selfHandle.getHermesValue();
}

void JSFunction::_finalizeImpl(GCCell *cell, GC *) {
  auto *self = vmcast<JSFunction>(cell);
  // Destruct the object.
  self->~JSFunction();
}

CallResult<HermesValue> JSFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime *runtime) {
  auto *self = vmcast<JSFunction>(selfHandle.get());
  if (auto *jitPtr = self->getCodeBlock()->getJITCompiled())
    return (*jitPtr)(runtime);
  return runtime->interpretFunction(self->getCodeBlock());
}

} // namespace vm
} // namespace hermes
