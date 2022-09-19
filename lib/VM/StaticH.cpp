/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Callable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StaticHUtils.h"

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

extern "C" SHLegacyValue _sh_ljs_create_this(
    SHRuntime *shr,
    SHLegacyValue *prototype,
    SHLegacyValue *callable) {
  Handle<> callableHandle{toPHV(callable)};
  Runtime &runtime = getRuntime(shr);
  if (LLVM_UNLIKELY(!vmisa<Callable>(*callableHandle))) {
    (void)runtime.raiseTypeError("constructor is not callable");
    _sh_throw_current(shr);
  }
  CallResult<PseudoHandle<JSObject>> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = Callable::newObject(
        Handle<Callable>::vmcast(callableHandle),
        runtime,
        Handle<JSObject>::vmcast(
            toPHV(prototype)->isObject() ? toPHV(prototype)
                                         : &runtime.objectPrototype));
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return res->getHermesValue();
}

extern "C" SHLegacyValue _sh_ljs_load_this_ns(
    SHRuntime *shr,
    SHLegacyValue *frame) {
  StackFramePtr framePtr(toPHV(frame));
  if (LLVM_LIKELY(framePtr.getThisArgRef().isObject())) {
    return framePtr.getThisArgRef();
  } else if (
      framePtr.getThisArgRef().isNull() ||
      framePtr.getThisArgRef().isUndefined()) {
    return getRuntime(shr).global_;
  } else {
    CallResult<HermesValue> res{HermesValue::encodeUndefinedValue()};
    {
      GCScopeMarkerRAII marker{getRuntime(shr)};
      res = toObject(getRuntime(shr), Handle<>(&framePtr.getThisArgRef()));
    }
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      _sh_throw_current(shr);
    return *res;
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

extern "C" SHLegacyValue
_sh_ljs_get_env(SHRuntime *shr, SHLegacyValue *frame, uint32_t level) {
  Runtime &runtime = getRuntime(shr);
  Environment *curEnv = StackFramePtr(toPHV(frame))
                            .getCalleeClosureUnsafe()
                            ->getEnvironment(runtime);
  while (level--) {
    assert(curEnv && "invalid environment relative level");
    curEnv = curEnv->getParentEnvironment(runtime);
  }

  return HermesValue::encodeObjectValue(curEnv);
}

extern "C" SHLegacyValue _sh_ljs_load_from_env(
    SHLegacyValue env,
    uint32_t index) {
  return vmcast<Environment>(HermesValue::fromRaw(env.raw))->slot(index);
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
    PropertyCacheEntry *cacheEntry) {
  //++NumPutById;
  if (LLVM_LIKELY(target->isObject())) {
    SmallHermesValue shv = SmallHermesValue::encodeHermesValue(*value, runtime);
    auto *obj = vmcast<JSObject>(*target);

#ifdef HERMESVM_PROFILER_BB
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
    if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
      //++NumPutByIdCacheHits;
      JSObject::setNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
          obj, runtime, cacheEntry->slot, shv);
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
      if (LLVM_LIKELY(!clazz->isDictionary())
          /* && LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)*/) {
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
    const SHLegacyValue *target,
    SHSymbolID symID,
    const SHLegacyValue *value,
    char *propCacheEntry) {
  putById_RJS<false, false>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<PropertyCacheEntry *>(propCacheEntry));
}

extern "C" void _sh_ljs_put_by_id_strict_rjs(
    SHRuntime *shr,
    const SHLegacyValue *target,
    SHSymbolID symID,
    const SHLegacyValue *value,
    char *propCacheEntry) {
  putById_RJS<false, true>(
      getRuntime(shr),
      toPHV(target),
      SymbolID::unsafeCreate(symID),
      toPHV(value),
      reinterpret_cast<PropertyCacheEntry *>(propCacheEntry));
}

template <bool tryProp>
static inline HermesValue getById_RJS(
    Runtime &runtime,
    const PinnedHermesValue *source,
    SymbolID symID,
    PropertyCacheEntry *cacheEntry) {
  //++NumGetById;
  // NOTE: it is safe to use OnREG(GetById) here because all instructions
  // have the same layout: opcode, registers, non-register operands, i.e.
  // they only differ in the width of the last "identifier" field.
  if (LLVM_LIKELY(source->isObject())) {
    auto *obj = vmcast<JSObject>(*source);

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
      *obj = vmcast<JSObject>(*source);
    }
    gcScope.flushToSmallCount(KEEP_HANDLES);
#endif
    CompressedPointer clazzPtr{obj->getClassGCPtr()};
#ifndef NDEBUG
    // if (vmcast<HiddenClass>(clazzPtr.getNonNull(runtime))->isDictionary())
    //   ++NumGetByIdDict;
#else
    //(void)NumGetByIdDict;
#endif

    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
      //++NumGetByIdCacheHits;
      return JSObject::getNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
                 obj, runtime, cacheEntry->slot)
          .unboxToHV(runtime);
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
      if (LLVM_LIKELY(!clazz->isDictionaryNoCache())
          /*&& LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)*/) {
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

    // The cache may also be populated via the prototype of the object.
    // This value is only reliable if the fast path was a definite
    // not-found.
    if (fastPathResult.hasValue() && !fastPathResult.getValue() &&
        LLVM_LIKELY(!obj->isProxyObject())) {
      JSObject *parent = obj->getParent(runtime);
      // TODO: This isLazy check is because a lazy object is reported as
      // having no properties and therefore cannot contain the property.
      // This check does not belong here, it should be merged into
      // tryGetOwnNamedDescriptorFast().
      if (parent && cacheEntry->clazz == parent->getClassGCPtr() &&
          LLVM_LIKELY(!obj->isLazy())) {
        //++NumGetByIdProtoHits;
        // We've already checked that this isn't a Proxy.
        return JSObject::getNamedSlotValueUnsafe(
                   parent, runtime, cacheEntry->slot)
            .unboxToHV(runtime);
      }
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
    // auto *savedClass = true /*cacheIdx != hbc::PROPERTY_CACHING_DISABLED*/
    //     ? cacheEntry->clazz.get(runtime, runtime.getHeap())
    //     : nullptr;
#endif
    //++NumGetByIdSlow;
    CallResult<PseudoHandle<>> resPH{ExecutionStatus::EXCEPTION};
    {
      GCScopeMarkerRAII marker(runtime);
      const PropOpFlags defaultPropOpFlags = DEFAULT_PROP_OP_FLAGS(false);
      resPH = JSObject::getNamed_RJS(
          Handle<JSObject>::vmcast(source),
          runtime,
          symID,
          !tryProp ? defaultPropOpFlags : defaultPropOpFlags.plusMustExist(),
          true /*cacheIdx != hbc::PROPERTY_CACHING_DISABLED*/ ? cacheEntry
                                                              : nullptr);
    }
    if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));
#ifdef HERMES_SLOW_DEBUG
      // if (cacheIdx != hbc::PROPERTY_CACHING_DISABLED && savedClass &&
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
      resPH =
          Interpreter::getByIdTransient_RJS(runtime, Handle<>(source), symID);
    }
    if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));
    return resPH->get();
  }
}

extern "C" SHLegacyValue _sh_ljs_try_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    char *propCacheEntry) {
  return getById_RJS<true>(
      getRuntime(shr),
      toPHV(source),
      SymbolID::unsafeCreate(symID),
      reinterpret_cast<PropertyCacheEntry *>(propCacheEntry));
}
extern "C" SHLegacyValue _sh_ljs_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    char *propCacheEntry) {
  return getById_RJS<false>(
      getRuntime(shr),
      toPHV(source),
      SymbolID::unsafeCreate(symID),
      reinterpret_cast<PropertyCacheEntry *>(propCacheEntry));
}
