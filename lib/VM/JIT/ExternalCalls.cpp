/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ExternalCalls.h"

#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/RuntimeModule-inline.h"

namespace hermes {
namespace vm {

CallResult<HermesValue> slowPathToNumber(
    Runtime *runtime,
    PinnedHermesValue *src) {
  GCScopeMarkerRAII marker{runtime};
  return toNumber_RJS(runtime, Handle<>(src));
}

CallResult<HermesValue> externToInt32(
    Runtime *runtime,
    PinnedHermesValue *val) {
  GCScopeMarkerRAII marker{runtime};
  return toInt32_RJS(runtime, Handle<>(val));
}

CallResult<HermesValue> slowPathAddEmptyString(
    Runtime *runtime,
    PinnedHermesValue *src) {
  GCScopeMarkerRAII marker{runtime};
  auto prim = toPrimitive_RJS(runtime, Handle<>(src), PreferredType::NONE);
  if (LLVM_UNLIKELY(prim == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  Handle<> tmpHandle(runtime, prim.getValue());
  auto str = toString_RJS(runtime, tmpHandle);
  if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return str->getHermesValue();
}

ExecutionStatus externCallDeclareGlobalVar(
    Runtime *runtime,
    uint32_t stringID) {
  GCScopeMarkerRAII marker{runtime};
  DefinePropertyFlags dpf{};
  dpf.setWritable = 1;
  dpf.setConfigurable = 1;
  dpf.setEnumerable = 1;
  dpf.writable = 1;
  dpf.enumerable = 1;
  dpf.configurable = 0;

  return JSObject::defineOwnProperty(
             runtime->getGlobal(),
             runtime,
             runtime->getCurrentFrame()
                 ->getCalleeCodeBlock()
                 ->getRuntimeModule()
                 ->getSymbolIDMustExist(stringID),
             dpf,
             Runtime::getUndefinedValue(),
             PropOpFlags().plusThrowOnError())
      .getStatus();
}

CallResult<HermesValue> externCreateEnvironment(
    Runtime *runtime,
    PinnedHermesValue *currentFrame,
    uint32_t envSize) {
  GCScopeMarkerRAII marker{runtime};
  StackFramePtr frame(currentFrame);
  frame.getScratchRef() = HermesValue::encodeObjectValue(
      frame->getCalleeClosureUnsafe()->getEnvironment(runtime));

  return Environment::create(
      runtime,
      frame.getScratchRef().getPointer()
          ? Handle<Environment>::vmcast(&frame.getScratchRef())
          : Runtime::makeNullHandle<Environment>(),
      envSize);
}

CallResult<HermesValue> externCreateClosure(
    Runtime *runtime,
    CodeBlock *codeBlock,
    PinnedHermesValue *env) {
  GCScopeMarkerRAII marker{runtime};

  return JSFunction::create(
             runtime,
             codeBlock->getRuntimeModule()->getDomain(runtime),
             Handle<JSObject>::vmcast(&runtime->functionPrototype),
             Handle<Environment>::vmcast(env),
             codeBlock)
      .getHermesValue();
}

ExecutionStatus externPutById(
    Runtime *runtime,
    PropOpFlags opFlags,
    uint32_t sid,
    PinnedHermesValue *target,
    PinnedHermesValue *prop,
    uint8_t cacheIdx) {
  GCScopeMarkerRAII marker{runtime};

  auto *codeBlock = runtime->getCurrentFrame()->getCalleeCodeBlock();

  if (LLVM_LIKELY(target->isObject())) {
    auto *obj = vmcast<JSObject>(*target);
    auto clazzGCPtr = obj->getClassGCPtr();
    auto *cacheEntry = codeBlock->getWriteCacheEntry(cacheIdx);

    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry->clazz == clazzGCPtr.getStorageType())) {
      JSObject::setNamedSlotValueInternal<PropStorage::Inline::Yes>(
          obj, runtime, cacheEntry->slot, *prop);
      return ExecutionStatus::RETURNED;
    }
    auto *clazz = clazzGCPtr.getNonNull(runtime);
    auto id = SymbolID::unsafeCreate(sid);
    NamedPropertyDescriptor desc;
    if (LLVM_LIKELY(
            JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc)) &&
        !desc.flags.accessor && desc.flags.writable &&
        !desc.flags.internalSetter) {
      // cacheIdx == 0 indicates no caching so don't update the cache in
      // those cases.
      if (LLVM_LIKELY(!clazz->isDictionary()) &&
          LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
        // Cache the class and property slot.
        cacheEntry->clazz = clazzGCPtr.getStorageType();
        cacheEntry->slot = desc.slot;
      }

      return JSObject::setNamedSlotValue(
                 createPseudoHandle<JSObject>(obj),
                 runtime,
                 desc,
                 createPseudoHandle(*prop))
          .getStatus();
    }

    return JSObject::putNamed_RJS(
               Handle<JSObject>::vmcast(target),
               runtime,
               id,
               Handle<>(prop),
               opFlags)
        .getStatus();
  } else {
    return Interpreter::putByIdTransient_RJS(
        runtime,
        Handle<>(target),
        SymbolID::unsafeCreate(sid),
        Handle<>(prop),
        codeBlock->isStrictMode());
  }
}

CallResult<HermesValue> externGetById(
    Runtime *runtime,
    PropOpFlags opFlags,
    uint32_t sid,
    PinnedHermesValue *target,
    uint8_t cacheIdx,
    CodeBlock *codeBlock) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_LIKELY(target->isObject())) {
    auto *obj = vmcast<JSObject>(*target);
    auto clazzGCPtr = obj->getClassGCPtr();
    auto *cacheEntry = codeBlock->getReadCacheEntry(cacheIdx);

    // If we have a cache hit, reuse the cached offset and immediately
    // return the property.
    if (LLVM_LIKELY(cacheEntry->clazz == clazzGCPtr.getStorageType())) {
      return JSObject::getNamedSlotValueInternal<PropStorage::Inline::Yes>(
          obj, runtime, cacheEntry->slot);
    }
    auto id = SymbolID::unsafeCreate(sid);
    NamedPropertyDescriptor desc;
    OptValue<bool> fastPathResult =
        JSObject::tryGetOwnNamedDescriptorFast(obj, runtime, id, desc);
    auto *clazz = clazzGCPtr.getNonNull(runtime);
    if (LLVM_LIKELY(fastPathResult.hasValue() && fastPathResult.getValue()) &&
        !desc.flags.accessor) {
      // cacheIdx == 0 indicates no caching so don't update the cache in
      // those cases.
      if (LLVM_LIKELY(!clazz->isDictionary()) &&
          LLVM_LIKELY(cacheIdx != hbc::PROPERTY_CACHING_DISABLED)) {
        // Cache the class, id and property slot.
        cacheEntry->clazz = clazzGCPtr.getStorageType();
        cacheEntry->slot = desc.slot;
      }

      return JSObject::getNamedSlotValue(
                 createPseudoHandle<JSObject>(obj), runtime, desc)
          .toCallResultHermesValue();
    }

    // The cache may also be populated via the prototype of the object.
    // This value is only reliable if the fast path was a definite
    // not-found.
    if (fastPathResult.hasValue() && !fastPathResult.getValue() &&
        !obj->isProxyObject()) {
      JSObject *parent = obj->getParent(runtime);
      // TODO: This isLazy check is because a lazy object is reported as
      // having no properties and therefore cannot contain the property.
      // This check does not belong here, it should be merged into
      // tryGetOwnNamedDescriptorFast().
      if (parent &&
          cacheEntry->clazz == parent->getClassGCPtr().getStorageType() &&
          LLVM_LIKELY(!obj->isLazy())) {
        return JSObject::getNamedSlotValueInternal(
            parent, runtime, cacheEntry->slot);
      }
    }

    return JSObject::getNamed_RJS(
               Handle<JSObject>::vmcast(target), runtime, id, opFlags)
        .toCallResultHermesValue();
  } else {
    /* Slow path. */
    return Interpreter::getByIdTransient_RJS(
               runtime, Handle<>(target), SymbolID::unsafeCreate(sid))
        .toCallResultHermesValue();
  }
}

