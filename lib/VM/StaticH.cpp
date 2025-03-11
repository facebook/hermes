/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/SerializedLiteralParser.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/FastArray.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JIT/Config.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSCallableProxy.h"
#include "hermes/VM/JSGeneratorObject.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/ModuleExportsCache-inline.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StaticHUtils.h"
#include "hermes/VM/StringBuilder.h"

#include "JSLib/JSLibInternal.h"

#include <cstdarg>

using namespace hermes;
using namespace hermes::vm;

extern "C" void _SH_MODEL(void) {}

namespace {
/// Convert the given \p cr to a \c CallResult<HermesValue>.
template <typename T>
CallResult<HermesValue> toCallResultHermesValue(CallResult<Handle<T>> cr) {
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return cr->getHermesValue();
}

template <typename T>
CallResult<HermesValue> toCallResultHermesValue(
    CallResult<PseudoHandle<T>> cr) {
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return cr->getHermesValue();
}
} // namespace

extern "C" SHLegacyValue *
_sh_push_locals(SHRuntime *shr, SHLocals *locals, uint32_t stackSize) {
  // TODO: [SH]: check for native stack overflow.
  Runtime &runtime = getRuntime(shr);

  PinnedHermesValue *savedSP = runtime.getStackPointer();

  // Allocate the registers for the new frame, but only if stack != 0.
  if (stackSize &&
      LLVM_UNLIKELY(!runtime.checkAndAllocStack(
          stackSize, HermesValue::encodeUndefinedValue()))) {
    (void)runtime.raiseStackOverflow(
        Runtime::StackOverflowKind::JSRegisterStack);
    _sh_throw_current(shr);
  }

  locals->prev = runtime.shLocals;
  runtime.shLocals = locals;
  return savedSP;
}

extern "C" SHLegacyValue *
_sh_enter(SHRuntime *shr, SHLocals *locals, uint32_t stackSize) {
  // TODO: [SH]: check for native stack overflow.
  Runtime &runtime = getRuntime(shr);

  PinnedHermesValue *frame = runtime.getStackPointer();

  // Allocate the registers for the new frame.
  if (LLVM_UNLIKELY(!runtime.checkAndAllocStack(
          stackSize, HermesValue::encodeUndefinedValue()))) {
    (void)runtime.raiseStackOverflow(
        Runtime::StackOverflowKind::JSRegisterStack);
    _sh_throw_current(shr);
  }

  runtime.setCurrentFrame(StackFramePtr(frame));
  locals->prev = runtime.shLocals;
  runtime.shLocals = locals;
  return frame;
}

extern "C" void _sh_check_native_stack_overflow(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  bool overflowing = runtime.isStackOverflowing();
  if (LLVM_UNLIKELY(overflowing)) {
    (void)runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
    _sh_throw_current(shr);
  }
}

extern "C" void
_sh_pop_locals(SHRuntime *shr, SHLocals *locals, SHLegacyValue *savedSP) {
  Runtime &runtime = getRuntime(shr);
  assert(runtime.shLocals == locals && "Only the current locals can be popped");
  runtime.shLocals = locals->prev;
  runtime.popToSavedStackPointer(toPHV(savedSP));
}

extern "C" void
_sh_leave(SHRuntime *shr, SHLocals *locals, SHLegacyValue *frame) {
  Runtime &runtime = getRuntime(shr);
  assert(runtime.shLocals == locals && "Only the current locals can be popped");
  runtime.shLocals = locals->prev;
  (void)runtime.restoreStackAndPreviousFrame(StackFramePtr(toPHV(frame)));
}

extern "C" SHLegacyValue _sh_ljs_param(SHLegacyValue *frame, uint32_t index) {
  assert(index <= INT32_MAX && "param index should be 31 bits");
  auto framePtr = StackFramePtr(toPHV(frame));
  if (LLVM_LIKELY(index <= framePtr.getArgCount())) {
    // getArgRef() expects `this` to be -1.
    return framePtr.getArgRef((int32_t)index - 1);
  } else {
    return _sh_ljs_undefined();
  }
}

extern "C" SHLegacyValue _sh_ljs_coerce_this_ns(
    SHRuntime *shr,
    SHLegacyValue value) {
  if (LLVM_LIKELY(_sh_ljs_is_object(value))) {
    return value;
  } else if (_sh_ljs_is_null(value) || _sh_ljs_is_undefined(value)) {
    return getRuntime(shr).global_.getHermesValue();
  } else {
    CallResult<HermesValue> res{HermesValue::encodeUndefinedValue()};
    {
      Runtime &runtime = getRuntime(shr);
      GCScopeMarkerRAII marker{runtime};
      res = toObject(
          runtime, runtime.makeHandle(HermesValue::fromRaw(value.raw)));
    }
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      _sh_throw_current(shr);
    return *res;
  }
}

static SHLegacyValue getArgumentsPropByVal_RJS(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *idx,
    SHLegacyValue *lazyReg,
    bool strictMode) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr framePtr(toPHV(frame));
  // If the arguments object hasn't been created yet and we have a
  // valid integer index, we use the fast path.
  if (toPHV(lazyReg)->isUndefined()) {
    if (auto index = toArrayIndexFastPath(*toPHV(idx))) {
      // Is this an existing argument?
      if (*index < framePtr.getArgCount())
        return framePtr.getArgRef(*index);
    }
  }
  // Slow path.
  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = Interpreter::getArgumentsPropByValSlowPath_RJS(
        runtime,
        toPHV(lazyReg),
        toPHV(idx),
        framePtr.getCalleeClosureHandleUnsafe(),
        strictMode);
  }
  if (res == ExecutionStatus::EXCEPTION)
    _sh_throw_current(shr);
  return res->getHermesValue();
}

extern "C" SHLegacyValue _sh_ljs_get_arguments_prop_by_val_loose(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *idx,
    SHLegacyValue *lazyReg) {
  return getArgumentsPropByVal_RJS(shr, frame, idx, lazyReg, false);
}
extern "C" SHLegacyValue _sh_ljs_get_arguments_prop_by_val_strict(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *idx,
    SHLegacyValue *lazyReg) {
  return getArgumentsPropByVal_RJS(shr, frame, idx, lazyReg, true);
}
extern "C" SHLegacyValue _sh_ljs_get_arguments_length(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr framePtr(toPHV(frame));
  // If the arguments object hasn't been created yet, use the fast path.
  if (toPHV(lazyReg)->isUndefined())
    return HermesValue::encodeTrustedNumberValue(framePtr.getArgCount());

  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    // The arguments object has been created, so this is a regular property
    // get.
    auto obj = Handle<JSObject>::vmcast(toPHV(lazyReg));
    res = JSObject::getNamed_RJS(
        obj, runtime, Predefined::getSymbolID(Predefined::length));
  }
  if (res == ExecutionStatus::EXCEPTION)
    _sh_throw_current(shr);
  return res->getHermesValue();
}

static void reifyArguments(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg,
    bool strictMode) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr framePtr(toPHV(frame));
  // If the arguments object was already created, do nothing.
  if (!toPHV(lazyReg)->isUndefined()) {
    assert(
        toPHV(lazyReg)->isObject() &&
        "arguments lazy register is not an object");
    return;
  }

  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = toCallResultHermesValue(Interpreter::reifyArgumentsSlowPath(
        runtime, framePtr.getCalleeClosureHandleUnsafe(), strictMode));
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  *lazyReg = *res;
}

extern "C" void _sh_ljs_reify_arguments_loose(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg) {
  reifyArguments(shr, frame, lazyReg, false);
}
extern "C" void _sh_ljs_reify_arguments_strict(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg) {
  reifyArguments(shr, frame, lazyReg, true);
}

extern "C" SHLegacyValue _sh_ljs_get_by_val_with_receiver_rjs(
    SHRuntime *shr,
    SHLegacyValue *source,
    SHLegacyValue *key,
    SHLegacyValue *receiver) {
  Runtime &runtime = getRuntime(shr);
  Handle<> sourceHandle{toPHV(source)};
  Handle<> keyHandle{toPHV(key)};
  Handle<> receiverHandle{(toPHV(receiver))};
  if (LLVM_LIKELY(sourceHandle->isObject())) {
    CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker{runtime};
      res = JSObject::getComputedWithReceiver_RJS(
          Handle<JSObject>::vmcast(sourceHandle),
          runtime,
          keyHandle,
          receiverHandle);
    }
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      _sh_throw_current(shr);
    return res->getHermesValue();
  }

  // This is the "slow path".
  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = Interpreter::getByValTransientWithReceiver_RJS(
        runtime, sourceHandle, keyHandle, receiverHandle);
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return res->getHermesValue();
}

