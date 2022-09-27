/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Callable.h"

#include "hermes/VM/ArrayLike.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSDate.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSMapImpl.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/JSWeakRef.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvh/ADT/ArrayRef.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class Environment

const VTable Environment::vt{CellKind::EnvironmentKind, 0};

void EnvironmentBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Environment *>(cell);
  mb.setVTable(&Environment::vt);
  mb.addField("parentEnvironment", &self->parentEnvironment_);
  mb.addArray(self->getSlots(), &self->size_, sizeof(GCHermesValue));
}

//===----------------------------------------------------------------------===//
// class Callable

void CallableBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<Callable>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const Callable *>(cell);
  mb.setVTable(&Callable::vt);
  mb.addField("environment", &self->environment_);
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string Callable::_snapshotNameImpl(GCCell *cell, GC &gc) {
  auto *const self = reinterpret_cast<Callable *>(cell);
  return self->getNameIfExists(gc.getPointerBase());
}
#endif

CallResult<PseudoHandle<JSObject>> Callable::_newObjectImpl(
    Handle<Callable> /*selfHandle*/,
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  return JSObject::create(runtime, parentHandle);
}

void Callable::defineLazyProperties(Handle<Callable> fn, Runtime &runtime) {
  // lazy functions can be Bound or JS Functions.
  if (auto jsFun = Handle<JSFunction>::dyn_vmcast(fn)) {
    const CodeBlock *codeBlock = jsFun->getCodeBlock(runtime);
    // Create empty object for prototype.
    auto prototypeParent = vmisa<JSGeneratorFunction>(*jsFun)
        ? Handle<JSObject>::vmcast(&runtime.generatorPrototype)
        : Handle<JSObject>::vmcast(&runtime.objectPrototype);

    // According to ES12 26.7.4, AsyncFunction instances do not have a
    // 'prototype' property, hence we need to set an null handle here.
    auto prototypeObjectHandle = vmisa<JSAsyncFunction>(*jsFun)
        ? Runtime::makeNullHandle<JSObject>()
        : runtime.makeHandle(JSObject::create(runtime, prototypeParent));

    auto cr = Callable::defineNameLengthAndPrototype(
        fn,
        runtime,
        codeBlock->getNameMayAllocate(),
        codeBlock->getParamCount() - 1,
        prototypeObjectHandle,
        Callable::WritablePrototype::Yes,
        codeBlock->isStrictMode());
    assert(
        cr != ExecutionStatus::EXCEPTION && "failed to define length and name");
    (void)cr;
  } else if (vmisa<BoundFunction>(fn.get())) {
    Handle<BoundFunction> boundfn = Handle<BoundFunction>::vmcast(fn);
    Handle<Callable> target = runtime.makeHandle(boundfn->getTarget(runtime));
    unsigned int argsWithThis = boundfn->getArgCountWithThis(runtime);

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
    Runtime &runtime,
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

  namespace P = Predefined;
/// Adds a property to the object in \p OBJ_HANDLE.  \p SYMBOL provides its name
/// as a \c Predefined enum value, and its value is  rooted in \p HANDLE.  If
/// property definition fails, the exceptional execution status will be
/// propagated to the outer function.
#define DEFINE_PROP(OBJ_HANDLE, SYMBOL, HANDLE)                            \
  do {                                                                     \
    auto status = JSObject::defineNewOwnProperty(                          \
        OBJ_HANDLE, runtime, Predefined::getSymbolID(SYMBOL), pf, HANDLE); \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {             \
      return ExecutionStatus::EXCEPTION;                                   \
    }                                                                      \
  } while (false)

  assert(name.isValid() && "A name must always be provided");

  // Length is the number of formal arguments.
  // 10.2.9 SetFunctionLength is performed during 10.2.3 OrdinaryFunctionCreate.
  auto lengthHandle =
      runtime.makeHandle(HermesValue::encodeDoubleValue(paramCount));
  DEFINE_PROP(selfHandle, P::length, lengthHandle);

  // Define the name.
  // 10.2.8 SetFunctionName is performed after 10.2.3 OrdinaryFunctionCreate.
  auto nameHandle = runtime.makeHandle(runtime.getStringPrimFromSymbolID(name));
  DEFINE_PROP(selfHandle, P::name, nameHandle);

  if (strictMode) {
    // Define .callee and .arguments properties: throw always in strict mode.
    auto accessor =
        Handle<PropertyAccessor>::vmcast(&runtime.throwTypeErrorAccessor);

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

    if (LLVM_LIKELY(!vmisa<JSGeneratorFunction>(*selfHandle))) {
      // Set the 'constructor' property in the prototype object.
      // This must not be set for GeneratorFunctions, because
      // prototypes must not point back to their constructors.
      // See the diagram: ES9.0 25.2 (GeneratorFunction objects).
      pf.clear();
      pf.enumerable = 0;
      pf.writable = 1;
      pf.configurable = 1;
      DEFINE_PROP(prototypeObjectHandle, P::constructor, selfHandle);
    }
  }

  return ExecutionStatus::RETURNED;

#undef DEFINE_PROP
}

/// Execute this function with no arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<PseudoHandle<>> Callable::executeCall0(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> thisArgHandle,
    bool construct) {
  ScopedNativeCallFrame newFrame{
      runtime,
      0,
      selfHandle.getHermesValue(),
      construct ? selfHandle.getHermesValue()
                : HermesValue::encodeUndefinedValue(),
      *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  return call(selfHandle, runtime);
}

/// Execute this function with one argument. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<PseudoHandle<>> Callable::executeCall1(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    bool construct) {
  ScopedNativeCallFrame newFrame{
      runtime,
      1,
      selfHandle.getHermesValue(),
      construct ? selfHandle.getHermesValue()
                : HermesValue::encodeUndefinedValue(),
      *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  newFrame->getArgRef(0) = param1;
  return call(selfHandle, runtime);
}

/// Execute this function with two arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<PseudoHandle<>> Callable::executeCall2(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    bool construct) {
  ScopedNativeCallFrame newFrame{
      runtime,
      2,
      selfHandle.getHermesValue(),
      construct ? selfHandle.getHermesValue()
                : HermesValue::encodeUndefinedValue(),
      *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  return call(selfHandle, runtime);
}

/// Execute this function with three arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<PseudoHandle<>> Callable::executeCall3(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    HermesValue param3,
    bool construct) {
  ScopedNativeCallFrame newFrame{
      runtime,
      3,
      selfHandle.getHermesValue(),
      construct ? selfHandle.getHermesValue()
                : HermesValue::encodeUndefinedValue(),
      *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  newFrame->getArgRef(2) = param3;
  return call(selfHandle, runtime);
}

/// Execute this function with four arguments. This is just a convenience
/// helper method; it actually invokes the interpreter recursively.
CallResult<PseudoHandle<>> Callable::executeCall4(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> thisArgHandle,
    HermesValue param1,
    HermesValue param2,
    HermesValue param3,
    HermesValue param4,
    bool construct) {
  ScopedNativeCallFrame newFrame{
      runtime,
      4,
      selfHandle.getHermesValue(),
      construct ? selfHandle.getHermesValue()
                : HermesValue::encodeUndefinedValue(),
      *thisArgHandle};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  newFrame->getArgRef(0) = param1;
  newFrame->getArgRef(1) = param2;
  newFrame->getArgRef(2) = param3;
  newFrame->getArgRef(3) = param4;
  return call(selfHandle, runtime);
}

CallResult<PseudoHandle<>> Callable::executeCall(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> newTarget,
    Handle<> thisArgument,
    Handle<JSObject> arrayLike) {
  CallResult<uint64_t> nRes = getArrayLikeLength(arrayLike, runtime);
  if (LLVM_UNLIKELY(nRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*nRes > UINT32_MAX) {
    return runtime.raiseRangeError("Too many arguments for apply");
  }
  uint32_t n = static_cast<uint32_t>(*nRes);
  ScopedNativeCallFrame newFrame{
      runtime, n, selfHandle.getHermesValue(), *newTarget, *thisArgument};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);

  // Initialize the arguments to undefined because we might allocate and cause
  // a gc while populating them.
  // TODO: look into doing this lazily.
  newFrame.fillArguments(n, HermesValue::encodeUndefinedValue());

  if (LLVM_UNLIKELY(
          createListFromArrayLike(
              arrayLike,
              runtime,
              n,
              [&newFrame](Runtime &, uint64_t index, PseudoHandle<> value) {
                newFrame->getArgRef(index) = value.getHermesValue();
                return ExecutionStatus::RETURNED;
              }) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return Callable::call(selfHandle, runtime);
}

CallResult<PseudoHandle<>> Callable::executeConstruct0(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  auto thisVal = Callable::createThisForConstruct(selfHandle, runtime);
  if (LLVM_UNLIKELY(thisVal == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto thisValHandle = runtime.makeHandle<JSObject>(std::move(thisVal->get()));
  auto result = executeCall0(selfHandle, runtime, thisValHandle, true);
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return (*result)->isObject() ? std::move(result)
                               : CallResult<PseudoHandle<>>(createPseudoHandle(
                                     thisValHandle.getHermesValue()));
}

CallResult<PseudoHandle<>> Callable::executeConstruct1(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<> param1) {
  auto thisVal = Callable::createThisForConstruct(selfHandle, runtime);
  if (LLVM_UNLIKELY(thisVal == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto thisValHandle = runtime.makeHandle<JSObject>(std::move(thisVal->get()));
  CallResult<PseudoHandle<>> result =
      executeCall1(selfHandle, runtime, thisValHandle, *param1, true);
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return (*result)->isObject() ? std::move(result)
                               : CallResult<PseudoHandle<>>(createPseudoHandle(
                                     thisValHandle.getHermesValue()));
}

CallResult<PseudoHandle<JSObject>> Callable::createThisForConstruct(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  CallResult<PseudoHandle<>> prototypeProp = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::prototype));
  if (LLVM_UNLIKELY(prototypeProp == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> prototype = vmisa<JSObject>(prototypeProp->get())
      ? runtime.makeHandle<JSObject>(prototypeProp->get())
      : Handle<JSObject>::vmcast(&runtime.objectPrototype);
  return Callable::newObject(selfHandle, runtime, prototype);
}

CallResult<double> Callable::extractOwnLengthProperty_RJS(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  CallResult<PseudoHandle<>> propRes{
      createPseudoHandle(HermesValue::encodeUndefinedValue())};
  NamedPropertyDescriptor desc;
  if (JSObject::getOwnNamedDescriptor(
          selfHandle,
          runtime,
          Predefined::getSymbolID(Predefined::length),
          desc)) {
    propRes = JSObject::getNamedPropertyValue_RJS(
        selfHandle, runtime, selfHandle, desc);
  } else if (selfHandle->isProxyObject()) {
    ComputedPropertyDescriptor desc;
    CallResult<bool> hasLength = JSProxy::getOwnProperty(
        selfHandle,
        runtime,
        runtime.getPredefinedStringHandle(Predefined::length),
        desc,
        nullptr);
    if (LLVM_UNLIKELY(hasLength == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (*hasLength) {
      propRes = JSProxy::getNamed(
          selfHandle,
          runtime,
          Predefined::getSymbolID(Predefined::length),
          selfHandle);
    }
  }

  {
    NoAllocScope naScope{runtime};

    if (propRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    if (!(*propRes)->isNumber()) {
      return 0.0;
    }
  }

  auto intRes =
      toIntegerOrInfinity(runtime, runtime.makeHandle(std::move(*propRes)));
  if (intRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return intRes->getNumber();
}

//===----------------------------------------------------------------------===//
// class BoundFunction

const CallableVTable BoundFunction::vt{
    {
        VTable(
            CellKind::BoundFunctionKind,
            cellSize<BoundFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure, BoundFunction::_snapshotNameImpl,
                  BoundFunction::_snapshotAddEdgesImpl, nullptr, nullptr
            }
#endif
            ),
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
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<BoundFunction>());
  CallableBuildMeta(cell, mb);
  const auto *self = static_cast<const BoundFunction *>(cell);
  mb.setVTable(&BoundFunction::vt);
  mb.addField("target", &self->target_);
  mb.addField("argStorage", &self->argStorage_);
}

CallResult<HermesValue> BoundFunction::create(
    Runtime &runtime,
    Handle<Callable> target,
    unsigned argCountWithThis,
    ConstArgIterator argsWithThis) {
  unsigned argCount = argCountWithThis > 0 ? argCountWithThis - 1 : 0;

  // Copy the arguments. If we don't have any, we must at least initialize
  // 'this' to 'undefined'.
  auto arrRes = ArrayStorage::create(runtime, argCount + 1);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arrHandle = runtime.makeMutableHandle(vmcast<ArrayStorage>(*arrRes));

  if (argCountWithThis) {
    for (unsigned i = 0; i != argCountWithThis; ++i) {
      ArrayStorage::push_back(arrHandle, runtime, Handle<>(&argsWithThis[i]));
    }
  } else {
    // Don't need to worry about resizing since it was created with a capacity
    // of at least 1.
    ArrayStorage::push_back(arrHandle, runtime, Runtime::getUndefinedValue());
  }

  auto *cell = runtime.makeAFixed<BoundFunction>(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      runtime.getHiddenClassForPrototype(
          runtime.functionPrototypeRawPtr, numOverlapSlots<BoundFunction>()),
      target,
      arrHandle);
  auto selfHandle = JSObjectInit::initToHandle(runtime, cell);

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
    Runtime &runtime,
    Handle<Callable> target,
    unsigned argCount) {
  if (LLVM_UNLIKELY(target->isLazy())) {
    Callable::initializeLazyObject(runtime, target);
  }

  // Extract target.length.
  auto targetLength = Callable::extractOwnLengthProperty_RJS(target, runtime);
  if (targetLength == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  // Define .length
  PropertyFlags pf{};
  pf.enumerable = 0;
  pf.writable = 0;
  pf.configurable = 1;

  // Length is the number of formal arguments.
  auto length = runtime.makeHandle(HermesValue::encodeNumberValue(
      argCount >= *targetLength ? 0.0 : *targetLength - argCount));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              pf,
              length) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Set the name by prepending "bound ".
  auto propRes = JSObject::getNamed_RJS(
      target, runtime, Predefined::getSymbolID(Predefined::name));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto nameHandle = (*propRes)->isString()
      ? runtime.makeHandle<StringPrimitive>(propRes->getHermesValue())
      : runtime.getPredefinedStringHandle(Predefined::emptyString);
  auto nameView = StringPrimitive::createStringView(runtime, nameHandle);
  llvh::SmallU16String<32> boundName{"bound "};
  boundName.append(nameView.begin(), nameView.end());
  // Share name strings for repeatedly bound functions by using the
  // identifier table. If a new symbol is created, it will disappear
  // after the name string dies, since nothing else refers to it.
  auto &identifierTable = runtime.getIdentifierTable();
  auto boundNameSym = identifierTable.getSymbolHandle(runtime, boundName);
  if (LLVM_UNLIKELY(boundNameSym == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> boundNameHandle(
      runtime, identifierTable.getStringPrim(runtime, **boundNameSym));
  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;

  if (LLVM_UNLIKELY(
          JSObject::defineOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(Predefined::name),
              dpf,
              boundNameHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Define .callee and .arguments properties: throw always in bound functions.
  auto accessor =
      Handle<PropertyAccessor>::vmcast(&runtime.throwTypeErrorAccessor);

  pf.clear();
  pf.enumerable = 0;
  pf.configurable = 0;
  pf.accessor = 1;

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(Predefined::caller),
              pf,
              accessor) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(Predefined::arguments),
              pf,
              accessor) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return ExecutionStatus::RETURNED;
}

CallResult<PseudoHandle<JSObject>> BoundFunction::_newObjectImpl(
    Handle<Callable> selfHandle,
    Runtime &runtime,
    Handle<JSObject>) {
  auto *self = vmcast<BoundFunction>(*selfHandle);

  // If it is a chain of bound functions, skip directly to the end.
  while (auto *targetAsBound =
             dyn_vmcast<BoundFunction>(self->getTarget(runtime)))
    self = targetAsBound;

  auto targetHandle = runtime.makeHandle(self->getTarget(runtime));

  // We must duplicate the [[Construct]] functionality here.

  // Obtain "target.prototype".
  auto propRes = JSObject::getNamed_RJS(
      targetHandle, runtime, Predefined::getSymbolID(Predefined::prototype));
  if (propRes == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  auto prototype = runtime.makeHandle(std::move(*propRes));

  // If target.prototype is an object, use it, otherwise use the standard
  // object prototype.
  return targetHandle->getVT()->newObject(
      targetHandle,
      runtime,
      prototype->isObject()
          ? Handle<JSObject>::vmcast(prototype)
          : Handle<JSObject>::vmcast(&runtime.objectPrototype));
}

CallResult<PseudoHandle<>> BoundFunction::_boundCall(
    BoundFunction *self,
    const Inst *ip,
    Runtime &runtime) {
  ScopedNativeDepthTracker depthTracker{runtime};
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }

  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
  StackFramePtr originalCalleeFrame = StackFramePtr(runtime.getStackPointer());
  // Save the original newTarget since we will overwrite it.
  HermesValue originalNewTarget = originalCalleeFrame.getNewTargetRef();
  // Save the original arg count since we will lose it.
  auto originalArgCount = originalCalleeFrame.getArgCount();
  // Keep track of the total arg count.
  auto totalArgCount = originalArgCount;

  auto callerFrame = runtime.getCurrentFrame();
  // We must preserve the "thisArg" passed to us by the caller because it is in
  // a register that is not supposed to be modified by a call. Copy it to the
  // scratch register in the caller's frame.
  // Note that since there is only one scratch reg, we must process all chained
  // bound calls in one go (which is more efficient anyway).
  callerFrame.getScratchRef() = originalCalleeFrame.getThisArgRef();

  // Pop the stack down to the first argument, erasing the call frame - we don't
  // need the call frame since we will build a new one.
  runtime.popToSavedStackPointer(&originalCalleeFrame.getThisArgRef());

  // Loop, copying the bound arguments of all chained bound functions.
  for (;;) {
    auto boundArgCount = self->getArgCountWithThis(runtime) - 1;
    totalArgCount += boundArgCount;

    // Check if we have enough stack for the arguments and the frame metadata.
    if (LLVM_UNLIKELY(!runtime.checkAvailableStack(
            StackFrameLayout::callerOutgoingRegisters(boundArgCount)))) {
      // Oops, we ran out of stack in the middle of calling a bound function.
      // Restore everything and bail.

      // We can't "pop" the stack pointer to an arbitrary value, which may be
      // higher than the current pointer. So, first we pop everything that we
      // may have pushed, then allocate the correct amount to get back to the
      // initial state.
      runtime.popToSavedStackPointer(&originalCalleeFrame.getThisArgRef());
      runtime.allocUninitializedStack(
          StackFrameLayout::CallerExtraRegistersAtEnd + 1);
      assert(
          runtime.getStackPointer() == originalCalleeFrame.ptr() &&
          "Stack wasn't restored properly");

      res = runtime.raiseStackOverflow(
          Runtime::StackOverflowKind::JSRegisterStack);
      goto bail;
    }

    // Allocate space only for the arguments for now.
    auto *stack = runtime.allocUninitializedStack(boundArgCount);

    // Copy the bound arguments (but not the bound "this").
    std::uninitialized_copy_n(
        self->getArgsWithThis(runtime) + 1, boundArgCount, ArgIterator(stack));

    // Loop while the target is another bound function.
    auto *targetAsBound = dyn_vmcast<BoundFunction>(self->getTarget(runtime));
    if (!targetAsBound)
      break;
    self = targetAsBound;
  }

  // Block scope for non-trivial variables to avoid complaints from "goto".
  {
    // Allocate space for "thisArg" and the frame metdata following the outgoing
    // registers. Note that we already checked earlier that we have enough
    // stack.
    auto *stack = runtime.allocUninitializedStack(
        StackFrameLayout::CallerExtraRegistersAtEnd + 1);

    // Initialize the new frame metadata.
    auto newCalleeFrame = StackFramePtr::initFrame(
        stack,
        runtime.getCurrentFrame(),
        ip,
        nullptr,
        totalArgCount,
        HermesValue::encodeObjectValue(self->getTarget(runtime)),
        originalNewTarget);
    // Initialize "thisArg". When constructing we must use the original 'this',
    // not the bound one.
    newCalleeFrame.getThisArgRef() = !originalNewTarget.isUndefined()
        ? static_cast<HermesValue>(callerFrame.getScratchRef())
        : self->getArgsWithThis(runtime)[0];

    res =
        Callable::call(newCalleeFrame.getCalleeClosureHandleUnsafe(), runtime);

    assert(
        runtime.getCurrentFrame() == callerFrame &&
        "caller frame not restored");

    // Restore the original stack level.
    runtime.popToSavedStackPointer(originalCalleeFrame.ptr());
  }

bail:
  // We must restore the original call frame. There is no need to restore
  // all the fields to their previous values, just the registers which are not
  // supposed to be modified by a call.
  StackFramePtr::initFrame(
      originalCalleeFrame.ptr(),
      StackFramePtr{},
      ip,
      nullptr,
      0,
      HermesValue::encodeEmptyValue(),
      HermesValue::encodeEmptyValue());

  // Restore "thisArg" and clear the scratch register to avoid a leak.
  originalCalleeFrame.getThisArgRef() = callerFrame.getScratchRef();
  callerFrame.getScratchRef() = HermesValue::encodeUndefinedValue();

  return res;
}

CallResult<PseudoHandle<>> BoundFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  // Pass `nullptr` as the IP because this function is never called
  // from the interpreter, which should use `_boundCall` directly.
  return _boundCall(vmcast<BoundFunction>(selfHandle.get()), nullptr, runtime);
}

//===----------------------------------------------------------------------===//
// class NativeFunction

const CallableVTable NativeFunction::vt{
    {
        VTable(
            CellKind::NativeFunctionKind,
            cellSize<NativeFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure,
                  NativeFunction::_snapshotNameImpl,
                  NativeFunction::_snapshotAddEdgesImpl, nullptr, nullptr
            }
#endif
            ),
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
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<NativeFunction>());
  CallableBuildMeta(cell, mb);
  mb.setVTable(&NativeFunction::vt);
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string NativeFunction::_snapshotNameImpl(GCCell *cell, GC &gc) {
  auto *const self = reinterpret_cast<NativeFunction *>(cell);
  return getFunctionName(self->functionPtr_);
}
#endif

Handle<NativeFunction> NativeFunction::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    void *context,
    NativeFunctionPtr functionPtr,
    SymbolID name,
    unsigned paramCount,
    Handle<JSObject> prototypeObjectHandle,
    unsigned additionalSlotCount) {
  size_t reservedSlots =
      numOverlapSlots<NativeFunction>() + additionalSlotCount;
  auto *cell = runtime.makeAFixed<NativeFunction>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(*parentHandle, reservedSlots),
      context,
      functionPtr);
  auto selfHandle = JSObjectInit::initToHandle(runtime, cell);

  // Allocate a propStorage if the number of additional slots requires it.
  runtime.ignoreAllocationFailure(
      JSObject::allocatePropStorage(selfHandle, runtime, reservedSlots));

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
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    Handle<Environment> parentEnvHandle,
    void *context,
    NativeFunctionPtr functionPtr,
    SymbolID name,
    unsigned paramCount,
    Handle<JSObject> prototypeObjectHandle,
    unsigned additionalSlotCount) {
  auto *cell = runtime.makeAFixed<NativeFunction>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle,
          numOverlapSlots<NativeFunction>() + additionalSlotCount),
      parentEnvHandle,
      context,
      functionPtr);
  auto selfHandle = JSObjectInit::initToHandle(runtime, cell);

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

CallResult<PseudoHandle<>> NativeFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  return _nativeCall(vmcast<NativeFunction>(selfHandle.get()), runtime);
}

CallResult<PseudoHandle<JSObject>> NativeFunction::_newObjectImpl(
    Handle<Callable>,
    Runtime &runtime,
    Handle<JSObject>) {
  return runtime.raiseTypeError(
      "This function cannot be used as a constructor.");
}

//===----------------------------------------------------------------------===//
// class NativeConstructor

template <class From>
static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
    PseudoHandle<From> &&other) {
  return PseudoHandle<JSObject>{std::move(other)};
}

template <class From>
static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
    CallResult<PseudoHandle<From>> &&other) {
  return std::move(other);
}

template <class From>
static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
    CallResult<Handle<From>> other) {
  if (LLVM_UNLIKELY(other == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return PseudoHandle<JSObject>{*other};
}

template <class From>
static CallResult<PseudoHandle<JSObject>> toCallResultPseudoHandleJSObject(
    Handle<From> other) {
  return PseudoHandle<JSObject>{other};
}

template <class NativeClass>
CallResult<PseudoHandle<JSObject>> NativeConstructor::creatorFunction(
    Runtime &runtime,
    Handle<JSObject> prototype,
    void *) {
  return toCallResultPseudoHandleJSObject(
      NativeClass::create(runtime, prototype));
}

#define NATIVE_CONSTRUCTOR(fun)                    \
  template CallResult<PseudoHandle<JSObject>> fun( \
      Runtime &, Handle<JSObject>, void *);
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_CONSTRUCTOR

const CallableVTable NativeConstructor::vt{
    {
        VTable(
            CellKind::NativeConstructorKind,
            cellSize<NativeConstructor>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure,
                  NativeConstructor::_snapshotNameImpl,
                  NativeConstructor::_snapshotAddEdgesImpl, nullptr, nullptr
            }
#endif
            ),
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
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<NativeConstructor>());
  NativeFunctionBuildMeta(cell, mb);
  mb.setVTable(&NativeConstructor::vt);
}

#ifndef NDEBUG
CallResult<PseudoHandle<>> NativeConstructor::_callImpl(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  StackFramePtr newFrame{runtime.getStackPointer()};

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

const CallableVTable JSFunction::vt{
    {
        VTable(
            CellKind::JSFunctionKind,
            cellSize<JSFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure, JSFunction::_snapshotNameImpl,
                  JSFunction::_snapshotAddEdgesImpl, nullptr,
                  JSFunction::_snapshotAddLocationsImpl
            }
#endif
            ),
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

void JSFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSFunction>());
  CallableBuildMeta(cell, mb);
  const auto *self = static_cast<const JSFunction *>(cell);
  mb.addField("domain", &self->domain_);
  mb.setVTable(&JSFunction::vt);
}

PseudoHandle<JSFunction> JSFunction::create(
    Runtime &runtime,
    Handle<Domain> domain,
    Handle<JSObject> parentHandle,
    Handle<Environment> envHandle,
    CodeBlock *codeBlock) {
  auto *cell = runtime.makeAFixed<JSFunction, kHasFinalizer>(
      runtime,
      domain,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSFunction>()),
      envHandle,
      codeBlock);
  auto self = JSObjectInit::initToPseudoHandle(runtime, cell);
  self->flags_.lazyObject = 1;
  return self;
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void JSFunction::addLocationToSnapshot(
    HeapSnapshot &snap,
    HeapSnapshot::NodeID id,
    GC &gc) const {
  if (auto location = codeBlock_.get(gc)->getSourceLocation()) {
    snap.addLocation(
        id,
        codeBlock_.get(gc)->getRuntimeModule()->getScriptID(),
        location->line,
        location->column);
  }
}
#endif

CallResult<PseudoHandle<>> JSFunction::_callImpl(
    Handle<Callable> selfHandle,
    Runtime &runtime) {
  auto *self = vmcast<JSFunction>(selfHandle.get());
  CallResult<HermesValue> result{ExecutionStatus::EXCEPTION};
  result = runtime.interpretFunction(self->getCodeBlock(runtime));
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return createPseudoHandle(*result);
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string JSFunction::_snapshotNameImpl(GCCell *cell, GC &gc) {
  auto *const self = vmcast<JSFunction>(cell);
  std::string funcName = Callable::_snapshotNameImpl(self, gc);
  if (!funcName.empty()) {
    return funcName;
  }
  return self->codeBlock_.get(gc)->getNameString(gc.getCallbacks());
}

void JSFunction::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSFunction>(cell);
  // Add the super type's edges too.
  Callable::_snapshotAddEdgesImpl(self, gc, snap);

  // A node for the code block will be added as part of the RuntimeModule.
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Shortcut,
      "codeBlock",
      gc.getIDTracker().getNativeID(self->codeBlock_.get(gc)));
}

void JSFunction::_snapshotAddLocationsImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSFunction>(cell);
  self->addLocationToSnapshot(snap, gc.getObjectID(self), gc);
}
#endif

//===----------------------------------------------------------------------===//
// class JSAsyncFunction

const CallableVTable JSAsyncFunction::vt{
    {
        VTable(
            CellKind::JSAsyncFunctionKind,
            cellSize<JSAsyncFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure,
                  JSAsyncFunction::_snapshotNameImpl,
                  JSAsyncFunction::_snapshotAddEdgesImpl, nullptr, nullptr
            }
#endif
            ),
        JSAsyncFunction::_getOwnIndexedRangeImpl,
        JSAsyncFunction::_haveOwnIndexedImpl,
        JSAsyncFunction::_getOwnIndexedPropertyFlagsImpl,
        JSAsyncFunction::_getOwnIndexedImpl,
        JSAsyncFunction::_setOwnIndexedImpl,
        JSAsyncFunction::_deleteOwnIndexedImpl,
        JSAsyncFunction::_checkAllOwnIndexedImpl,
    },
    JSAsyncFunction::_newObjectImpl,
    JSAsyncFunction::_callImpl};

void JSAsyncFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSAsyncFunction>());
  JSFunctionBuildMeta(cell, mb);
  mb.setVTable(&JSAsyncFunction::vt);
}