CallResult<HermesValue> externCall(
    Runtime *runtime,
    PinnedHermesValue *callable,
    uint32_t argCount,
    PinnedHermesValue *stackPointer,
    const Inst *ip,
    PinnedHermesValue *previousFrame) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_UNLIKELY(!dyn_vmcast<Callable>(*callable)))
    return runtime->raiseTypeErrorForValue(
        Handle<>(callable), " is not a function");

  StackFramePtr frame(previousFrame);
  (void)StackFramePtr::initFrame(
      stackPointer,
      frame,
      ip,
      // If the call goes to the interpret loop, it gives Ret a signal to
      // exit that the savedCodeBlock is nullptr.
      // It's temporarily breaking the stack trace functionality because
      // otherwise it confuses the interpreter about when to exit.
      nullptr, /* SavedCodeBlock */
      argCount - 1,
      *callable,
      HermesValue::encodeUndefinedValue());
  runtime->setCurrentIP(ip);
  auto res = Callable::call(Handle<Callable>::vmcast(callable), runtime);
  return res.toCallResultHermesValue();
}

CallResult<HermesValue> externConstruct(
    Runtime *runtime,
    PinnedHermesValue *callable,
    uint32_t argCount,
    PinnedHermesValue *stackPointer,
    const Inst *ip,
    PinnedHermesValue *previousFrame) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_UNLIKELY(!dyn_vmcast<Callable>(*callable)))
    return runtime->raiseTypeErrorForValue(
        Handle<>(callable), " is not a function");

  StackFramePtr frame(previousFrame);
  (void)StackFramePtr::initFrame(
      stackPointer,
      frame,
      ip,
      // If the call goes to the interpret loop, it gives Ret a signal to
      // exit that the savedCodeBlock is nullptr.
      // It's temporarily breaking the stack trace functionality because
      // otherwise it confuses the interpreter about when to exit.
      nullptr, /* SavedCodeBlock */
      argCount - 1,
      *callable,
      *callable);
  runtime->setCurrentIP(ip);
  auto res = Callable::call(Handle<Callable>::vmcast(callable), runtime);
  return res.toCallResultHermesValue();
}