extern "C" SHLegacyValue
_sh_ljs_get_by_index_rjs(SHRuntime *shr, SHLegacyValue *source, uint32_t key) {
  Handle<> sourceHandle{toPHV(source)};
  Runtime &runtime = getRuntime(shr);
  if (LLVM_LIKELY(sourceHandle->isObject())) {
    Handle<JSObject> objHandle = Handle<JSObject>::vmcast(sourceHandle);
    if (LLVM_LIKELY(objHandle->hasFastIndexProperties())) {
      GCScopeMarkerRAII marker{runtime};
      auto ourValue = createPseudoHandle(JSObject::getOwnIndexed(
          createPseudoHandle(*objHandle), runtime, key));
      if (LLVM_LIKELY(!ourValue->isEmpty())) {
        return ourValue.getHermesValue();
      }
    }
  }

  // Otherwise...
  // This is the "slow path".
  auto res = [&]() {
    GCScopeMarkerRAII marker{runtime};
    struct : public Locals {
      PinnedValue<> key;
    } lv;
    LocalsRAII lraii{runtime, &lv};

    lv.key = HermesValue::encodeTrustedNumberValue(key);
    return Interpreter::getByValTransient_RJS(runtime, sourceHandle, lv.key);
  }();

  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

extern "C" SHLegacyValue _sh_catch(
    SHRuntime *shr,
    SHLocals *locals,
    SHLegacyValue *frame,
    uint32_t stackSize) {
  Runtime &runtime = getRuntime(shr);
  runtime.shCurJmpBuf = runtime.shCurJmpBuf->prev;
  _sh_catch_no_pop(shr, locals, frame, stackSize);
  return _sh_get_clear_thrown_value(shr);
}

extern "C" void _sh_catch_no_pop(
    SHRuntime *shr,
    SHLocals *locals,
    SHLegacyValue *frame,
    uint32_t stackSize) {
  Runtime &runtime = getRuntime(shr);

  runtime.shLocals = locals;
  runtime.popToSavedStackPointer(toPHV(frame + stackSize));
  runtime.setCurrentFrame(StackFramePtr(toPHV(frame)));
}

extern "C" SHLegacyValue _sh_get_clear_thrown_value(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  SHLegacyValue res = runtime.getThrownValue();
  runtime.clearThrownValue();
  return res;
}

extern "C" void _sh_throw_current(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  assert(runtime.shCurJmpBuf && "No SH exception handler installed");
  if (!runtime.shCurJmpBuf) {
    fprintf(stderr, "SH: uncaught exception");
    abort();
  }
  _sh_longjmp(runtime.shCurJmpBuf->buf, 1);
  // longjmp is not marked as noreturn, so use llvm_unreachable to avoid a
  // compiler warning that this function doesn't actually seem to be noreturn.
  llvm_unreachable("longjmp cannot return");
}

extern "C" void _sh_throw(SHRuntime *shr, SHLegacyValue value) {
  Runtime &runtime = getRuntime(shr);
  runtime.setThrownValue(HermesValue::fromRaw(value.raw));
  _sh_throw_current(shr);
}

extern "C" void _sh_throw_type_error(SHRuntime *shr, SHLegacyValue *message) {
  (void)getRuntime(shr).raiseTypeError(Handle<>(toPHV(message)));
  _sh_throw_current(shr);
}

extern "C" void _sh_throw_type_error_ascii(
    SHRuntime *shr,
    const char *message) {
  (void)getRuntime(shr).raiseTypeError(TwineChar16(message));
  _sh_throw_current(shr);
}

extern "C" void _sh_throw_reference_error_ascii(
    SHRuntime *shr,
    const char *message) {
  Runtime &runtime = getRuntime(shr);
  (void)runtime.raiseReferenceError(message);
  _sh_throw_current(shr);
}

extern "C" void _sh_throw_empty(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  (void)runtime.raiseReferenceError("accessing an uninitialized variable");
  _sh_throw_current(shr);
}

static SHLegacyValue doCall(Runtime &runtime, PinnedHermesValue *callTarget) {
  if (vmisa<NativeJSFunction>(*callTarget)) {
    return NativeJSFunction::_legacyCall(
        getSHRuntime(runtime), vmcast<NativeJSFunction>(*callTarget));
  }

  if (vmisa<JSFunction>(*callTarget)) {
    JSFunction *jsFunc = vmcast<JSFunction>(*callTarget);
#if HERMESVM_JIT
    if (auto *fnPtr = jsFunc->getCodeBlock()->getJITCompiled())
      return fnPtr(&runtime);
#elif !defined(HERMESVM_JIT)
#error JIT/Config.h has not been included
#endif
    CallResult<HermesValue> result = jsFunc->_interpret(runtime);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));

    return result.getValue();
  }

  // FIXME: check for register stack overflow.
  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};

  {
    GCScopeMarkerRAII marker{runtime};
    if (vmisa<NativeFunction>(*callTarget)) {
      auto *native = vmcast<NativeFunction>(*callTarget);
      res = NativeFunction::_nativeCall(native, runtime);
    } else if (vmisa<BoundFunction>(*callTarget)) {
      auto *bound = vmcast<BoundFunction>(*callTarget);
      res = BoundFunction::_boundCall(bound, runtime.getCurrentIP(), runtime);
    } else if (vmisa<Callable>(*callTarget)) {
      auto callable = Handle<Callable>::vmcast(callTarget);
      res = callable->call(callable, runtime);
    } else {
      res = runtime.raiseTypeErrorForValue(
          Handle<>(callTarget), " is not a function");
    }
  }

  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(getSHRuntime(runtime));
  }

  return res->getHermesValue();
}

extern "C" SHLegacyValue
_sh_ljs_call(SHRuntime *shr, SHLegacyValue *frame, uint32_t argCount) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr newFrame(runtime.getStackPointer());
  newFrame.getPreviousFrameRef() = HermesValue::encodeNativePointer(frame);
  newFrame.getSavedIPRef() = HermesValue::encodeNativePointer(nullptr);
  newFrame.getSavedCodeBlockRef() = HermesValue::encodeNativePointer(nullptr);
  newFrame.getSHLocalsRef() = HermesValue::encodeNativePointer(nullptr);
  newFrame.getArgCountRef() = HermesValue::encodeNativeUInt32(argCount);
  return doCall(runtime, &newFrame.getCalleeClosureOrCBRef());
}

