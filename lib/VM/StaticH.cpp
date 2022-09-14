/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StaticH-internal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/StackFrame-inline.h"

using namespace hermes;
using namespace hermes::vm;

/// The lifetime of a Runtime is managed by a smart pointer, but the C API wants
/// to deal with a regular pointer. Keep all created runtimes here, so they can
/// be destroyed from a pointer.
static llvh::DenseMap<Runtime *, std::shared_ptr<Runtime>> s_runtimes{};

extern "C" SHRuntime *_sh_init(void) {
  auto config = RuntimeConfig::Builder().build();
  std::shared_ptr<Runtime> runtimePtr = Runtime::create(config);
  // Get the pointer first, since order of argument evaluation is not defined.
  Runtime *pRuntime = runtimePtr.get();
  s_runtimes.try_emplace(pRuntime, std::move(runtimePtr));
  return getSHRuntime(*pRuntime);
}

extern "C" void _sh_done(SHRuntime *shr) {
  auto it = s_runtimes.find(&getRuntime(shr));
  if (it == s_runtimes.end()) {
    llvh::errs() << "SHRuntime not found\n";
    abort();
  }
  s_runtimes.erase(it);
}

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
  runtime.setCurrentFrame(StackFramePtr(frame));

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
  return frame;
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

extern "C" void _sh_push_try(SHRuntime *shr, SHJmpBuf *buf) {
  Runtime &runtime = getRuntime(shr);
  buf->prev = runtime.shCurJmpBuf;
  runtime.shCurJmpBuf = buf;
}

extern "C" void _sh_end_try(SHRuntime *shr, SHJmpBuf *prev) {
  Runtime &runtime = getRuntime(shr);
  runtime.shCurJmpBuf = prev;
}

extern "C" SHLegacyValue _sh_catch(
    SHRuntime *shr,
    SHLocals *locals,
    SHLegacyValue *frame,
    uint32_t stackSize) {
  Runtime &runtime = getRuntime(shr);
  runtime.shCurJmpBuf = runtime.shCurJmpBuf->prev;

  runtime.shLocals = locals;
  runtime.popToSavedStackPointer(toPHV(frame + stackSize));
  runtime.setCurrentFrame(StackFramePtr(toPHV(frame)));

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
  _longjmp(runtime.shCurJmpBuf->buf, 1);
}

extern "C" void _sh_throw(SHRuntime *shr, SHLegacyValue value) {
  Runtime &runtime = getRuntime(shr);
  runtime.setThrownValue(HermesValue::fromRaw(value.raw));
  _sh_throw_current(shr);
}

static SHLegacyValue doCall(Runtime &runtime, PinnedHermesValue *callTarget) {
  if (vmisa<SHLegacyFunction>(*callTarget)) {
    return SHLegacyFunction::_legacyCall(
        getSHRuntime(runtime), vmcast<SHLegacyFunction>(*callTarget));
  }

  // FIXME: check for register stack overflow.
  CallResult<PseudoHandle<>> res{ExecutionStatus::EXCEPTION};
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
  newFrame.getArgCountRef() = HermesValue::encodeNativeUInt32(argCount);
  newFrame.getNewTargetRef() = HermesValue::encodeUndefinedValue();
  return doCall(runtime, &newFrame.getCalleeClosureOrCBRef());
}

extern "C" SHLegacyValue
_sh_ljs_construct(SHRuntime *shr, SHLegacyValue *frame, uint32_t argCount) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr newFrame(runtime.getStackPointer());
  newFrame.getPreviousFrameRef() = HermesValue::encodeNativePointer(frame);
  newFrame.getSavedIPRef() = HermesValue::encodeNativePointer(nullptr);
  newFrame.getSavedCodeBlockRef() = HermesValue::encodeNativePointer(nullptr);
  newFrame.getArgCountRef() = HermesValue::encodeNativeUInt32(argCount);
  // Must be initialized by the caller:
  // newFrame.getNewTargetRef() = HermesValue::encodeUndefinedValue();
  return doCall(runtime, &newFrame.getCalleeClosureOrCBRef());
}

extern "C" void _sh_ljs_create_environment(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *result,
    uint32_t size) {
  Runtime &runtime = getRuntime(shr);
  auto framePtr = StackFramePtr(toPHV(frame));

  auto *parentEnv = framePtr.getCalleeClosureUnsafe()->getEnvironment(runtime);
  // We are not allowed to store null object pointers in locals, so we convert
  // a null pointer to JS Null.
  result->raw = parentEnv
      ? HermesValue::encodeObjectValueUnsafe(parentEnv).getRaw()
      : HermesValue::encodeNullValue().getRaw();

  CallResult<HermesValue> res{HermesValue::encodeUndefinedValue()};
  {
    GCScopeMarkerRAII marker{runtime};
    res = Environment::create(
        runtime,
        _sh_ljs_is_null(*result) ? runtime.makeNullHandle<Environment>()
                                 : Handle<Environment>::vmcast(toPHV(result)),
        size);
  }
  if (res == ExecutionStatus::EXCEPTION) {
    _sh_throw_current(shr);
  }
  //#ifdef HERMES_ENABLE_DEBUGGER
  //  framePtr.getDebugEnvironmentRef() = *res;
  //#endif
  result->raw = res->getRaw();
}

extern "C" SHLegacyValue _sh_ljs_create_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHLegacyValue (*func)(SHRuntime *),
    SHSymbolID name,
    uint32_t paramCount) {
  Runtime &runtime = getRuntime(shr);
  GCScopeMarkerRAII marker{runtime};

  // TODO: make this lazy!
  // Create empty object for prototype.
  auto prototypeParent = /*vmisa<JSGeneratorFunction>(*jsFun)
                         ? Handle<JSObject>::vmcast(&runtime.generatorPrototype)
                         :*/
      Handle<JSObject>::vmcast(&runtime.objectPrototype);

  // According to ES12 26.7.4, AsyncFunction instances do not have a
  // 'prototype' property, hence we need to set an null handle here.
  auto prototypeObjectHandle =
      /*vmisa<JSAsyncFunction>(*jsFun)
      ? Runtime::makeNullHandle<JSObject>()
      :*/
      runtime.makeHandle(JSObject::create(runtime, prototypeParent));

  SHLegacyValue res =
      SHLegacyFunction::create(
          runtime,
          Handle<JSObject>::vmcast(&runtime.functionPrototype),
          _sh_ljs_is_null(*env) ? runtime.makeNullHandle<Environment>()
                                : Handle<Environment>::vmcast(toPHV(env)),
          func,
          SymbolID::unsafeCreate(name),
          paramCount,
          prototypeObjectHandle,
          0)
          .getHermesValue();
  return res;
}

extern "C" SHLegacyValue _sh_ljs_get_global_object(SHRuntime *shr) {
  return getRuntime(shr).global_;
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
