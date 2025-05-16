/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitHandlers.h"

#include "../../JSLib/JSLibInternal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSObject-inline.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StackFrame.h"
#include "hermes/VM/StaticHUtils.h"
#include "hermes/VM/StringPrimitiveValueDenseMapInfo-inline.h"

#define DEBUG_TYPE "jit"

namespace hermes::vm {
SHLegacyValue _sh_ljs_create_bytecode_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID) {
  Runtime &runtime = getRuntime(shr);
  auto *runtimeModule = (RuntimeModule *)shRuntimeModule;
  GCScopeMarkerRAII marker{runtime};
  return JSFunction::createWithInferredParent(
             runtime,
             runtimeModule->getDomain(runtime),
             _sh_ljs_is_undefined(*env)
                 ? Runtime::makeNullHandle<Environment>()
                 : Handle<Environment>::vmcast(toPHV(env)),
             runtimeModule->getCodeBlockMayAllocate(functionID))
      .getHermesValue();
}

SHLegacyValue _interpreter_create_generator(
    SHRuntime *shr,
    SHLegacyValue *frame,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID) {
  Runtime &runtime = getRuntime(shr);
  StackFramePtr framePtr{toPHV(frame)};
  auto *runtimeModule = (RuntimeModule *)shRuntimeModule;
  CallResult<PseudoHandle<JSGeneratorObject>> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = Interpreter::createGenerator_RJS(
        runtime,
        runtimeModule,
        functionID,
        env ? Handle<Environment>::vmcast(toPHV(env))
            : Runtime::makeNullHandle<Environment>(),
        framePtr.getNativeArgs());
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

SHLegacyValue _sh_ljs_get_bytecode_string(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t stringID) {
  return HermesValue::encodeStringValue(
      ((RuntimeModule *)runtimeModule)
          ->getStringPrimFromStringIDMayAllocate(stringID));
}

SHLegacyValue _sh_ljs_get_bytecode_bigint(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t bigintID) {
  Runtime &runtime = getRuntime(shr);
  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    res = BigIntPrimitive::fromBytes(
        runtime,
        ((RuntimeModule *)runtimeModule)->getBigIntBytesFromBigIntId(bigintID));
  }
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return *res;
}