extern "C" SHLegacyValue _sh_ljs_callRequire(
    SHRuntime *shr,
    SHArrayStorage **exportCache,
    SHLegacyValue *requireFunc,
    uint32_t modIndex) {
  // Check the cache.
  SHLegacyValue res = module_export_cache::get(
      reinterpret_cast<ArrayStorage *>(*exportCache), modIndex);
  if (!_sh_ljs_is_empty(res)) {
    return res;
  }

  Runtime &runtime = getRuntime(shr);
  auto slowRes = [&]() -> CallResult<SHLegacyValue> {
    struct : public Locals {
      PinnedValue<> modExport;
      PinnedValue<Callable> reqFuncPV;
    } lv;
    LocalsRAII lraii{runtime, &lv};

    // The value should be a Callable, or else we raise an error (as
    // Interpreter::handleCallSlowPath does).
    auto *reqPHV = toPHV(requireFunc);
    Callable *reqFuncCallable = dyn_vmcast_or_null<Callable>(*reqPHV);
    if (!reqFuncCallable) {
      (void)runtime.raiseTypeErrorForValue(
          Handle<>(reqPHV), " is not a function");
      return ExecutionStatus::EXCEPTION;
    }

    lv.reqFuncPV = reqFuncCallable;
    CallResult<PseudoHandle<>> modExport = Callable::executeCall1(
        lv.reqFuncPV,
        runtime,
        HandleRootOwner::getUndefinedValue(),
        HermesValue::encodeTrustedNumberValue(modIndex));
    if (LLVM_UNLIKELY(modExport == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;

    lv.modExport = modExport->get();

    // Populate the cache.
    module_export_cache::set(
        getRuntime(shr),
        reinterpret_cast<ArrayStorage *&>(*exportCache),
        modIndex,
        lv.modExport);

    // Whether or not the caching above succeeded, return the module export.
    return lv.modExport.get();
  }();
  if (LLVM_UNLIKELY(slowRes == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return *slowRes;
}

extern "C" SHLegacyValue _sh_ljs_call_builtin(
    SHRuntime *shr,
    SHLegacyValue *frame,
    uint32_t argCount,
    uint32_t builtinMethodID) {
  auto res = [&]() {
    Runtime &runtime = getRuntime(shr);
    StackFramePtr newFrame(runtime.getStackPointer());
    newFrame.getPreviousFrameRef() = HermesValue::encodeNativePointer(frame);
    newFrame.getSavedIPRef() = HermesValue::encodeNativePointer(nullptr);
    newFrame.getSavedCodeBlockRef() = HermesValue::encodeNativePointer(nullptr);
    newFrame.getSHLocalsRef() = HermesValue::encodeNativePointer(nullptr);
    newFrame.getArgCountRef() = HermesValue::encodeNativeUInt32(argCount);
    newFrame.getNewTargetRef() = HermesValue::encodeUndefinedValue();

    auto callee =
        vmcast<NativeFunction>(runtime.getBuiltinCallable(builtinMethodID));
    newFrame.getCalleeClosureOrCBRef() = HermesValue::encodeObjectValue(callee);

    GCScopeMarkerRAII marker{runtime};
    return NativeFunction::_nativeCall(callee, runtime);
  }();
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return res->getHermesValue();
}

extern "C" SHLegacyValue _sh_ljs_get_builtin_closure(
    SHRuntime *shr,
    uint32_t builtinMethodID) {
  return HermesValue::encodeObjectValue(
      getRuntime(shr).getBuiltinCallable(builtinMethodID));
}

extern "C" SHLegacyValue _sh_ljs_create_environment(
    SHRuntime *shr,
    const SHLegacyValue *parentEnv,
    uint32_t size) {
  Runtime &runtime = getRuntime(shr);

  auto parentHandle = parentEnv
      ? Handle<Environment>::vmcast(toPHV(parentEnv))
      : HandleRootOwner::makeNullHandle<Environment>();

  GCScopeMarkerRAII marker{runtime};
  return Environment::create(runtime, parentHandle, size);
  // #ifdef HERMES_ENABLE_DEBUGGER
  //   framePtr.getDebugEnvironmentRef() = *res;
  // #endif
}

extern "C" SHLegacyValue
_sh_ljs_get_env(SHRuntime *shr, SHLegacyValue startEnv, uint32_t level) {
  Runtime &runtime = getRuntime(shr);
  Environment *curEnv = vmcast<Environment>(*toPHV(&startEnv));
  while (level--) {
    assert(curEnv && "invalid environment relative level");
    curEnv = curEnv->getParentEnvironment(runtime);
  }

  return HermesValue::encodeObjectValue(curEnv);
}

extern "C" void _sh_ljs_store_to_env(
    SHRuntime *shr,
    SHLegacyValue env,
    SHLegacyValue val,
    uint32_t index) {
  vmcast<Environment>(HermesValue::fromRaw(env.raw))
      ->slot(index)
      .set(HermesValue::fromRaw(val.raw), getRuntime(shr).getHeap());
}

extern "C" void _sh_ljs_store_np_to_env(
    SHRuntime *shr,
    SHLegacyValue env,
    SHLegacyValue val,
    uint32_t index) {
  vmcast<Environment>(HermesValue::fromRaw(env.raw))
      ->slot(index)
      .setNonPtr(HermesValue::fromRaw(val.raw), getRuntime(shr).getHeap());
}

extern "C" SHLegacyValue _sh_ljs_create_generator_object(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHLegacyValue (*func)(SHRuntime *),
    const SHNativeFuncInfo *funcInfo,
    const SHUnit *unit) {
  assert(
      !_sh_ljs_is_null(*env) && "inner generator cannot have null environment");
  Runtime &runtime = getRuntime(shr);

  auto genObjRes =
      [&runtime, env, func, funcInfo, unit]() -> CallResult<SHLegacyValue> {
    GCScopeMarkerRAII marker{runtime};
    Handle<NativeJSFunction> innerFunc = NativeJSFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime.functionPrototype),
        env ? Handle<Environment>::vmcast(toPHV(env))
            : Runtime::makeNullHandle<Environment>(),
        func,
        funcInfo,
        unit);
    auto generatorFunction = runtime.makeHandle(vmcast<NativeJSFunction>(
        runtime.getCurrentFrame().getCalleeClosureUnsafe()));
    assert(
        generatorFunction->getFunctionInfo()->kind == FuncKind::Generator &&
        "should be called from a generator function");

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
    return JSGeneratorObject::create(runtime, innerFunc, prototype)
        ->getHermesValue();
  }();

  if (genObjRes == ExecutionStatus::EXCEPTION)
    _sh_throw_current(shr);

  return *genObjRes;
}

extern "C" SHLegacyValue _sh_ljs_create_class(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHLegacyValue (*func)(SHRuntime *),
    const SHNativeFuncInfo *funcInfo,
    const SHUnit *unit,
    SHLegacyValue *homeObjectOut,
    SHLegacyValue *superClass) {
  Runtime &runtime = getRuntime(shr);
  GCScopeMarkerRAII marker{runtime};
  auto classRes = createClass(
      runtime,
      superClass ? Handle{toPHV(superClass)} : Runtime::getEmptyValue(),
      [&runtime, env, func, funcInfo, unit, superClass](
          Handle<JSObject> ctorParent) {
        auto envHandle = env ? Handle<Environment>::vmcast(toPHV(env))
                             : HandleRootOwner::makeNullHandle<Environment>();
        // Derived classes get their own special CellKind.
        return superClass
            ? *NativeJSDerivedClass::create(
                  runtime, ctorParent, envHandle, func, funcInfo, unit)
            : *NativeJSFunction::create(
                  runtime, ctorParent, envHandle, func, funcInfo, unit);
      });
  if (classRes == ExecutionStatus::EXCEPTION)
    _sh_throw_current(shr);
  *homeObjectOut = HermesValue::encodeObjectValue(std::get<1>(*classRes));
  return HermesValue::encodeObjectValue(std::get<0>(*classRes));
}

extern "C" SHLegacyValue _sh_ljs_create_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHLegacyValue (*func)(SHRuntime *),
    const SHNativeFuncInfo *funcInfo,
    const SHUnit *unit) {
  Runtime &runtime = getRuntime(shr);
  GCScopeMarkerRAII marker{runtime};
  return NativeJSFunction::createWithInferredParent(
             runtime,
             env ? Handle<Environment>::vmcast(toPHV(env))
                 : Runtime::makeNullHandle<Environment>(),
             func,
             funcInfo,
             unit)
      .getHermesValue();
}

extern "C" SHLegacyValue _sh_ljs_get_global_object(SHRuntime *shr) {
  return getRuntime(shr).global_.getHermesValue();
}

extern "C" void _sh_ljs_declare_global_var(SHRuntime *shr, SHSymbolID name) {
  Runtime &runtime = getRuntime(shr);
  {
    GCScopeMarkerRAII mark{runtime};

    DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
    dpf.configurable = 0;
    // Do not overwrite existing globals with undefined.
    dpf.setValue = 0;

    auto res = JSObject::defineOwnProperty(
        runtime.getGlobal(),
        runtime,
        SymbolID::unsafeCreate(name),
        dpf,
        Runtime::getUndefinedValue(),
        PropOpFlags().plusThrowOnError());
    if (res != ExecutionStatus::EXCEPTION)
      return;
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
    auto res1 = JSObject::getOwnNamedDescriptor(
        runtime.getGlobal(), runtime, SymbolID::unsafeCreate(name), desc);
    if (res1) {
      runtime.clearThrownValue();
      return;
    }
    // fall through for exception
  }
  _sh_throw_current(shr);
}

/// Calculate the default property access flags depending on the mode of the
/// \c CodeBlock.
#define DEFAULT_PROP_OP_FLAGS(strictMode) \
  (strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags())

template <bool tryProp, bool strictMode>
static inline void putById_RJS(
    Runtime &runtime,
    const PinnedHermesValue *target,
    SymbolID symID,
    const PinnedHermesValue *value,
    WritePropertyCacheEntry *cacheEntry) {
  //++NumPutById;
  if (LLVM_LIKELY(target->isObject())) {
    SmallHermesValue shv = SmallHermesValue::encodeHermesValue(*value, runtime);
    auto *obj = vmcast<JSObject>(*target);

    // #ifdef HERMESVM_PROFILER_BB
#if 0
    {
      HERMES_SLOW_ASSERT(
          gcScope.getHandleCountDbg() == KEEP_HANDLES &&
          "unaccounted handles were created");
      auto shvHandle = runtime.makeHandle(shv.toHV(runtime));
      auto cacheHCPtr = vmcast_or_null<HiddenClass>(static_cast<GCCell *>(
          cacheEntry->clazz.get(runtime, runtime.getHeap())));
      CAPTURE_IP(runtime.recordHiddenClass(
          curCodeBlock, ip, ID(idVal), obj->getClass(runtime), cacheHCPtr));
      // shv/obj may be invalidated by recordHiddenClass
      if (shv.isPointer())
        shv.unsafeUpdatePointer(
            static_cast<GCCell *>(shvHandle->getPointer()), runtime);
      obj = vmcast<JSObject>(*target);
    }
    gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
    CompressedPointer clazzPtr{obj->getClassGCPtr()};
    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry && cacheEntry->clazz == clazzPtr)) {
      //++NumPutByIdCacheHits;
      JSObject::setNamedSlotValueUnsafe(obj, runtime, cacheEntry->slot, shv);
      return;
    }
    NamedPropertyDescriptor desc;
    OptValue<bool> hasOwnProp =
        JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, symID, desc);
    if (LLVM_LIKELY(hasOwnProp.hasValue() && hasOwnProp.getValue()) &&
        !desc.flags.accessor && desc.flags.writable &&
        !desc.flags.internalSetter) {
      //++NumPutByIdFastPaths;

      // cacheIdx == 0 indicates no caching so don't update the cache in
      // those cases.
      HiddenClass *clazz = vmcast<HiddenClass>(clazzPtr.getNonNull(runtime));
      if (LLVM_LIKELY(!clazz->isDictionary()) && LLVM_LIKELY(cacheEntry)) {
#ifdef HERMES_SLOW_DEBUG
        // if (cacheEntry->clazz && cacheEntry->clazz != clazzPtr)
        //   ++NumPutByIdCacheEvicts;
#else
        //(void)NumPutByIdCacheEvicts;
#endif
        // Cache the class and property slot.
        cacheEntry->clazz = clazzPtr;
        cacheEntry->slot = desc.slot;
      }

      // This must be valid because an own property was already found.
      JSObject::setNamedSlotValueUnsafe(obj, runtime, desc.slot, shv);
      return;
    }

    CallResult<bool> putRes{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker{runtime};
      const PropOpFlags defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(strictMode);
      putRes = JSObject::putNamed_RJS(
          Handle<JSObject>::vmcast(target),
          runtime,
          symID,
          Handle<>(value),
          !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist());
    }
    if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));
  } else {
    //++NumPutByIdTransient;
    assert(!tryProp && "TryPutById can only be used on the global object");
    ExecutionStatus retStatus;
    {
      GCScopeMarkerRAII marker{runtime};
      retStatus = Interpreter::putByIdTransient_RJS(
          runtime, Handle<>(target), symID, Handle<>(value), strictMode);
    }
    if (retStatus == ExecutionStatus::EXCEPTION)
      _sh_throw_current(getSHRuntime(runtime));
  }
}

extern "C" void _sh_ljs_put_by_id_loose_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    SHWritePropertyCacheEntry *propCacheEntry) {
  putById_RJS<false, false>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<WritePropertyCacheEntry *>(propCacheEntry));
}

extern "C" void _sh_ljs_put_by_id_strict_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    SHWritePropertyCacheEntry *propCacheEntry) {
  putById_RJS<false, true>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<WritePropertyCacheEntry *>(propCacheEntry));
}