PseudoHandle<JSAsyncFunction> JSAsyncFunction::create(
    Runtime &runtime,
    Handle<Domain> domain,
    Handle<JSObject> parentHandle,
    Handle<Environment> envHandle,
    CodeBlock *codeBlock) {
  auto *cell = runtime.makeAFixed<JSAsyncFunction, kHasFinalizer>(
      runtime,
      domain,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSAsyncFunction>()),
      envHandle,
      codeBlock);
  auto self = JSObjectInit::initToPseudoHandle(runtime, cell);
  self->flags_.lazyObject = 1;
  return self;
}

//===----------------------------------------------------------------------===//
// class JSGeneratorFunction

const CallableVTable JSGeneratorFunction::vt{
    {
        VTable(
            CellKind::JSGeneratorFunctionKind,
            cellSize<JSGeneratorFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure,
                  JSGeneratorFunction::_snapshotNameImpl,
                  JSGeneratorFunction::_snapshotAddEdgesImpl, nullptr, nullptr
            }
#endif
            ),
        JSGeneratorFunction::_getOwnIndexedRangeImpl,
        JSGeneratorFunction::_haveOwnIndexedImpl,
        JSGeneratorFunction::_getOwnIndexedPropertyFlagsImpl,
        JSGeneratorFunction::_getOwnIndexedImpl,
        JSGeneratorFunction::_setOwnIndexedImpl,
        JSGeneratorFunction::_deleteOwnIndexedImpl,
        JSGeneratorFunction::_checkAllOwnIndexedImpl,
    },
    JSGeneratorFunction::_newObjectImpl,
    JSGeneratorFunction::_callImpl};

void JSGeneratorFunctionBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSGeneratorFunction>());
  JSFunctionBuildMeta(cell, mb);
  mb.setVTable(&JSGeneratorFunction::vt);
}

PseudoHandle<JSGeneratorFunction> JSGeneratorFunction::create(
    Runtime &runtime,
    Handle<Domain> domain,
    Handle<JSObject> parentHandle,
    Handle<Environment> envHandle,
    CodeBlock *codeBlock) {
  auto *cell = runtime.makeAFixed<JSGeneratorFunction, kHasFinalizer>(
      runtime,
      domain,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSGeneratorFunction>()),
      envHandle,
      codeBlock);
  auto self = JSObjectInit::initToPseudoHandle(runtime, cell);
  self->flags_.lazyObject = 1;
  return self;
}

//===----------------------------------------------------------------------===//
// class GeneratorInnerFunction

const CallableVTable GeneratorInnerFunction::vt{
    {
        VTable(
            CellKind::GeneratorInnerFunctionKind,
            cellSize<GeneratorInnerFunction>(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
            ,
            VTable::HeapSnapshotMetadata {
              HeapSnapshot::NodeType::Closure,
                  GeneratorInnerFunction::_snapshotNameImpl,
                  GeneratorInnerFunction::_snapshotAddEdgesImpl, nullptr,
                  nullptr
            }
#endif
            ),
        GeneratorInnerFunction::_getOwnIndexedRangeImpl,
        GeneratorInnerFunction::_haveOwnIndexedImpl,
        GeneratorInnerFunction::_getOwnIndexedPropertyFlagsImpl,
        GeneratorInnerFunction::_getOwnIndexedImpl,
        GeneratorInnerFunction::_setOwnIndexedImpl,
        GeneratorInnerFunction::_deleteOwnIndexedImpl,
        GeneratorInnerFunction::_checkAllOwnIndexedImpl,
    },
    GeneratorInnerFunction::_newObjectImpl,
    GeneratorInnerFunction::_callImpl};

void GeneratorInnerFunctionBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<GeneratorInnerFunction>());
  JSFunctionBuildMeta(cell, mb);
  const auto *self = static_cast<const GeneratorInnerFunction *>(cell);
  mb.setVTable(&GeneratorInnerFunction::vt);
  mb.addField("savedContext", &self->savedContext_);
  mb.addField("result", &self->result_);
}