SHLegacyValue _interpreter_create_object_from_buffer(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  Runtime &runtime = getRuntime(shr);
  CallResult<PseudoHandle<>> res = Interpreter::createObjectFromBuffer(
      runtime,
      (CodeBlock *)codeBlock,
      runtime.objectPrototype,
      shapeTableIndex,
      valBufferOffset);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

SHLegacyValue _interpreter_create_object_from_buffer_with_parent(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHLegacyValue *parent,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset) {
  Runtime &runtime = getRuntime(shr);
  auto *parentPHV = toPHV(parent);
  Handle<JSObject> parentHandle = parentPHV->isObject()
      ? Handle<JSObject>::vmcast(parentPHV)
      : parentPHV->isNull()
      ? Runtime::makeNullHandle<JSObject>()
      : Handle<JSObject>::vmcast(&runtime.objectPrototype);
  CallResult<PseudoHandle<>> res = Interpreter::createObjectFromBuffer(
      runtime,
      (CodeBlock *)codeBlock,
      parentHandle,
      shapeTableIndex,
      valBufferOffset);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

/// Wrapper around Interpreter::createArrayFromBuffer.
SHLegacyValue _interpreter_create_array_from_buffer(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    unsigned numElements,
    unsigned numLiterals,
    unsigned bufferIndex) {
  Runtime &runtime = getRuntime(shr);
  CallResult<PseudoHandle<>> res = [&] {
    GCScopeMarkerRAII marker{runtime};
    return Interpreter::createArrayFromBuffer(
        runtime, (CodeBlock *)codeBlock, numElements, numLiterals, bufferIndex);
  }();
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
  return res->getHermesValue();
}

/// Alternative to _sh_ljs_create_regexp that allows using the precompiled
/// regexp bytecode.
SHLegacyValue _interpreter_create_regexp(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHSymbolID patternID,
    SHSymbolID flagsID,
    uint32_t regexpID) {
  Runtime &runtime = getRuntime(shr);
  return Interpreter::createRegExp(
             runtime,
             (CodeBlock *)codeBlock,
             SymbolID::unsafeCreate(patternID),
             SymbolID::unsafeCreate(flagsID),
             regexpID)
      .getHermesValue();
}

void _interpreter_create_class(SHRuntime *shr, SHLegacyValue *frameRegs) {
  Runtime &runtime = getRuntime(shr);
  if (LLVM_UNLIKELY(
          Interpreter::caseCreateClass(
              runtime, (PinnedHermesValue *)frameRegs) ==
          ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
}

/// Implementation of createFunctionEnvironment that takes the closure to get
/// the parentEnvironment from.
/// The native backend doesn't use createFunctionEnvironment.
SHLegacyValue _sh_ljs_create_function_environment(
    SHRuntime *shr,
    SHLegacyValue *frame,
    uint32_t size) {
  Runtime &runtime = getRuntime(shr);

  StackFramePtr framePtr{toPHV(frame)};
  struct : public Locals {
    PinnedValue<Environment> parent;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.parent = framePtr.getCalleeClosureUnsafe()->getEnvironment(runtime);
  return Environment::create(runtime, lv.parent, size);
}

void _sh_print_function_entry_exit(bool enter, const char *msg) {
  static unsigned level = 0;
  if (enter) {
    printf("%*s*** Enter FunctionID ", level * 4, "");
    ++level;
  } else {
    --level;
    printf("%*s*** Leave FunctionID ", level * 4, "");
  }
  printf("%s\n", msg);
  fflush(stdout);
}

SHLegacyValue
_sh_ljs_string_add(SHRuntime *shr, SHLegacyValue *left, SHLegacyValue *right) {
  Runtime &runtime = getRuntime(shr);

  // StringPrimitive::concat has special handling for two arguments,
  auto lhsHandle = Handle<StringPrimitive>::vmcast(toPHV(left));
  auto rhsHandle = Handle<StringPrimitive>::vmcast(toPHV(right));
  CallResult<HermesValue> result{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII marker{runtime};
    result = StringPrimitive::concat(runtime, lhsHandle, rhsHandle);
  }
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
    _sh_throw_current(shr);
  return *result;
}

HermesValue _jit_new_empty_object_for_buffer(
    Runtime &runtime,
    CodeBlock *codeBlock,
    uint32_t shapeTableIndex,
    PinnedHermesValue *tmp) {
  // Get or create the HiddenClass.
  // TODO: Inline the fast path completely into the JIT emitted code once we can
  // also inline the allocation.
  CallResult<HiddenClass *> clazzRes = Interpreter::getHiddenClassForBuffer(
      runtime,
      codeBlock,
      Handle<JSObject>::vmcast(&runtime.objectPrototype),
      shapeTableIndex);
  if (LLVM_UNLIKELY(clazzRes == ExecutionStatus::EXCEPTION))
    _sh_throw_current(&runtime);
  HiddenClass *clazz = *clazzRes;

  // Construct the object.
  *tmp = HermesValue::encodeObjectValue(clazz);
  PseudoHandle<JSObject> result = JSObject::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.objectPrototype),
      Handle<HiddenClass>::vmcast(toPHV(tmp)));
  assert(
      runtime.getHeap().inYoungGen(result.get()) &&
      "New object is not in young gen");

  return result.getHermesValue();
}

void _jit_put_by_id(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHLegacyValue *shBase,
    SHLegacyValue *shValue,
    uint8_t cacheIdx,
    SHSymbolID symID,
    bool strictMode,
    bool tryProp) {
  Runtime &runtime = getRuntime(shr);
  CodeBlock *curCodeBlock = (CodeBlock *)codeBlock;
  Handle<> value{toPHV(shValue)};
  SmallHermesValue shv = SmallHermesValue::encodeHermesValue(*value, runtime);

  if (HermesValue base = *toPHV(shBase); LLVM_LIKELY(base.isObject())) {
    auto *obj = vmcast<JSObject>(base);
    auto *cacheEntry = curCodeBlock->getWriteCacheEntry(cacheIdx);

    CompressedPointer clazzPtr{obj->getClassGCPtr()};
    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
      JSObject::setNamedSlotValueUnsafe(
          obj, runtime, cacheEntry->getSlot(), shv);
      return;
    }

    // Now check against the AddPropertyCacheEntry to ensure we can still
    // use the cached information.
    // NOTE: Need to check resultClazz in all cases because it's a
    // weak reference that may have been freed even if the add cache is
    // valid.
    const auto &addCacheEntry =
        curCodeBlock->getRuntimeModule()->getAddCacheEntry(
            cacheEntry->getAddCacheIndex());
    if (LLVM_LIKELY(addCacheEntry.startClazz == clazzPtr) &&
        LLVM_LIKELY(addCacheEntry.resultClazz) &&
        LLVM_LIKELY(
            addCacheEntry.getParentEpoch() == runtime.getParentCacheEpoch()) &&
        LLVM_LIKELY(addCacheEntry.parent == obj->getParentGCPtr())) {
      HiddenClass *resultClazz =
          addCacheEntry.resultClazz.getNonNull(runtime, runtime.getHeap());
      JSObject::addNewOwnPropertyInSlot(
          obj, runtime, resultClazz, addCacheEntry.getSlot(), shv);
      return;
    }
  }

  ExecutionStatus status;
  {
    GCScopeMarkerRAII marker{runtime};
    status = Interpreter::putByIdSlowPath_RJS(
        runtime,
        curCodeBlock,
        toPHV(shBase),
        toPHV(shValue),
        cacheIdx,
        SymbolID::unsafeCreate(symID),
        strictMode,
        tryProp);
  }
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    _sh_throw_current(shr);
  }
}

#ifdef HERMESVM_PROFILER_BB
void _interpreter_register_bb_execution(SHRuntime *shr, uint16_t pointIndex) {
  Runtime &runtime = getRuntime(shr);
  CodeBlock *codeBlock = runtime.getCurrentFrame().getCalleeCodeBlock();
  runtime.getBasicBlockExecutionInfo().executeBlock(codeBlock, pointIndex);
}
#endif

HermesValue
_jit_direct_eval(Runtime &runtime, PinnedHermesValue *text, bool strictCaller) {
  if (!text->isString()) {
    return *text;
  }

  CallResult<HermesValue> cr{ExecutionStatus::EXCEPTION};
  {
    GCScopeMarkerRAII gcMarker{runtime};
    cr = vm::directEval(
        runtime,
        Handle<StringPrimitive>::vmcast(text),
        strictCaller,
        nullptr,
        false);
  }
  if (cr == ExecutionStatus::EXCEPTION)
    _sh_throw_current(&runtime);

  return *cr;
}

void _sh_throw_invalid_construct(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  (void)runtime.raiseTypeError("Function is not a constructor");
  _sh_throw_current(shr);
}
void _sh_throw_invalid_call(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  (void)runtime.raiseTypeError("Class constructor invoked without new");
  _sh_throw_current(shr);
}
void _sh_throw_register_stack_overflow(SHRuntime *shr) {
  Runtime &runtime = getRuntime(shr);
  (void)runtime.raiseStackOverflow(Runtime::StackOverflowKind::JSRegisterStack);
  _sh_throw_current(shr);
}

void *_jit_find_catch_target(
    SHRuntime *shr,
    SHCodeBlock *shCodeBlock,
    SHLegacyValue *frame,
    SHJmpBuf *jmpBuf,
    SHLocals *savedLocals,
    int32_t *addressTable) {
  Runtime &runtime = getRuntime(shr);
  CodeBlock *codeBlock = (CodeBlock *)shCodeBlock;
  if (isUncatchableError(runtime.getThrownValue())) {
    _sh_end_try(shr, jmpBuf);
    _sh_throw_current(shr);
  }
  // Find the IP. Either the exception was thrown from the current JS frame,
  // in which case it's in the Runtime currentIP slot, or it was thrown by a
  // callee, in which case it's in the register stack's SavedIP slot.
  const inst::Inst *ip;
  if (frame == runtime.getCurrentFrame().ptr()) {
    ip = runtime.getCurrentIP();
  } else {
    // Not the same frame. Load from the SavedIP StackFrameLayout slot.
    // This is valid because the register stack hasn't been reset by
    // _sh_catch_no_pop yet.
    uint32_t nextFrameOffset =
        (codeBlock->getFrameSize() +
         StackFrameLayout::CalleeExtraRegistersAtStart);
    StackFramePtr nextFrame{toPHV(frame) + nextFrameOffset};
    ip = nextFrame.getSavedIP();
  }
  // Look up the offset in the exception table.
  auto offset = codeBlock->getOffsetOf(ip);
  auto excTable =
      codeBlock->getRuntimeModule()->getBytecode()->getExceptionTable(
          codeBlock->getFunctionID());
  for (unsigned i = 0, e = excTable.size(); i < e; ++i) {
    if (excTable[i].start <= offset && offset < excTable[i].end) {
      _sh_catch_no_pop(
          shr,
          savedLocals,
          frame,
          codeBlock->getFrameSize() + hbc::StackFrameLayout::FirstLocal);
      return (void *)((char *)addressTable + addressTable[i]);
    }
  }
  _sh_end_try(shr, jmpBuf);
  _sh_throw_current(shr);
}

SHLegacyValue _jit_dispatch_call(
    SHRuntime *shr,
    SHLegacyValue *callTargetSHLV) {
  Runtime &runtime = getRuntime(shr);

  auto *callTarget = toPHV(callTargetSHLV);
  if (vmisa<JSFunction>(*callTarget)) {
    JSFunction *jsFunc = vmcast<JSFunction>(*callTarget);
    assert(
        !jsFunc->getCodeBlock()->getJITCompiled() &&
        "Calls to JITted code should go directly.");
    CallResult<HermesValue> result = jsFunc->_interpret(runtime);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
      _sh_throw_current(getSHRuntime(runtime));

    return result.getValue();
  }

  if (vmisa<NativeJSFunction>(*callTarget)) {
    return NativeJSFunction::_legacyCall(
        getSHRuntime(runtime), vmcast<NativeJSFunction>(*callTarget));
  }

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

void *_jit_string_switch_imm_table_lookup(
    StringSwitchDenseMap *table,
    SHLegacyValue *switchValueLegacy) {
  PinnedHermesValue *switchValue = toPHV(switchValueLegacy);
  if (!switchValue->isString()) {
    // Not a string; should branch to the default case.
    return nullptr;
  }
  auto iter = table->find(switchValue->getString());
  if (iter == table->end()) {
    // Not found; branch to the default case.
    return nullptr;
  }
  return iter->second.jitCodeTarget;
}

} // namespace hermes::vm

#endif // HERMESVM_JIT