extern "C" void _sh_ljs_try_put_by_id_loose_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    SHWritePropertyCacheEntry *propCacheEntry) {
  putById_RJS<true, true>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<WritePropertyCacheEntry *>(propCacheEntry));
}

extern "C" void _sh_ljs_try_put_by_id_strict_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    SHWritePropertyCacheEntry *propCacheEntry) {
  putById_RJS<true, true>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<WritePropertyCacheEntry *>(propCacheEntry));
}

static inline void putByValWithReceiver_RJS(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value,
    SHLegacyValue *receiver,
    bool strictMode) {
  Handle<> targetHandle{toPHV(target)}, keyHandle{toPHV(key)},
      valueHandle{toPHV(value)}, receiverHandle{toPHV(receiver)};
  Runtime &runtime = getRuntime(shr);
  if (LLVM_LIKELY(targetHandle->isObject())) {
    const PropOpFlags defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(strictMode);
    CallResult<bool> res{false};
    {
      GCScopeMarkerRAII marker{runtime};
      res = JSObject::putComputedWithReceiver_RJS(
          Handle<JSObject>::vmcast(targetHandle),
          runtime,
          keyHandle,
          valueHandle,
          receiverHandle,
          defaultPropOpFlags);
    }
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      _sh_throw_current(shr);
    return;
  }

  // This is the "slow path".
  ExecutionStatus res;
  {
    GCScopeMarkerRAII marker{runtime};
    res = Interpreter::putByValTransient_RJS(
        runtime, targetHandle, keyHandle, valueHandle, strictMode);
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

static inline void putByVal_RJS(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value,
    bool strictMode) {
  putByValWithReceiver_RJS(shr, target, key, value, target, strictMode);
}

extern "C" void _sh_ljs_put_by_val_loose_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value) {
  putByVal_RJS(shr, target, key, value, false);
}
extern "C" void _sh_ljs_put_by_val_strict_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value) {
  putByVal_RJS(shr, target, key, value, true);
}
extern "C" void _sh_ljs_put_by_val_with_receiver_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value,
    SHLegacyValue *receiver,
    bool isStrict) {
  putByValWithReceiver_RJS(shr, target, key, value, receiver, isStrict);
}

template <bool tryProp>
static inline HermesValue getByIdWithReceiver_RJS(
    Runtime &runtime,
    Handle<> source,
    SymbolID symID,
    Handle<> receiver,
    ReadPropertyCacheEntry *cacheEntry) {
  //++NumGetById;
  // NOTE: it is safe to use OnREG(GetById) here because all instructions
  // have the same layout: opcode, registers, non-register operands, i.e.
  // they only differ in the width of the last "identifier" field.
  if (LLVM_LIKELY(source->isObject())) {
    auto *obj = vmcast<JSObject>(*source);

    CompressedPointer clazzPtr{obj->getClassGCPtr()};
#ifndef NDEBUG
    // if (vmcast<HiddenClass>(clazzPtr.getNonNull(runtime))->isDictionary())
    //   ++NumGetByIdDict;
#else
    //(void)NumGetByIdDict;
#endif

    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry && cacheEntry->clazz == clazzPtr)) {
      //++NumGetByIdCacheHits;
      return JSObject::getNamedSlotValueUnsafe(obj, runtime, cacheEntry->slot)
          .unboxToHV(runtime);
    }

    // See if it's a proto cache hit.
    if (LLVM_LIKELY(cacheEntry->negMatchClazz == clazzPtr)) {
      // Proxy, HostObject and lazy objects have special hidden classes, so they
      // should never match the cached class.
      assert(!obj->getFlags().proxyObject);
      assert(!obj->getFlags().hostObject);
      assert(!obj->getFlags().lazyObject);
      const GCPointer<JSObject> &parentGCPtr = obj->getParentGCPtr();
      if (LLVM_LIKELY(parentGCPtr)) {
        JSObject *parent = parentGCPtr.getNonNull(runtime);
        if (LLVM_LIKELY(cacheEntry->clazz == parent->getClassGCPtr())) {
          return JSObject::getNamedSlotValueUnsafe(
                     parent, runtime, cacheEntry->slot)
              .unboxToHV(runtime);
        }
      }
    }

    NamedPropertyDescriptor desc;
    OptValue<bool> fastPathResult =
        JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, symID, desc);
    if (LLVM_LIKELY(fastPathResult.hasValue() && fastPathResult.getValue()) &&
        !desc.flags.accessor) {
      //++NumGetByIdFastPaths;

      // cacheIdx == 0 indicates no caching so don't update the cache in
      // those cases.
      HiddenClass *clazz = vmcast<HiddenClass>(clazzPtr.getNonNull(runtime));
      if (LLVM_LIKELY(!clazz->isDictionaryNoCache()) &&
          LLVM_LIKELY(cacheEntry)) {
#ifdef HERMES_SLOW_DEBUG
        // if (cacheEntry->clazz && cacheEntry->clazz != clazzPtr)
        //   ++NumGetByIdCacheEvicts;
#else
        //(void)NumGetByIdCacheEvicts;
#endif
        // Cache the class, id and property slot.
        cacheEntry->clazz = clazzPtr;
        cacheEntry->slot = desc.slot;
      }

      assert(
          !obj->isProxyObject() &&
          "tryGetOwnNamedDescriptorFast returned true on Proxy");
      return JSObject::getNamedSlotValueUnsafe(obj, runtime, desc)
          .unboxToHV(runtime);
    }

#ifdef HERMES_SLOW_DEBUG
    // Call to getNamedDescriptorUnsafe is safe because `id` is kept alive
    // by the IdentifierTable.
    // JSObject *propObj = JSObject::getNamedDescriptorUnsafe(
    //    Handle<JSObject>::vmcast(&O2REG(GetById)), runtime, id, desc);
    // if (propObj) {
    //  if (desc.flags.accessor)
    //    ++NumGetByIdAccessor;
    //  else if (propObj != vmcast<JSObject>(O2REG(GetById)))
    //    ++NumGetByIdProto;
    //} else {
    //  ++NumGetByIdNotFound;
    //}
#else
    //(void)NumGetByIdAccessor;
    //(void)NumGetByIdProto;
    //(void)NumGetByIdNotFound;
#endif
#ifdef HERMES_SLOW_DEBUG
    // auto *savedClass = true /*cacheEntry*/
    //     ? cacheEntry->clazz.get(runtime, runtime.getHeap())
    //     : nullptr;
#endif
    //++NumGetByIdSlow;
    CallResult<PseudoHandle<>> resPH{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker(runtime);
      const PropOpFlags defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(false);
      resPH = JSObject::getNamedWithReceiver_RJS(
          Handle<JSObject>::vmcast(source),
          runtime,
          symID,
          receiver,
          !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist(),
          cacheEntry);
    }
    if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));
#ifdef HERMES_SLOW_DEBUG
      // if (cacheEntry && savedClass &&
      //     cacheEntry->clazz.get(runtime, runtime.getHeap()) != savedClass) {
      //   ++NumGetByIdCacheEvicts;
      // }
#endif
    return resPH->get();
  } else {
    //++NumGetByIdTransient;
    assert(!tryProp && "TryGetById can only be used on the global object");
    /* Slow path. */
    CallResult<PseudoHandle<>> resPH{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker{runtime};
      resPH = Interpreter::getByIdTransientWithReceiver_RJS(
          runtime, source, symID, receiver);
    }
    if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));
    return resPH->get();
  }
}

/// Assume the receiver is the same as the source object.
template <bool tryProp>
static inline HermesValue getById_RJS(
    Runtime &runtime,
    Handle<> source,
    SymbolID symID,
    ReadPropertyCacheEntry *cacheEntry) {
  return getByIdWithReceiver_RJS<tryProp>(
      runtime, source, symID, source, cacheEntry);
}