/// Implement a slow path call for a binary operator.
/// \param name the name of the slow path call
/// \param oper the binary operator to use against numbers.
#define IMPLEMENT_BIN_OP(name, oper)                                      \
  CallResult<HermesValue> slowPath##name(                                 \
      Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) { \
    GCScopeMarkerRAII marker{runtime};                                    \
                                                                          \
    CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};              \
    if ((res = toNumber_RJS(runtime, Handle<>(op1))) ==                   \
        ExecutionStatus::EXCEPTION)                                       \
      return ExecutionStatus::EXCEPTION;                                  \
    double left = res->getDouble();                                       \
                                                                          \
    if ((res = toNumber_RJS(runtime, Handle<>(op2))) ==                   \
        ExecutionStatus::EXCEPTION)                                       \
      return ExecutionStatus::EXCEPTION;                                  \
                                                                          \
    return HermesValue::encodeDoubleValue(left oper res->getDouble());    \
  }

IMPLEMENT_BIN_OP(Sub, -);
IMPLEMENT_BIN_OP(Mul, *);
IMPLEMENT_BIN_OP(Div, /);

HermesValue externLoadConstStringMayAllocate(
    uint32_t stringID,
    RuntimeModule *runtimeModule) {
  return HermesValue::encodeStringValue(
      runtimeModule->getStringPrimFromStringIDMayAllocate(stringID));
}

HermesValue externTypeOf(Runtime *runtime, PinnedHermesValue *src) {
  switch (src->getTag()) {
    case UndefinedNullTag:
      return HermesValue::encodeStringValue(
          src->isUndefined()
              ? runtime->getPredefinedString(Predefined::undefined)
              : runtime->getPredefinedString(Predefined::object));
    case StrTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::string));
    case BoolTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::boolean));
    case SymbolTag:
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::symbol));
    case ObjectTag:
      if (vmisa<Callable>(*src)) {
        return HermesValue::encodeStringValue(
            runtime->getPredefinedString(Predefined::function));
      } else {
        return HermesValue::encodeStringValue(
            runtime->getPredefinedString(Predefined::object));
      }
    default:
      assert(src->isNumber() && "Invalid type.");
      return HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::number));
  }
}