CallResult<Handle<GeneratorInnerFunction>> GeneratorInnerFunction::create(
    Runtime &runtime,
    Handle<Domain> domain,
    Handle<JSObject> parentHandle,
    Handle<Environment> envHandle,
    CodeBlock *codeBlock,
    NativeArgs args) {
  auto *cell = runtime.makeAFixed<GeneratorInnerFunction>(
      runtime,
      domain,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<GeneratorInnerFunction>()),
      envHandle,
      codeBlock,
      args.getArgCount());
  auto self = JSObjectInit::initToHandle(runtime, cell);

  const uint32_t ctxSize = getContextSize(codeBlock, args.getArgCount());

  auto ctxRes = ArrayStorage::create(runtime, ctxSize, ctxSize);
  if (LLVM_UNLIKELY(ctxRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto ctx = runtime.makeHandle<ArrayStorage>(*ctxRes);

  // Set "this" as the first element.
  ctx->set(0, args.getThisArg(), runtime.getHeap());

  // Set the rest of the arguments.
  // Argument i goes in slot i+1 to account for the "this".
  for (uint32_t i = 0, e = args.getArgCount(); i < e; ++i) {
    ctx->set(i + 1, args.getArg(i), runtime.getHeap());
  }

  self->savedContext_.set(runtime, ctx.get(), runtime.getHeap());

  return self;
}

/// Call the callable with arguments already on the stack.
CallResult<PseudoHandle<>> GeneratorInnerFunction::callInnerFunction(
    Handle<GeneratorInnerFunction> selfHandle,
    Runtime &runtime,
    Handle<> arg,
    Action action) {
  auto self = Handle<GeneratorInnerFunction>::vmcast(selfHandle);

  SmallHermesValue shv =
      SmallHermesValue::encodeHermesValue(arg.getHermesValue(), runtime);
  self->result_.set(shv, runtime.getHeap());
  self->action_ = action;

  auto ctx = runtime.makeMutableHandle(selfHandle->savedContext_);
  // Account for the `this` argument stored as the first element of ctx.
  const uint32_t argCount = self->argCount_;
  // Generators cannot be used as constructors, so newTarget is always
  // undefined.
  HermesValue newTarget = HermesValue::encodeUndefinedValue();
  ScopedNativeCallFrame frame{
      runtime,
      argCount, // Account for `this`.
      selfHandle.getHermesValue(),
      newTarget,
      ctx->at(0)};
  if (LLVM_UNLIKELY(frame.overflowed()))
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  for (ArrayStorage::size_type i = 0, e = argCount; i < e; ++i) {
    frame->getArgRef(i) = ctx->at(i + 1);
  }

  // Force lazy compilation immediately in order to size the context properly.
  // We're about to call the function anyway, so this doesn't reduce laziness.
  // Note that this will do nothing after the very first time a lazy function
  // is called, so we only resize before we save any registers at all.
  if (LLVM_UNLIKELY(selfHandle->getCodeBlock(runtime)->isLazy())) {
    if (LLVM_UNLIKELY(
            selfHandle->getCodeBlock(runtime)->lazyCompile(runtime) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_UNLIKELY(
            ArrayStorage::resize(
                ctx,
                runtime,
                getContextSize(
                    selfHandle->getCodeBlock(runtime),
                    selfHandle->argCount_)) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    selfHandle->savedContext_.set(runtime, ctx.get(), runtime.getHeap());
  }

  return JSFunction::_callImpl(selfHandle, runtime);
}

void GeneratorInnerFunction::restoreStack(Runtime &runtime) {
  const uint32_t frameOffset = getFrameOffsetInContext();
  const uint32_t frameSize = getFrameSizeInContext(runtime);
  // Start at the lower end of the range to be copied.
  PinnedHermesValue *dst = runtime.getCurrentFrame().ptr();
  assert(
      dst + frameSize <= runtime.getStackPointer() &&
      "writing off the end of the stack");
  const GCHermesValue *src =
      savedContext_.getNonNull(runtime)->data() + frameOffset;
  GCHermesValueUtil::copyToPinned(src, src + frameSize, dst);
}

void GeneratorInnerFunction::saveStack(Runtime &runtime) {
  const uint32_t frameOffset = getFrameOffsetInContext();
  const uint32_t frameSize = getFrameSizeInContext(runtime);
  // Start at the lower end of the range to be copied.
  PinnedHermesValue *first = runtime.getCurrentFrame().ptr();
  assert(
      first + frameSize <= runtime.getStackPointer() &&
      "reading off the end of the stack");
  // Use GCHermesValue::copy to ensure write barriers are executed.
  GCHermesValue::copy(
      first,
      first + frameSize,
      savedContext_.getNonNull(runtime)->data() + frameOffset,
      runtime.getHeap());
}

} // namespace vm
} // namespace hermes