extern "C" SHLegacyValue _sh_ljs_create_this(
    SHRuntime *shr,
    SHLegacyValue *callee,
    SHLegacyValue *newTarget,
    SHReadPropertyCacheEntry *propCacheEntry) {
  Runtime &runtime = getRuntime(shr);
  auto *calleePHV = toPHV(callee);
  auto *newTargetPHV = toPHV(newTarget);
  {
    // TODO(T168592126) standardize on where we perform function call
    // validation for the native backend.
    if (LLVM_UNLIKELY(!vmisa<Callable>(*calleePHV))) {
      (void)runtime.raiseTypeErrorForValue(
          Handle<>(calleePHV), " cannot be used as a constructor.");
      goto throwCurrent;
    }
    if (LLVM_UNLIKELY(!vmisa<Callable>(*newTargetPHV))) {
      (void)runtime.raiseTypeErrorForValue(
          Handle<>(newTargetPHV), " invalid new.target.");
      goto throwCurrent;
    }

    auto *calleeFunc = vmcast<JSObject>(*calleePHV);
    auto cellKind = calleeFunc->getKind();
    Callable *correctNewTarget;
    // If this is a callable that expects a `this`, then skip ahead and start
    // making the object.
    if (cellKind >= CellKind::CallableExpectsThisKind_first) {
      correctNewTarget = vmcast<Callable>(*toPHV(newTarget));
    } else if (cellKind >= CellKind::CallableMakesThisKind_first) {
      // Callables that make their own this should be given undefined in a
      // construct call.
      return HermesValue::encodeUndefinedValue();
    } else if (cellKind >= CellKind::CallableUnknownMakesThisKind_first) {
      correctNewTarget = vmcast<Callable>(*toPHV(newTarget));
      while (auto *bound = dyn_vmcast<BoundFunction>(calleeFunc)) {
        calleeFunc = bound->getTarget(runtime);
        // From ES15 10.4.1.2 [[Construct]] Step 5: If SameValue(F, newTarget)
        // is true, set newTarget to target.
        if (bound == vmcast<Callable>(correctNewTarget)) {
          correctNewTarget = vmcast<Callable>(calleeFunc);
        }
      }
      cellKind = calleeFunc->getKind();
      // Repeat the checks, now against the target.
      if (cellKind >= CellKind::CallableExpectsThisKind_first) {
        // Do nothing, correctNewTarget is already set up correctly.
      } else if (cellKind >= CellKind::CallableMakesThisKind_first) {
        return HermesValue::encodeUndefinedValue();
      } else {
        // If we still can't recognize what to do after advancing through bound
        // functions (if any), error out. This code path is hit when invoking a
        // NativeFunction as a constructor.
        (void)runtime.raiseTypeError(
            "This function cannot be used as a constructor.");
        goto throwCurrent;
      }
    } else {
      // Not a Callable.
      (void)runtime.raiseTypeErrorForValue(
          Handle<>(calleePHV), " cannot be used as a constructor.");
      goto throwCurrent;
    }

    // We shouldn't need to check that new.target is a constructor explicitly.
    // There are 2 cases where this instruction is emitted.
    // 1. new expressions. In this case, new.target == callee. We always verify
    // that a function call is performed correctly, so don't need to
    // double-verify new.target.
    // 2. super() calls in derived constructors. In this case, new.target will
    // be set to the original callee for the new expression which triggered the
    // constructor now invoking super. This means that new.target will be
    // checked. Unfortunately, this check for constructors is done *after* this
    // instruction, so we can't add an assert that newTarget is a constructor
    // here. For example, using `new` on an arrow function, newTarget is not a
    // constructor, but we don't find that out until after this instruction.

    struct : public Locals {
      PinnedValue<Callable> newTarget;
      // This is the .prototype of new.target
      PinnedValue<> newTargetPrototype;
    } lv;
    LocalsRAII lraii(runtime, &lv);
    lv.newTarget = correctNewTarget;
    lv.newTargetPrototype = getById_RJS<false>(
        getRuntime(shr),
        lv.newTarget,
        Predefined::getSymbolID(Predefined::prototype),
        reinterpret_cast<ReadPropertyCacheEntry *>(propCacheEntry));

    return JSObject::create(
               runtime,
               lv.newTargetPrototype->isObject()
                   ? Handle<JSObject>::vmcast(&lv.newTargetPrototype)
                   : runtime.objectPrototype)
        .getHermesValue();
  }
throwCurrent:
  _sh_throw_current(shr);
}

extern "C" SHLegacyValue _sh_ljs_try_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    SHReadPropertyCacheEntry *propCacheEntry) {
  return getById_RJS<true>(
      getRuntime(shr),
      Handle<>{toPHV(source)},
      SymbolID::unsafeCreate(symID),
      reinterpret_cast<ReadPropertyCacheEntry *>(propCacheEntry));
}
extern "C" SHLegacyValue _sh_ljs_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    SHReadPropertyCacheEntry *propCacheEntry) {
  return getById_RJS<false>(
      getRuntime(shr),
      Handle<>{toPHV(source)},
      SymbolID::unsafeCreate(symID),
      reinterpret_cast<ReadPropertyCacheEntry *>(propCacheEntry));
}

extern "C" SHLegacyValue _sh_ljs_get_by_id_with_receiver_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    const SHLegacyValue *receiver,
    SHSymbolID symID,
    SHReadPropertyCacheEntry *propCacheEntry) {
  return getByIdWithReceiver_RJS<false>(
      getRuntime(shr),
      Handle<>{toPHV(source)},
      SymbolID::unsafeCreate(symID),
      Handle<>{toPHV(receiver)},
      reinterpret_cast<ReadPropertyCacheEntry *>(propCacheEntry));
}

extern "C" void _sh_ljs_define_own_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  CallResult<bool> cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    cr = JSObject::defineOwnComputed(
        Handle<JSObject>::vmcast(toPHV(target)),
        runtime,
        Handle<>(toPHV(key)),
        DefinePropertyFlags::getDefaultNewPropertyFlags(),
        Handle<>(toPHV(value)),
        PropOpFlags().plusThrowOnError());
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}
extern "C" void _sh_ljs_define_own_ne_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  CallResult<bool> cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    cr = JSObject::defineOwnComputed(
        Handle<JSObject>::vmcast(toPHV(target)),
        runtime,
        Handle<>(toPHV(key)),
        DefinePropertyFlags::getNewNonEnumerableFlags(),
        Handle<>(toPHV(value)),
        PropOpFlags().plusThrowOnError());
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

extern "C" void _sh_ljs_define_own_by_index(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t key,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  CallResult<bool> cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    Handle<> indexHandle{runtime, HermesValue::encodeTrustedNumberValue(key)};
    cr = JSObject::defineOwnComputedPrimitive(
        Handle<JSObject>::vmcast(toPHV(target)),
        runtime,
        indexHandle,
        DefinePropertyFlags::getDefaultNewPropertyFlags(),
        Handle<>(toPHV(value)),
        PropOpFlags().plusThrowOnError());
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

/// Put an enumerable property.
extern "C" void _sh_ljs_define_new_own_by_id(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t key,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  ExecutionStatus cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    cr = JSObject::defineNewOwnProperty(
        Handle<JSObject>::vmcast(toPHV(target)),
        runtime,
        SymbolID::unsafeCreate(key),
        PropertyFlags::defaultNewNamedPropertyFlags(),
        Handle<>(toPHV(value)));
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

/// Put a non-enumerable property.
extern "C" void _sh_ljs_define_new_own_ne_by_id(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t key,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  ExecutionStatus cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    cr = JSObject::defineNewOwnProperty(
        Handle<JSObject>::vmcast(toPHV(target)),
        runtime,
        SymbolID::unsafeCreate(key),
        PropertyFlags::nonEnumerablePropertyFlags(),
        Handle<>(toPHV(value)));
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

/// Put a non-enumerable property.
extern "C" void _sh_ljs_define_own_getter_setter_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *getter,
    SHLegacyValue *setter,
    bool isEnumerable) {
  Runtime &runtime{getRuntime(shr)};

  ExecutionStatus cr = [&runtime, target, key, getter, setter, isEnumerable]() {
    GCScopeMarkerRAII marker{runtime};

    DefinePropertyFlags dpFlags{};
    dpFlags.setConfigurable = 1;
    dpFlags.configurable = 1;
    dpFlags.setEnumerable = 1;
    dpFlags.enumerable = isEnumerable;

    Handle<Callable> getterCallable = Runtime::makeNullHandle<Callable>();
    Handle<Callable> setterCallable = Runtime::makeNullHandle<Callable>();
    if (LLVM_LIKELY(!toPHV(getter)->isUndefined())) {
      dpFlags.setGetter = 1;
      getterCallable = Handle<Callable>::vmcast(toPHV(getter));
    }
    if (LLVM_LIKELY(!toPHV(setter)->isUndefined())) {
      dpFlags.setSetter = 1;
      setterCallable = Handle<Callable>::vmcast(toPHV(setter));
    }
    assert(
        (dpFlags.setSetter || dpFlags.setGetter) &&
        "No accessor set in DefineOwnGetterSetterByVal");

    auto res =
        PropertyAccessor::create(runtime, getterCallable, setterCallable);
    auto accessor = runtime.makeHandle<PropertyAccessor>(std::move(res));

    return JSObject::defineOwnComputed(
               Handle<JSObject>::vmcast(toPHV(target)),
               runtime,
               Handle<>(toPHV(key)),
               dpFlags,
               accessor)
        .getStatus();
  }();

  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

static HermesValue delByVal(
    Runtime &runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *key,
    bool strictMode) {
  const PropOpFlags defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(strictMode);
  CallResult<bool> status{ExecutionStatus::EXCEPTION};
  if (LLVM_LIKELY(target->isObject())) {
    {
      GCScopeMarkerRAII marker{runtime};
      status = JSObject::deleteComputed(
          Handle<JSObject>::vmcast(toPHV(target)),
          runtime,
          Handle<>(toPHV(key)),
          defaultPropOpFlags);
    }
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      _sh_throw_current(getSHRuntime(runtime));
    }
    return HermesValue::encodeBoolValue(status.getValue());
  } else {
    // This is the "slow path".
    CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker{runtime};
      res = toObject(runtime, Handle<>(target));
    }
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      // If an exception is thrown, likely we are trying to convert
      // undefined/null to an object. Passing over the name of the property
      // so that we could emit more meaningful error messages.
      // CAPTURE_IP(amendPropAccessErrorMsgWithPropName(
      //     runtime, Handle<>(&O2REG(DelById)), "delete", ID(idVal)));
      _sh_throw_current(getSHRuntime(runtime));
    }
    {
      GCScopeMarkerRAII marker{runtime};
      Handle tmpHandle{runtime, res.getValue()};
      status = JSObject::deleteComputed(
          Handle<JSObject>::vmcast(tmpHandle),
          runtime,
          Handle<>(toPHV(key)),
          defaultPropOpFlags);
    }
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      _sh_throw_current(getSHRuntime(runtime));
    }
    return HermesValue::encodeBoolValue(status.getValue());
  }
}