#define PROXY_EXTERN_CMP(slowPathName, OpName)                            \
  CallResult<HermesValue> slowPathName(                                   \
      Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) { \
    GCScopeMarkerRAII marker{runtime};                                    \
    auto res = OpName(runtime, Handle<>(op1), Handle<>(op2));             \
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {               \
      return ExecutionStatus::EXCEPTION;                                  \
    }                                                                     \
    return HermesValue::encodeBoolValue(*res);                            \
  }

#define PROXY_EXTERN_BIN_OP(slowPathName, OpName)                         \
  CallResult<HermesValue> slowPathName(                                   \
      Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) { \
    GCScopeMarkerRAII marker{runtime};                                    \
    return OpName(runtime, Handle<>(op1), Handle<>(op2));                 \
  }

PROXY_EXTERN_CMP(slowPathLess, lessOp_RJS);
PROXY_EXTERN_CMP(slowPathGreater, greaterOp_RJS);
PROXY_EXTERN_CMP(slowPathLessEq, lessEqualOp_RJS);
PROXY_EXTERN_CMP(slowPathGreaterEq, greaterEqualOp_RJS);
PROXY_EXTERN_BIN_OP(slowPathAdd, addOp_RJS);
PROXY_EXTERN_BIN_OP(externAbstractEqualityTest, abstractEqualityTest_RJS);

HermesValue externNewObject(Runtime *runtime) {
  GCScopeMarkerRAII marker{runtime};
  return JSObject::create(runtime).getHermesValue();
}

CallResult<HermesValue> externCreateThis(
    Runtime *runtime,
    PinnedHermesValue *proto,
    PinnedHermesValue *closure) {
  GCScopeMarkerRAII marker{runtime};
  if (LLVM_UNLIKELY(!vmisa<Callable>(*closure))) {
    return runtime->raiseTypeError("constructor is not callable");
  }
  auto res = Callable::newObject(
      Handle<Callable>::vmcast(closure),
      runtime,
      Handle<JSObject>::vmcast(
          (*proto).isObject() ? proto : &runtime->objectPrototype));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return res->getHermesValue();
}

CallResult<HermesValue> externNewArray(Runtime *runtime, uint32_t size) {
  GCScopeMarkerRAII marker{runtime};
  auto res = JSArray::create(runtime, size, size);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return res->getHermesValue();
}

ExecutionStatus externPutOwnByIndex(
    Runtime *runtime,
    PinnedHermesValue *obj,
    PinnedHermesValue *prop,
    uint32_t idVal) {
  GCScopeMarkerRAII marker{runtime};

  PinnedHermesValue &scratch = runtime->getCurrentFrame().getScratchRef();
  scratch = HermesValue::encodeDoubleValue(idVal);

  return JSObject::defineOwnComputedPrimitive(
             Handle<JSObject>::vmcast(obj),
             runtime,
             Handle<>(&scratch),
             DefinePropertyFlags::getDefaultNewPropertyFlags(),
             Handle<>(prop))
      .getStatus(); // We don't need the bool value it returns
}

ExecutionStatus externPutNewOwnById(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *prop,
    uint32_t sid) {
  GCScopeMarkerRAII marker{runtime};
  if (LLVM_LIKELY((*target).isObject())) {
    if (LLVM_UNLIKELY(
            JSObject::defineNewOwnProperty(
                Handle<JSObject>::vmcast(target),
                runtime,
                SymbolID::unsafeCreate(sid),
                PropertyFlags::defaultNewNamedPropertyFlags(),
                Handle<>(prop)) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    };
  } else {
    /* This is the "slow path". */
    auto res = toObject(runtime, Handle<>(target));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;

    PinnedHermesValue &scratch = runtime->getCurrentFrame().getScratchRef();
    scratch = res.getValue();
    if (LLVM_UNLIKELY(
            JSObject::defineNewOwnProperty(
                Handle<JSObject>::vmcast(&scratch),
                runtime,
                SymbolID::unsafeCreate(sid),
                PropertyFlags::defaultNewNamedPropertyFlags(),
                Handle<>(prop)) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> slowPathCoerceThis(
    Runtime *runtime,
    PinnedHermesValue *thisVal) {
  GCScopeMarkerRAII marker{runtime};
  auto res = toObject(runtime, Handle<>(thisVal));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return *res;
}

CallResult<HermesValue> externGetByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_LIKELY(target->isObject())) {
    return JSObject::getComputed_RJS(
               Handle<JSObject>::vmcast(target), runtime, Handle<>(nameVal))
        .toCallResultHermesValue();
  } else {
    // This is the "slow path".
    return Interpreter::getByValTransient_RJS(
               runtime, Handle<>(target), Handle<>(nameVal))
        .toCallResultHermesValue();
  }
}

ExecutionStatus externPutByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal,
    PinnedHermesValue *value,
    PropOpFlags flags) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_LIKELY(target->isObject())) {
    return JSObject::putComputed_RJS(
               Handle<JSObject>::vmcast(target),
               runtime,
               Handle<>(nameVal),
               Handle<>(value),
               flags)
        .getStatus();
  } else {
    // This is the "slow path".
    return Interpreter::putByValTransient_RJS(
        runtime,
        Handle<>(target),
        Handle<>(nameVal),
        Handle<>(value),
        /*strictMode*/ flags.getThrowOnError());
  }
}

CallResult<HermesValue> externDelByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal,
    PropOpFlags flags) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_LIKELY(target->isObject())) {
    auto res = JSObject::deleteComputed(
        Handle<JSObject>::vmcast(target), runtime, Handle<>(nameVal), flags);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeBoolValue(*res);
  } else {
    // This is the "slow path".
    auto res = toObject(runtime, Handle<>(target));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    PinnedHermesValue &scratch = runtime->getCurrentFrame().getScratchRef();
    scratch = res.getValue();
    auto delRes = JSObject::deleteComputed(
        Handle<JSObject>::vmcast(&scratch), runtime, Handle<>(nameVal), flags);
    scratch = HermesValue::encodeUndefinedValue();
    if (LLVM_UNLIKELY(delRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeBoolValue(*delRes);
  }
}

CallResult<HermesValue> externDelById(
    Runtime *runtime,
    PinnedHermesValue *target,
    uint32_t sid,
    PropOpFlags flags) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_LIKELY(target->isObject())) {
    auto status = JSObject::deleteNamed(
        Handle<JSObject>::vmcast(target),
        runtime,
        SymbolID::unsafeCreate(sid),
        flags);
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeBoolValue(*status);
  } else {
    // This is the "slow path".
    auto res = toObject(runtime, Handle<>(target));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      // If an exception is thrown, likely we are trying to convert
      // undefined/null to an object. Passing over the name of the property
      // so that we could emit more meaningful error messages.
      (void)amendPropAccessErrorMsgWithPropName(
          runtime, Handle<>(target), "delete", SymbolID::unsafeCreate(sid));
      return ExecutionStatus::EXCEPTION;
    }
    PinnedHermesValue &scratch = runtime->getCurrentFrame().getScratchRef();
    scratch = res.getValue();
    auto status = JSObject::deleteNamed(
        Handle<JSObject>::vmcast(&scratch),
        runtime,
        SymbolID::unsafeCreate(sid),
        flags);
    scratch = HermesValue::encodeUndefinedValue();
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeBoolValue(status.getValue());
  }
}

void externStoreToEnvironment(
    PinnedHermesValue *env,
    uint32_t idx,
    PinnedHermesValue *val,
    Runtime *runtime) {
  // TODO: emit a check if \p val is non-pointer in fast path, if yes, set the
  // value directly inline without emitting an external call
  vmcast<Environment>(*env)->slot(idx).set(*val, &runtime->getHeap());
}
void externStoreNPToEnvironment(
    PinnedHermesValue *env,
    uint32_t idx,
    PinnedHermesValue *val,
    Runtime *runtime) {
  // TODO: emit the value setting directly in fast path without emitting an
  // external call
  vmcast<Environment>(*env)->slot(idx).setNonPtr(*val, &runtime->getHeap());
}