extern "C" SHLegacyValue _sh_ljs_del_by_val_strict(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key) {
  return delByVal(getRuntime(shr), toPHV(target), toPHV(key), true);
}
extern "C" SHLegacyValue _sh_ljs_del_by_val_loose(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key) {
  return delByVal(getRuntime(shr), toPHV(target), toPHV(key), false);
}

extern "C" SHLegacyValue _sh_ljs_get_string(SHRuntime *shr, SHSymbolID symID) {
  NoHandleScope noHandles{getRuntime(shr)};
  return HermesValue::encodeStringValue(
      getRuntime(shr).getStringPrimFromSymbolID(SymbolID::unsafeCreate(symID)));
}

extern "C" SHLegacyValue
_sh_ljs_create_regexp(SHRuntime *shr, SHSymbolID pattern, SHSymbolID flags) {
  Runtime &runtime = getRuntime(shr);
  CallResult<HermesValue> cr =
      [&runtime, pattern, flags]() -> CallResult<HermesValue> {
    GCScopeMarkerRAII marker{runtime};
    Handle<JSRegExp> re = runtime.makeHandle(JSRegExp::create(runtime));
    auto patternHandle = runtime.makeHandle(
        runtime.getStringPrimFromSymbolID(SymbolID::unsafeCreate(pattern)));
    auto flagsHandle = runtime.makeHandle(
        runtime.getStringPrimFromSymbolID(SymbolID::unsafeCreate(flags)));
    if (LLVM_UNLIKELY(
            JSRegExp::initialize(re, runtime, patternHandle, flagsHandle) ==
            ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    return re.getHermesValue();
  }();
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *cr;
}

extern "C" SHLegacyValue
_sh_ljs_create_bigint(SHRuntime *shr, const uint8_t *value, uint32_t size) {
  Runtime &runtime = getRuntime(shr);
  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  {
    // This function won't allocate handles because the `fromBytes` function
    // is also used in the interpreter.
    NoHandleScope noHandle{runtime};
    res = BigIntPrimitive::fromBytes(runtime, llvh::ArrayRef{value, size});
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return *res;
}

extern "C" SHLegacyValue _sh_ljs_new_object(SHRuntime *shr) {
  PseudoHandle<JSObject> result;
  {
    NoHandleScope noHandle{getRuntime(shr)};
    result = JSObject::create(getRuntime(shr));
  }
  return result.getHermesValue();
}
extern "C" SHLegacyValue _sh_ljs_new_object_with_parent(
    SHRuntime *shr,
    const SHLegacyValue *parent) {
  PseudoHandle<JSObject> result;
  {
    NoHandleScope noHandle{getRuntime(shr)};
    result = JSObject::create(
        getRuntime(shr),
        toPHV(parent)->isObject() ? Handle<JSObject>::vmcast(toPHV(parent))
            : toPHV(parent)->isNull()
            ? Runtime::makeNullHandle<JSObject>()
            : Handle<JSObject>::vmcast(&getRuntime(shr).objectPrototype));
  }
  return result.getHermesValue();
}

static HiddenClass *addBufferPropertiesToHiddenClass(
    Runtime &runtime,
    SHUnit *unit,
    const SHShapeTableEntry *info,
    HiddenClass *rootClazz) {
  struct : Locals {
    PinnedValue<HiddenClass> clazz;
    PinnedValue<> tmpHandleKey;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  lv.clazz = rootClazz;

  struct {
    void addProperty(SymbolID sym) {
      auto addResult = HiddenClass::addProperty(
          clazz, runtime, sym, PropertyFlags::defaultNewNamedPropertyFlags());
      clazz = addResult->first;
      marker.flush();
    }

    void visitStringID(StringID id) {
      auto sym = SymbolID::unsafeCreate(unit->symbols[id]);
      addProperty(sym);
    }
    void visitNumber(double d) {
      tmpHandleKey = HermesValue::encodeTrustedNumberValue(d);
      // Note that this handle is released in addProperty.
      Handle<SymbolID> symHandle = *valueToSymbolID(runtime, tmpHandleKey);
      addProperty(*symHandle);
    }
    void visitNull() {
      llvm_unreachable("Object literal key cannot be null.");
    }
    void visitUndefined() {
      llvm_unreachable("Object literal key cannot be undefined.");
    }
    void visitBool(bool) {
      llvm_unreachable("Object literal key cannot be a bool.");
    }

    PinnedValue<HiddenClass> &clazz;
    PinnedValue<> &tmpHandleKey;
    Runtime &runtime;
    SHUnit *unit;
    GCScopeMarkerRAII marker;
  } v{lv.clazz, lv.tmpHandleKey, runtime, unit, GCScopeMarkerRAII{runtime}};

  llvh::ArrayRef keyBuffer{unit->obj_key_buffer, unit->obj_key_buffer_size};

  SerializedLiteralParser::parse(
      keyBuffer.slice(info->key_buffer_offset), info->num_props, v);

  return *lv.clazz;
}

extern "C" SHLegacyValue _sh_ljs_new_object_with_buffer(
    SHRuntime *shr,
    SHUnit *unit,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  Runtime &runtime = getRuntime(shr);
  NoLeakHandleScope marker{runtime};

  HiddenClass *clazz;
  auto *cacheEntry = reinterpret_cast<WeakRoot<HiddenClass> *>(
      &unit->object_literal_class_cache[shapeTableIndex]);
  if (*cacheEntry) {
    // There is a already a cached entry for this shape, we can just use that.
    clazz = cacheEntry->getNonNull(runtime, runtime.getHeap());
  } else {
    // There is no cached entry, construct the class from scratch and try to
    // cache it.
    const SHShapeTableEntry *shapeInfo =
        &unit->obj_shape_table[shapeTableIndex];

    if (shapeInfo->num_props > HiddenClass::maxNumProperties()) {
      (void)runtime.raiseRangeError(
          TwineChar16("Object has more than ") +
          HiddenClass::maxNumProperties() + " properties");
      _sh_throw_current(shr);
    }

    clazz = addBufferPropertiesToHiddenClass(
        runtime,
        unit,
        shapeInfo,
        *runtime.getHiddenClassForPrototype(
            *runtime.objectPrototype, JSObject::numOverlapSlots<JSObject>()));
    assert(
        shapeInfo->num_props == clazz->getNumProperties() &&
        "numLiterals should match hidden class property count.");
    // Dictionary mode classes cannot be cached since they can change as the
    // resulting object is modified.
    if (LLVM_LIKELY(!clazz->isDictionary()))
      cacheEntry->set(runtime, clazz);
  }

  struct : Locals {
    PinnedValue<HiddenClass> clazz;
    PinnedValue<JSObject> obj;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  // Create a new object using the built-in constructor or cached hidden class.
  // Note that the built-in constructor is empty, so we don't actually need to
  // call it.
  lv.clazz = clazz;
  auto numProps = lv.clazz->getNumProperties();
  lv.obj = JSObject::create(runtime, lv.clazz);

  struct {
    void visitStringID(StringID id) {
      auto shv = SmallHermesValue::encodeStringValue(
          runtime.getStringPrimFromSymbolID(
              SymbolID::unsafeCreate(unit->symbols[id])),
          runtime);
      JSObject::setNamedSlotValueUnsafe(*obj, runtime, i++, shv);
    }
    void visitNumber(double d) {
      auto shv = SmallHermesValue::encodeNumberValue(d, runtime);
      JSObject::setNamedSlotValueUnsafe(*obj, runtime, i++, shv);
    }
    void visitNull() {
      constexpr auto shv = SmallHermesValue::encodeNullValue();
      JSObject::setNamedSlotValueUnsafe(*obj, runtime, i++, shv);
    }
    void visitUndefined() {
      constexpr auto shv = SmallHermesValue::encodeUndefinedValue();
      JSObject::setNamedSlotValueUnsafe(*obj, runtime, i++, shv);
    }
    void visitBool(bool b) {
      auto shv = SmallHermesValue::encodeBoolValue(b);
      JSObject::setNamedSlotValueUnsafe(*obj, runtime, i++, shv);
    }

    PinnedValue<JSObject> &obj;
    Runtime &runtime;
    SHUnit *unit;
    size_t i;
  } v{lv.obj, runtime, unit, 0};

  llvh::ArrayRef literalValBuffer{
      unit->literal_val_buffer, unit->literal_val_buffer_size};
  SerializedLiteralParser::parse(
      literalValBuffer.slice(valBufferOffset), numProps, v);

  return lv.obj.getHermesValue();
}

extern "C" SHLegacyValue _sh_ljs_new_array(SHRuntime *shr, uint32_t sizeHint) {
  Runtime &runtime = getRuntime(shr);

  CallResult<HermesValue> arrayRes = [&runtime, sizeHint]() {
    GCScopeMarkerRAII marker{runtime};
    return toCallResultHermesValue(
        JSArray::create(runtime, sizeHint, sizeHint));
  }();
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *arrayRes;
}

extern "C" SHLegacyValue _sh_ljs_new_array_with_buffer(
    SHRuntime *shr,
    SHUnit *unit,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t arrayBufferIndex) {
  // Create a new array using the built-in constructor, and initialize
  // the elements from a literal array buffer.
  Runtime &runtime = getRuntime(shr);
  llvh::ArrayRef arrayBuffer{
      unit->literal_val_buffer, unit->literal_val_buffer_size};

  CallResult<HermesValue> arrayRes = [&runtime, numElements]() {
    GCScopeMarkerRAII marker{runtime};
    return toCallResultHermesValue(
        JSArray::create(runtime, numElements, numElements));
  }();
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  HermesValue arr = [&runtime,
                     unit,
                     &arrayRes,
                     numElements,
                     numLiterals,
                     arrayBuffer,
                     arrayBufferIndex]() {
    GCScopeMarkerRAII marker{runtime};
    // Resize the array storage in advance.
    Handle<JSArray> arr = runtime.makeHandle(vmcast<JSArray>(*arrayRes));
    JSArray::setStorageEndIndex(arr, runtime, numElements);

    struct {
      void visitStringID(StringID id) {
        auto shv = SmallHermesValue::encodeStringValue(
            runtime.getStringPrimFromSymbolID(
                SymbolID::unsafeCreate(unit->symbols[id])),
            runtime);
        JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, shv);
      }
      void visitNumber(double d) {
        auto shv = SmallHermesValue::encodeNumberValue(d, runtime);
        JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, shv);
      }
      void visitNull() {
        constexpr auto shv = SmallHermesValue::encodeNullValue();
        JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, shv);
      }
      void visitUndefined() {
        constexpr auto shv = SmallHermesValue::encodeUndefinedValue();
        JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, shv);
      }
      void visitBool(bool b) {
        auto shv = SmallHermesValue::encodeBoolValue(b);
        JSArray::unsafeSetExistingElementAt(*arr, runtime, i++, shv);
      }

      Handle<JSArray> arr;
      Runtime &runtime;
      SHUnit *unit;
      size_t i;
    } v{arr, runtime, unit, 0};

    SerializedLiteralParser::parse(
        arrayBuffer.slice(arrayBufferIndex), numLiterals, v);

    return arr.getHermesValue();
  }();

  return arr;
}

extern "C" void _sh_ljs_cache_new_object(
    SHRuntime *shr,
    SHUnit *unit,
    SHLegacyValue *thisArg,
    SHLegacyValue *newTarget,
    uint32_t shapeTableIndex) {}

extern "C" SHLegacyValue _sh_new_fastarray(SHRuntime *shr, uint32_t sizeHint) {
  Runtime &runtime = getRuntime(shr);

  CallResult<HermesValue> arrayRes = [&runtime, sizeHint]() {
    NoLeakHandleScope noLeakHandles{runtime};
    return FastArray::create(runtime, sizeHint);
  }();
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *arrayRes;
}

extern "C" SHLegacyValue
_sh_ljs_is_in_rjs(SHRuntime *shr, SHLegacyValue *name, SHLegacyValue *obj) {
  Runtime &runtime = getRuntime(shr);
  CallResult<bool> cr{false};
  {
    GCScopeMarkerRAII marker{runtime};
    if (LLVM_UNLIKELY(!_sh_ljs_is_object(*obj))) {
      (void)runtime.raiseTypeError("right operand of 'in' is not an object");
      cr = ExecutionStatus::EXCEPTION;
    } else {
      cr = JSObject::hasComputed(
          Handle<JSObject>::vmcast(toPHV(obj)), runtime, Handle<>(toPHV(name)));
    }
  }
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return _sh_ljs_bool(*cr);
}

extern "C" SHLegacyValue _sh_ljs_get_pname_list_rjs(
    SHRuntime *shr,
    SHLegacyValue *base,
    SHLegacyValue *index,
    SHLegacyValue *size) {
  Runtime &runtime = getRuntime(shr);
  if (toPHV(base)->isUndefined() || toPHV(base)->isNull()) {
    // Set the iterator to be undefined value.
    return HermesValue::encodeUndefinedValue();
  }

  auto cr = [&runtime, base, index, size]() -> CallResult<HermesValue> {
    GCScopeMarkerRAII marker{runtime};
    if (LLVM_UNLIKELY(!vmisa<JSObject>(*toPHV(base)))) {
      Handle<> baseHandle{toPHV(base)};
      // Convert to object and store it back to the register.
      auto res = toObject(runtime, baseHandle);
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      *base = res.getValue();
    }

    struct : public Locals {
      PinnedValue<JSObject> obj;
    } lv;
    LocalsRAII lraii{runtime, &lv};
    lv.obj = vmcast<JSObject>(*toPHV(base));
    uint32_t beginIndex;
    uint32_t endIndex;
    MutableHandle<JSObject> obj{lv.obj};
    auto cr = getForInPropertyNames(runtime, obj, beginIndex, endIndex);
    if (cr == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto arr = *cr;
    *index = HermesValue::encodeTrustedNumberValue(beginIndex);
    *size = HermesValue::encodeTrustedNumberValue(endIndex);
    return arr.getHermesValue();
  }();

  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *cr;
}

extern "C" SHLegacyValue _sh_ljs_get_next_pname_rjs(
    SHRuntime *shr,
    SHLegacyValue *props,
    SHLegacyValue *base,
    SHLegacyValue *indexVal,
    SHLegacyValue *sizeVal) {
  Runtime &runtime = getRuntime(shr);
  assert(
      vmisa<BigStorage>(*toPHV(props)) &&
      "GetNextPName's props must be BigStorage");
  Handle<BigStorage> arr = Handle<BigStorage>::vmcast(toPHV(props));
  Handle obj = Handle<JSObject>::vmcast(toPHV(base));
  auto result =
      [&runtime, arr, obj, indexVal, sizeVal]() -> CallResult<HermesValue> {
    struct : public Locals {
      PinnedValue<> tmp;
      PinnedValue<JSObject> propObj;
      PinnedValue<SymbolID> tmpPropNameStorage;
      PinnedValue<HiddenClass> cachedClass;
    } lv;
    LocalsRAII lraii{runtime, &lv};
    GCScopeMarkerRAII marker{runtime};
    uint32_t idx = toPHV(indexVal)->getNumber();
    uint32_t size = toPHV(sizeVal)->getNumber();
    // If there's a class at index 2, it means we have a cached class.
    uint32_t startIdx = 0;
    uint32_t numObjProps = 0;
    if (LLVM_LIKELY(size > 2)) {
      lv.cachedClass = dyn_vmcast<HiddenClass>(arr->at(runtime, 2));
      if (lv.cachedClass.get()) {
        startIdx = arr->at(runtime, 0).getNumberAs<uint32_t>();
        numObjProps = arr->at(runtime, 1).getNumberAs<uint32_t>();
      }
    }

    MutableHandle<JSObject> propObj{lv.propObj};
    MutableHandle<SymbolID> tmpPropNameStorage{lv.tmpPropNameStorage};
    // Loop until we find a property which is present.
    while (idx < size) {
      lv.tmp = arr->at(runtime, idx);
      // If there's no caching, lv.cachedClass is nullptr and the comparison
      // will fail.
      if (LLVM_LIKELY(size > 0) && idx - startIdx < numObjProps &&
          LLVM_LIKELY(lv.cachedClass.get() == obj->getClass(runtime))) {
        // Cached.
        propObj = obj;
        break;
      }
      if (lv.tmp->isSymbol()) {
        // NOTE: This call is safe because we immediately discard desc,
        // so it can't outlive the SymbolID.
        NamedPropertyDescriptor desc;
        propObj = JSObject::getNamedDescriptorUnsafe(
            obj, runtime, lv.tmp->getSymbol(), desc);
      } else {
        assert(
            (lv.tmp->isNumber() || lv.tmp->isString()) &&
            "GetNextPName must be symbol, string, number");
        ComputedPropertyDescriptor desc;
        ExecutionStatus status = JSObject::getComputedPrimitiveDescriptor(
            obj, runtime, lv.tmp, propObj, tmpPropNameStorage, desc);
        if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      if (LLVM_LIKELY(propObj))
        break;
      ++idx;
    }
    if (idx < size) {
      // We must return the property as a string
      if (lv.tmp->isNumber()) {
        auto strRes = numberToStringPrimitive(runtime, lv.tmp->getNumber());
        if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        lv.tmp = strRes->getHermesValue();
      } else if (lv.tmp->isSymbol()) {
        // for-in enumeration only returns numbers and strings.
        // In most cases (i.e. non-Proxy), we keep the symbol around instead
        // and convert here, so that the above getNamedDescriptor call is
        // faster. Proxy has a filter so that it only returns Strings here. So
        // we don't have to check isUniqued and can convert to string
        // unconditionally.
        assert(
            lv.tmp->getSymbol().isUniqued() &&
            "Symbol primitives (non-uniqued) can't be used in for-in, "
            "not even by Proxy");
        lv.tmp = HermesValue::encodeStringValue(
            runtime.getStringPrimFromSymbolID(lv.tmp->getSymbol()));
      } else {
        assert(
            lv.tmp->isString() &&
            "GetNextPName must be symbol, string, number");
      }
      *indexVal = HermesValue::encodeTrustedNumberValue(idx + 1);
      return lv.tmp.get();
    } else {
      return HermesValue::encodeUndefinedValue();
    }
  }();

  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *result;
}

extern "C" SHLegacyValue _sh_ljs_direct_eval(
    SHRuntime *shr,
    SHLegacyValue *evalText,
    bool strictCaller) {
  if (!_sh_ljs_is_string(*evalText))
    return *evalText;
  auto result = [&runtime = getRuntime(shr),
                 evalText,
                 strictCaller]() -> CallResult<HermesValue> {
    GCScopeMarkerRAII marker{runtime};

    return vm::directEval(
        runtime,
        Handle<StringPrimitive>::vmcast(toPHV(evalText)),
        strictCaller,
        nullptr,
        false);
  }();

  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);

  return *result;
}

extern "C" int32_t _sh_to_int32_double_slow_path(double d) {
  return truncateToInt32SlowPath(d);
}

extern "C" SHLegacyValue
_sh_prload_direct(SHRuntime *shr, SHLegacyValue source, uint32_t propIndex) {
  Runtime &runtime = getRuntime(shr);
  NoAllocScope noAlloc(runtime);
  return JSObject::getNamedSlotValueDirectUnsafe(
             vmcast<JSObject>(*toPHV(&source)), runtime, propIndex)
      .unboxToHV(runtime);
}

extern "C" SHLegacyValue
_sh_prload_indirect(SHRuntime *shr, SHLegacyValue source, uint32_t propIndex) {
  Runtime &runtime = getRuntime(shr);
  NoAllocScope noAlloc(runtime);
  return JSObject::getNamedSlotValueIndirectUnsafe(
             vmcast<JSObject>(*toPHV(&source)), runtime, propIndex)
      .unboxToHV(runtime);
}

extern "C" void _sh_prstore_direct(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  SmallHermesValue shv =
      SmallHermesValue::encodeHermesValue(*toPHV(value), runtime);
  JSObject::setNamedSlotValueDirectUnsafe(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_prstore_direct_bool(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  assert(_sh_ljs_is_bool(*value));
  Runtime &runtime = getRuntime(shr);
  auto shv = SmallHermesValue::encodeBoolValue(_sh_ljs_get_bool(*value));
  JSObject::setNamedSlotValueDirectUnsafe<std::false_type>(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_prstore_direct_number(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  assert(_sh_ljs_is_double(*value));
  Runtime &runtime = getRuntime(shr);
  auto shv =
      SmallHermesValue::encodeNumberValue(_sh_ljs_get_double(*value), runtime);
  JSObject::setNamedSlotValueDirectUnsafe(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_prstore_direct_object(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  assert(_sh_ljs_is_object(*value));
  Runtime &runtime = getRuntime(shr);
  auto shv = SmallHermesValue::encodeObjectValue(
      vmcast<JSObject>(*toPHV(value)), runtime);
  JSObject::setNamedSlotValueDirectUnsafe(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_prstore_direct_string(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  assert(_sh_ljs_is_string(*value));
  Runtime &runtime = getRuntime(shr);
  auto shv = SmallHermesValue::encodeStringValue(
      vmcast<StringPrimitive>(*toPHV(value)), runtime);
  JSObject::setNamedSlotValueDirectUnsafe(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_prstore_indirect(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t propIndex,
    SHLegacyValue *value) {
  Runtime &runtime = getRuntime(shr);
  SmallHermesValue shv =
      SmallHermesValue::encodeHermesValue(*toPHV(value), runtime);
  JSObject::setNamedSlotValueIndirectUnsafe(
      vmcast<JSObject>(*toPHV(target)), runtime, propIndex, shv);
}

extern "C" void _sh_typed_store_parent(
    SHRuntime *shr,
    const SHLegacyValue *storedValue,
    const SHLegacyValue *object) {
  Runtime &runtime = getRuntime(shr);
  Handle<JSObject> objectHandle = Handle<JSObject>::vmcast(toPHV(object));
  Handle<JSObject> parentHandle = Handle<JSObject>::vmcast(toPHV(storedValue));
  JSObject::unsafeSetParentInternal(
      objectHandle.get(), runtime, parentHandle.get());
}

extern "C" void _sh_unreachable() {
  hermes_fatal("Unreachable code reached");
}

extern "C" void _sh_throw_array_oob(SHRuntime *shr) {
  (void)getRuntime(shr).raiseRangeError("array load index out of range");
  _sh_throw_current(shr);
}

extern "C" SHLegacyValue
_sh_fastarray_load(SHRuntime *shr, SHLegacyValue *array, double index) {
  Runtime &runtime = getRuntime(shr);
  auto *arr = vmcast<FastArray>(*toPHV(array));
  ArrayStorageSmall *storage = arr->unsafeGetIndexedStorage(runtime);

  uint32_t intIndex = _sh_tryfast_f64_to_u32_cvt(index);
  // Check that the index is an unsigned integer that is within range.
  if (LLVM_UNLIKELY(intIndex >= storage->size() || intIndex != index))
    _sh_throw_array_oob(shr);

  return storage->at(intIndex).unboxToHV(runtime);
}

extern "C" void _sh_fastarray_store(
    SHRuntime *shr,
    const SHLegacyValue *storedValue,
    SHLegacyValue *array,
    double index) {
  Runtime &runtime = getRuntime(shr);
  auto shv = SmallHermesValue::encodeHermesValue(*toPHV(storedValue), runtime);
  auto *arr = vmcast<FastArray>(*toPHV(array));
  ArrayStorageSmall *storage = arr->unsafeGetIndexedStorage(runtime);

  uint32_t intIndex = _sh_tryfast_f64_to_u32_cvt(index);
  // Check that the index is an unsigned integer that is within range.
  if (LLVM_UNLIKELY(intIndex >= storage->size() || intIndex != index))
    _sh_throw_array_oob(shr);

  return storage->set(intIndex, shv, runtime.getHeap());
}

extern "C" void _sh_fastarray_push(
    SHRuntime *shr,
    SHLegacyValue *pushedValue,
    SHLegacyValue *array) {
  Runtime &runtime = getRuntime(shr);
  auto arr = Handle<FastArray>::vmcast(toPHV(array));
  auto val = Handle<>(toPHV(pushedValue));
  auto res = FastArray::push(arr, runtime, val);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

extern "C" void _sh_fastarray_append(
    SHRuntime *shr,
    SHLegacyValue *other,
    SHLegacyValue *array) {
  Runtime &runtime = getRuntime(shr);
  auto arr = Handle<FastArray>::vmcast(toPHV(array));
  auto source = Handle<FastArray>::vmcast(toPHV(other));
  auto res = FastArray::append(arr, runtime, source);

  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
}

extern "C" SHLegacyValue
_sh_string_concat(SHRuntime *shr, uint32_t argCount, ...) {
  Runtime &runtime = getRuntime(shr);

  // StringPrimitive::concat has special handling for two arguments,
  // so use that fast path when possible.
  if (argCount == 2) {
    va_list args;
    va_start(args, argCount);
    auto lhsHandle = Handle<StringPrimitive>::vmcast(
        toPHV(va_arg(args, const SHLegacyValue *)));
    auto rhsHandle = Handle<StringPrimitive>::vmcast(
        toPHV(va_arg(args, const SHLegacyValue *)));
    va_end(args);
    CallResult<HermesValue> result{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker{runtime};
      result = StringPrimitive::concat(runtime, lhsHandle, rhsHandle);
    }
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      _sh_throw_current(shr);
    }
    return *result;
  }

  // Otherwise, we concatenate all the strings ourselves.

  // Whether the first string is Buffered, and we should use
  // StringPrimitive::concat instead of StringBuilder.
  bool firstIsBuffered = false;
  // Information used for StringBuilder, if it's used instead of
  // StringPrimitive::concat.
  SafeUInt32 resultSize{0};
  bool isASCII = true;

  va_list args;

  // Compute the size, check for non-ASCII strings.
  va_start(args, argCount);
  for (size_t i = 0; i < argCount; ++i) {
    auto argHandle = Handle<StringPrimitive>::vmcast(
        toPHV(va_arg(args, const SHLegacyValue *)));
    if (i == 0 && isBufferedStringPrimitive(*argHandle)) {
      firstIsBuffered = true;
      // Don't need the other information any more.
      break;
    }
    resultSize.add(argHandle->getStringLength());
    if (!argHandle->isASCII()) {
      isASCII = false;
    }
  }
  va_end(args);

  // Perform the concatenation using a StringBuilder.
  va_start(args, argCount);
  CallResult<HermesValue> result = [&]() -> CallResult<HermesValue> {
    GCScopeMarkerRAII marker{runtime};
    if (firstIsBuffered) {
      // The first argument is a BufferedStringPrimitive, so we can use it and
      // concatenate the rest using StringPrimitive::concat.
      // Begin with the first argument in a MutableHandle.
      MutableHandle<StringPrimitive> outputStrHandle{
          runtime,
          vmcast<StringPrimitive>(*toPHV(va_arg(args, const SHLegacyValue *)))};
      // Start with 1 to account for the first arg already being in the handle.
      for (size_t i = 1; i < argCount; ++i) {
        auto argHandle = Handle<StringPrimitive>::vmcast(
            toPHV(va_arg(args, const SHLegacyValue *)));
        // Use concat repeatedly to use a BufferedStringPrimitive for faster
        // concatenation.
        auto res = StringPrimitive::concat(runtime, outputStrHandle, argHandle);
        if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        outputStrHandle = vmcast<StringPrimitive>(*res);
      }
      return outputStrHandle.getHermesValue();
    } else {
      // Otherwise, avoid allocating intermediate strings by using a
      // StringBuilder.
      auto builderRes =
          StringBuilder::createStringBuilder(runtime, resultSize, isASCII);
      if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto builder = std::move(*builderRes);

      for (size_t i = 0; i < argCount; ++i) {
        auto argHandle = Handle<StringPrimitive>::vmcast(
            toPHV(va_arg(args, const SHLegacyValue *)));
        assert(!isASCII || argHandle->isASCII() && "ASCII flag incorrect");
        builder.appendStringPrim(argHandle);
      }
      return builder.getStringPrimitive().getHermesValue();
    }
  }();
  va_end(args);

  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return *result;
}

extern "C" int _sh_errno(void) {
  return errno;
}

extern "C" SHLegacyValue
_sh_asciiz_to_string(SHRuntime *shr, const char *str, ptrdiff_t len) {
  Runtime &runtime = getRuntime(shr);
  NoHandleScope noHandle{runtime};

  auto res = StringPrimitive::createEfficient(
      runtime,
      UTF8Ref((const uint8_t *)str, len < 0 ? strlen(str) : len),
      false);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return *res;
}