HermesValue externLoadFromEnvironment(PinnedHermesValue *env, uint32_t idx) {
  // TODO: emit the instruction directly in fast path instead of emitting an
  // external call
  return vmcast<Environment>(*env)->slot(idx);
}

CallResult<HermesValue>
externMod(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) {
  if (LLVM_LIKELY(op1->isNumber() && op2->isNumber())) {
    /* Fast-path. */
    return HermesValue::encodeDoubleValue(
        std::fmod(op1->getNumber(), op2->getNumber()));
  }

  GCScopeMarkerRAII marker{runtime};

  CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};
  if (LLVM_UNLIKELY(
          (res = toNumber_RJS(runtime, Handle<>(op1))) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  double left = res->getDouble();

  if (LLVM_UNLIKELY(
          (res = toNumber_RJS(runtime, Handle<>(op2))) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return HermesValue::encodeDoubleValue(std::fmod(left, res->getDouble()));
}

/// Implement a shift instruction
/// \param name the name of the instruction.
/// \param oper the C++ operator to actually perform the shift operation.
/// \param lConv the conversion function for the LHS of the expression.
/// \param lType the type of the LHS operand.
/// \param returnType the type of the return value.
#define IMPLEMENT_SHIFT_OP(name, oper, lConv, lType, returnType)             \
  CallResult<HermesValue> extern##name(                                      \
      Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) {    \
    if (LLVM_LIKELY(op1->isNumber() && op2->isNumber())) { /* Fast-path. */  \
      auto lnum =                                                            \
          static_cast<lType>(hermes::truncateToInt32(op1->getNumber()));     \
      auto rnum =                                                            \
          static_cast<uint32_t>(hermes::truncateToInt32(op2->getNumber())) & \
          0x1f;                                                              \
      return HermesValue::encodeDoubleValue(                                 \
          static_cast<returnType>(lnum oper rnum));                          \
    }                                                                        \
    GCScopeMarkerRAII marker{runtime};                                       \
    CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};                 \
    if (LLVM_UNLIKELY(                                                       \
            (res = lConv(runtime, Handle<>(op1))) ==                         \
            ExecutionStatus::EXCEPTION)) {                                   \
      return ExecutionStatus::EXCEPTION;                                     \
    }                                                                        \
    auto lnum = static_cast<lType>(res->getNumber());                        \
    if (LLVM_UNLIKELY(                                                       \
            (res = toUInt32_RJS(runtime, Handle<>(op2))) ==                  \
            ExecutionStatus::EXCEPTION)) {                                   \
      return ExecutionStatus::EXCEPTION;                                     \
    }                                                                        \
    auto rnum = static_cast<uint32_t>(res->getNumber()) & 0x1f;              \
    return HermesValue::encodeDoubleValue(                                   \
        static_cast<returnType>(lnum oper rnum));                            \
  }

IMPLEMENT_SHIFT_OP(LShift, <<, toUInt32_RJS, uint32_t, int32_t);
IMPLEMENT_SHIFT_OP(RShift, >>, toInt32_RJS, int32_t, int32_t);
IMPLEMENT_SHIFT_OP(URshift, >>, toUInt32_RJS, uint32_t, uint32_t);

/// Implement a binary bitwise instruction with a fast path where both
/// operands are numbers.
/// \param name the name of the instruction.
/// \param oper the C++ operator to use to actually perform the bitwise
///     operation.
#define IMPLEMENT_BITWISEBIN_OP(name, oper)                                  \
  CallResult<HermesValue> extern##name(                                      \
      Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2) {    \
    if (LLVM_LIKELY(op1->isNumber() && op2->isNumber())) {                   \
      /* Fast-path. */                                                       \
      return HermesValue::encodeDoubleValue(hermes::truncateToInt32(         \
          op1->getNumber()) oper hermes::truncateToInt32(op2->getNumber())); \
    }                                                                        \
    GCScopeMarkerRAII marker{runtime};                                       \
    CallResult<HermesValue> res{ExecutionStatus::EXCEPTION};                 \
    if (LLVM_UNLIKELY(                                                       \
            (res = toInt32_RJS(runtime, Handle<>(op1))) ==                   \
            ExecutionStatus::EXCEPTION)) {                                   \
      return ExecutionStatus::EXCEPTION;                                     \
    }                                                                        \
    int32_t left = res->getNumberAs<int32_t>();                              \
    if (LLVM_UNLIKELY(                                                       \
            (res = toInt32_RJS(runtime, Handle<>(op2))) ==                   \
            ExecutionStatus::EXCEPTION)) {                                   \
      return ExecutionStatus::EXCEPTION;                                     \
    }                                                                        \
    return HermesValue::encodeNumberValue(                                   \
        left oper res->getNumberAs<int32_t>());                              \
  }

IMPLEMENT_BITWISEBIN_OP(BitAnd, &);
IMPLEMENT_BITWISEBIN_OP(BitOr, |);
IMPLEMENT_BITWISEBIN_OP(BitXor, ^);

HermesValue externGetEnvironment(
    Runtime *runtime,
    PinnedHermesValue *frame,
    uint32_t numLevel) {
  Environment *curEnv =
      StackFramePtr{frame}.getCalleeClosureUnsafe()->getEnvironment(runtime);
  for (unsigned level = numLevel; level; --level) {
    assert(curEnv && "invalid environment relative level");
    curEnv = curEnv->getParentEnvironment(runtime);
  }
  return HermesValue::encodeObjectValue(curEnv);
}

CallResult<HermesValue> externNewObjectWithBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    uint32_t numLiterals,
    uint32_t keyBufferIndex,
    uint32_t valBufferIndex) {
  GCScopeMarkerRAII marker{runtime};
  return Interpreter::createObjectFromBuffer(
             runtime, curCodeBlock, numLiterals, keyBufferIndex, valBufferIndex)
      .toCallResultHermesValue();
}

CallResult<HermesValue> externNewArrayWithBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex) {
  GCScopeMarkerRAII marker{runtime};
  return Interpreter::createArrayFromBuffer(
             runtime, curCodeBlock, numElements, numLiterals, bufferIndex)
      .toCallResultHermesValue();
}

CallResult<HermesValue> slowPathNegate(
    Runtime *runtime,
    PinnedHermesValue *op1) {
  GCScopeMarkerRAII marker{runtime};
  auto res = toNumber_RJS(runtime, Handle<>(op1));
  if (res == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  return HermesValue::encodeNumberValue(-res->getNumber());
}

HermesValue externGetNextPName(
    Runtime *runtime,
    PinnedHermesValue *arrReg,
    PinnedHermesValue *objReg,
    PinnedHermesValue *iterReg,
    PinnedHermesValue *sizeReg) {
  GCScopeMarkerRAII marker{runtime};

  assert(
      vmisa<BigStorage>(*arrReg) && "GetNextPName's second op must be JSArray");
  auto obj = Handle<JSObject>::vmcast(objReg);
  auto arr = Handle<BigStorage>::vmcast(arrReg);
  uint32_t idx = iterReg->getNumber();
  uint32_t size = sizeReg->getNumber();
  PinnedHermesValue *scratch = &runtime->getCurrentFrame().getScratchRef();
  MutableHandle<JSObject> propObj{runtime};

  // Loop until we find a property which is present.
  while (idx < size) {
    *scratch = arr->at(idx);
    ComputedPropertyDescriptor desc;
    JSObject::getComputedPrimitiveDescriptor(
        obj, runtime, Handle<>(scratch), propObj, desc);
    if (LLVM_LIKELY(propObj))
      break;
    ++idx;
  }
  if (idx < size) {
    // We must return the property as a string
    if (scratch->isNumber()) {
      auto status = toString_RJS(runtime, Handle<>(scratch));
      assert(
          status == ExecutionStatus::RETURNED &&
          "toString on number cannot fail");
      *scratch = status->getHermesValue();
    }
    *iterReg = HermesValue::encodeNumberValue(idx + 1);
    return *scratch;

  } else {
    return HermesValue::encodeUndefinedValue();
  }
}

CallResult<HermesValue> externSlowPathReifyArguments(
    Runtime *runtime,
    PinnedHermesValue *currentFrame,
    bool isStrict) {
  GCScopeMarkerRAII marker{runtime};

  StackFramePtr frame(currentFrame);

  auto res = Interpreter::reifyArgumentsSlowPath(
      runtime, frame.getCalleeClosureHandleUnsafe(), isStrict);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return res->getHermesValue();
}

CallResult<HermesValue> externSlowPathGetArgumentsPropByVal(
    Runtime *runtime,
    PinnedHermesValue *lazyReg,
    PinnedHermesValue *valueReg,
    PinnedHermesValue *currentFrame,
    bool isStrict) {
  GCScopeMarkerRAII marker{runtime};

  StackFramePtr frame(currentFrame);

  return Interpreter::getArgumentsPropByValSlowPath_RJS(
             runtime,
             lazyReg,
             valueReg,
             frame.getCalleeClosureHandleUnsafe(),
             isStrict)
      .toCallResultHermesValue();
}

CallResult<HermesValue> externSlowPathBitNot(
    Runtime *runtime,
    PinnedHermesValue *op) {
  GCScopeMarkerRAII marker{runtime};

  auto res = toInt32_RJS(runtime, Handle<>(op));
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeNumberValue(
      ~static_cast<int32_t>(res->getNumber()));
}

CallResult<HermesValue> slowPathGetArgumentsLength(
    Runtime *runtime,
    PinnedHermesValue *obj) {
  GCScopeMarkerRAII marker{runtime};
  assert(obj->isObject() && "arguments lazy register is not an object");
  return JSObject::getNamed_RJS(
             Handle<JSObject>::vmcast(obj),
             runtime,
             Predefined::getSymbolID(Predefined::length))
      .toCallResultHermesValue();
}

CallResult<HermesValue> externIsIn(
    Runtime *runtime,
    PinnedHermesValue *propName,
    PinnedHermesValue *obj) {
  GCScopeMarkerRAII marker{runtime};

  if (LLVM_UNLIKELY(!obj->isObject())) {
    return runtime->raiseTypeError("right operand of 'in' is not an object");
  }

  CallResult<bool> resultRes = JSObject::hasComputed(
      Handle<JSObject>::vmcast(obj), runtime, Handle<>(propName));

  if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeBoolValue(*resultRes);
}

CallResult<HermesValue> externInstanceOf(
    Runtime *runtime,
    PinnedHermesValue *obj,
    PinnedHermesValue *constructor) {
  GCScopeMarkerRAII marker{runtime};

  auto res =
      instanceOfOperator_RJS(runtime, Handle<>(obj), Handle<>(constructor));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(*res);
}

CallResult<HermesValue> externCreateRegExpMayAllocate(
    Runtime *runtime,
    uint32_t patternIdx,
    uint32_t flagsIdx,
    uint32_t bytecodeIdx,
    CodeBlock *codeBlock) {
  GCScopeMarkerRAII marker{runtime};

  // Create the RegExp object.
  Handle<JSRegExp> re = JSRegExp::create(runtime);
  // Initialize the regexp.
  auto pattern = runtime->makeHandle(
      codeBlock->getRuntimeModule()->getStringPrimFromStringIDMayAllocate(
          patternIdx));
  auto flags = runtime->makeHandle(
      codeBlock->getRuntimeModule()->getStringPrimFromStringIDMayAllocate(
          flagsIdx));
  auto bytecode =
      codeBlock->getRuntimeModule()->getRegExpBytecodeFromRegExpID(bytecodeIdx);
  if (LLVM_UNLIKELY(
          JSRegExp::initialize(re, runtime, pattern, flags, bytecode) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return re.getHermesValue();
}

} // namespace vm
} // namespace hermes
