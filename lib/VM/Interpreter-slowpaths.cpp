/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "vm"
#include "JSLib/JSLibInternal.h"
#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSCallableProxy.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/Runtime-inline.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

#include "Interpreter-internal.h"

namespace hermes {
namespace vm {

ExecutionStatus Interpreter::caseDirectEval(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  auto *result = &O1REG(DirectEval);
  auto *evalText = &O2REG(DirectEval);
  bool strictCaller = ip->iDirectEval.op3;

  if (!evalText->isString()) {
    *result = *evalText;
    return ExecutionStatus::RETURNED;
  }

  GCScopeMarkerRAII gcMarker{runtime};

  auto cr = vm::directEval(
      runtime,
      Handle<StringPrimitive>::vmcast(evalText),
      strictCaller,
      nullptr,
      false);
  if (cr == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  *result = *cr;
  return ExecutionStatus::RETURNED;
}

ExecutionStatus Interpreter::casePutOwnByVal(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  return JSObject::defineOwnComputed(
             Handle<JSObject>::vmcast(&O1REG(PutOwnByVal)),
             runtime,
             Handle<>(&O3REG(PutOwnByVal)),
             ip->iPutOwnByVal.op4
                 ? DefinePropertyFlags::getDefaultNewPropertyFlags()
                 : DefinePropertyFlags::getNewNonEnumerableFlags(),
             Handle<>(&O2REG(PutOwnByVal)))
      .getStatus();
}

ExecutionStatus Interpreter::casePutOwnGetterSetterByVal(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const inst::Inst *ip) {
  DefinePropertyFlags dpFlags{};
  dpFlags.setConfigurable = 1;
  dpFlags.configurable = 1;
  dpFlags.setEnumerable = 1;
  dpFlags.enumerable = ip->iPutOwnGetterSetterByVal.op5;

  MutableHandle<Callable> getter(runtime);
  MutableHandle<Callable> setter(runtime);
  if (LLVM_LIKELY(!O3REG(PutOwnGetterSetterByVal).isUndefined())) {
    dpFlags.setGetter = 1;
    getter = vmcast<Callable>(O3REG(PutOwnGetterSetterByVal));
  }
  if (LLVM_LIKELY(!O4REG(PutOwnGetterSetterByVal).isUndefined())) {
    dpFlags.setSetter = 1;
    setter = vmcast<Callable>(O4REG(PutOwnGetterSetterByVal));
  }
  assert(
      (dpFlags.setSetter || dpFlags.setGetter) &&
      "No accessor set in PutOwnGetterSetterByVal");

  auto accessor = runtime.makeHandle<PropertyAccessor>(
      PropertyAccessor::create(runtime, getter, setter));

  return JSObject::defineOwnComputed(
             Handle<JSObject>::vmcast(&O1REG(PutOwnGetterSetterByVal)),
             runtime,
             Handle<>(&O2REG(PutOwnGetterSetterByVal)),
             dpFlags,
             accessor)
      .getStatus();
}

ExecutionStatus Interpreter::caseIteratorBegin(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const inst::Inst *ip) {
  if (LLVM_LIKELY(vmisa<JSArray>(O2REG(IteratorBegin)))) {
    // Attempt to get the fast path for array iteration.
    NamedPropertyDescriptor desc;
    JSObject *propObj = JSObject::getNamedDescriptorPredefined(
        Handle<JSArray>::vmcast(&O2REG(IteratorBegin)),
        runtime,
        Predefined::SymbolIterator,
        desc);
    if (LLVM_LIKELY(propObj)) {
      auto slotValueRes = JSObject::getNamedSlotValue(
          createPseudoHandle(propObj), runtime, desc);
      if (LLVM_UNLIKELY(slotValueRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      PseudoHandle<> slotValue = std::move(*slotValueRes);
      if (LLVM_LIKELY(
              slotValue->getRaw() ==
              runtime.arrayPrototypeValues.getHermesValue().getRaw())) {
        O1REG(IteratorBegin) = HermesValue::encodeTrustedNumberValue(0);
        return ExecutionStatus::RETURNED;
      }
    }
  }
  GCScopeMarkerRAII marker{runtime};
  CallResult<UncheckedIteratorRecord> iterRecord =
      getIterator(runtime, Handle<>(&O2REG(IteratorBegin)));
  if (LLVM_UNLIKELY(iterRecord == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  O2REG(IteratorBegin) = iterRecord->nextMethod.getHermesValue();
  // Write the result last in case it is the same register as O2REG.
  O1REG(IteratorBegin) = iterRecord->iterator.getHermesValue();
  return ExecutionStatus::RETURNED;
}

ExecutionStatus Interpreter::caseIteratorNext(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const inst::Inst *ip) {
  if (LLVM_LIKELY(O2REG(IteratorNext).isNumber())) {
    JSArray::size_type i =
        O2REG(IteratorNext).getNumberAs<JSArray::size_type>();
    if (i >=
        JSArray::getLength(vmcast<JSArray>(O3REG(IteratorNext)), runtime)) {
      // Finished iterating the array, stop.
      O2REG(IteratorNext) = HermesValue::encodeUndefinedValue();
      O1REG(IteratorNext) = HermesValue::encodeUndefinedValue();
      return ExecutionStatus::RETURNED;
    }
    Handle<JSArray> arr = Handle<JSArray>::vmcast(&O3REG(IteratorNext));
    {
      // Fast path: look up the property in indexed storage.
      // Runs when there is no hole and a regular non-accessor property exists
      // at the current index, because those are the only properties stored
      // in indexed storage.
      // If there is another kind of property we have to call getComputed_RJS.
      // No need to check the fastIndexProperties flag because the indexed
      // storage would be deleted and at() would return empty in that case.
      NoAllocScope noAlloc{runtime};
      SmallHermesValue value = arr->at(runtime, i);
      if (LLVM_LIKELY(!value.isEmpty())) {
        O2REG(IteratorNext) = HermesValue::encodeTrustedNumberValue(i + 1);
        // Write the result last in case it is the same register as O2REG.
        O1REG(IteratorNext) = value.unboxToHV(runtime);
        return ExecutionStatus::RETURNED;
      }
    }
    // Slow path, just run the full getComputedPropertyValue_RJS path.
    GCScopeMarkerRAII marker{runtime};
    Handle<> idxHandle{&O2REG(IteratorNext)};
    CallResult<PseudoHandle<>> valueRes =
        JSObject::getComputed_RJS(arr, runtime, idxHandle);
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    O2REG(IteratorNext) = HermesValue::encodeTrustedNumberValue(i + 1);
    // Write the result last in case it is the same register as O2REG.
    O1REG(IteratorNext) = valueRes->get();
    return ExecutionStatus::RETURNED;
  }
  if (LLVM_UNLIKELY(O2REG(IteratorNext).isUndefined())) {
    // In all current use cases of IteratorNext, we check and branch away
    // from IteratorNext in the case that iterStorage was set to undefined
    // (which indicates completion of iteration).
    // If we introduce a use case which allows calling IteratorNext,
    // then this assert can be removed. For now, this branch just returned
    // undefined in NDEBUG mode.
    assert(false && "IteratorNext called on completed iterator");
    O1REG(IteratorNext) = HermesValue::encodeUndefinedValue();
    return ExecutionStatus::RETURNED;
  }

  if (LLVM_UNLIKELY(!vmisa<Callable>(O3REG(IteratorNext)))) {
    return runtime.raiseTypeError("'next' method on iterator must be callable");
  }

  GCScopeMarkerRAII marker{runtime};

  CheckedIteratorRecord iterRecord{
      Handle<JSObject>::vmcast(&O2REG(IteratorNext)),
      Handle<Callable>::vmcast(&O3REG(IteratorNext))};

  CallResult<PseudoHandle<JSObject>> resultObjRes =
      iteratorNext(runtime, iterRecord, llvh::None);
  if (LLVM_UNLIKELY(resultObjRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> resultObj = runtime.makeHandle(std::move(*resultObjRes));
  CallResult<PseudoHandle<>> doneRes = JSObject::getNamed_RJS(
      resultObj, runtime, Predefined::getSymbolID(Predefined::done));
  if (LLVM_UNLIKELY(doneRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (toBoolean(doneRes->get())) {
    // Done with iteration. Clear the iterator so that subsequent
    // instructions do not call next() or return().
    O2REG(IteratorNext) = HermesValue::encodeUndefinedValue();
    O1REG(IteratorNext) = HermesValue::encodeUndefinedValue();
  } else {
    // Not done iterating, so get the `value` property and store it
    // as the result.
    CallResult<PseudoHandle<>> propRes = JSObject::getNamed_RJS(
        resultObj, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    O1REG(IteratorNext) = propRes->get();
    propRes->invalidate();
  }
  return ExecutionStatus::RETURNED;
}

ExecutionStatus Interpreter::caseGetPNameList(
    Runtime &runtime,
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

  auto obj = runtime.makeMutableHandle(vmcast<JSObject>(res.getValue()));
  uint32_t beginIndex;
  uint32_t endIndex;
  auto cr = getForInPropertyNames(runtime, obj, beginIndex, endIndex);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arr = *cr;
  O3REG(GetPNameList) = HermesValue::encodeTrustedNumberValue(beginIndex);
  O4REG(GetPNameList) = HermesValue::encodeTrustedNumberValue(endIndex);
  // Write the result last in case it is the same register as one of the in/out
  // operands.
  O1REG(GetPNameList) = arr.getHermesValue();
  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue> Interpreter::createThisImpl(
    Runtime &runtime,
    PinnedHermesValue *callee,
    PinnedHermesValue *newTarget,
    uint8_t cacheIdx,
    CodeBlock *curCodeBlock) {
  if (LLVM_UNLIKELY(!callee->isObject())) {
    // Add a leading space because the value will come first when we use
    // raiseTypeErrorForValue.
    return runtime.raiseTypeErrorForValue(
        Handle<>(callee), " cannot be used as a constructor.");
  }

  auto *calleeFunc = vmcast<JSObject>(*callee);
  auto cellKind = calleeFunc->getKind();
  Callable *correctNewTarget;
  // CellKind.h documents the invariants we take advantage of here to
  // efficiently check what kind of callable we are dealing with.
  // If this is a callable that expects a `this`, then skip ahead and start
  // making the object.
  if (cellKind >= CellKind::CallableExpectsThisKind_first) {
    correctNewTarget = vmcast<Callable>(*newTarget);
  } else if (cellKind >= CellKind::CallableMakesThisKind_first) {
    // Callables that make their own this should be given undefined in a
    // construct call.
    return HermesValue::encodeUndefinedValue();
  } else if (cellKind >= CellKind::CallableKind_first) {
    correctNewTarget = vmcast<Callable>(*newTarget);
    while (auto *bound = dyn_vmcast<BoundFunction>(calleeFunc)) {
      calleeFunc = bound->getTarget(runtime);
      // From ES15 10.4.1.2 [[Construct]] Step 5: If SameValue(F, newTarget) is
      // true, set newTarget to target.
      if (bound == vmcast<Callable>(correctNewTarget)) {
        correctNewTarget = vmcast<Callable>(calleeFunc);
      }
    }
    cellKind = calleeFunc->getKind();
    // Repeat the checks, now against the target.
    if (cellKind >= CellKind::CallableExpectsThisKind_first) {
      correctNewTarget = vmcast<Callable>(calleeFunc);
    } else if (cellKind >= CellKind::CallableMakesThisKind_first) {
      return HermesValue::encodeUndefinedValue();
    } else {
      // If we still can't recognize what to do after advancing through bound
      // functions (if any), error out. This code path is hit when invoking a
      // NativeFunction as a constructor.
      return runtime.raiseTypeError(
          "This function cannot be used as a constructor.");
    }
  } else {
    // Not a Callable.
    return runtime.raiseTypeErrorForValue(
        Handle<>(callee), " cannot be used as a constructor.");
  }

  // We shouldn't need to check that new.target is a constructor explicitly.
  // There are 2 cases where this instruction is emitted.
  // 1. new expressions. In this case, new.target == callee. We always verify
  // that a function call is performed correctly, so don't need to double-verify
  // new.target.
  // 2. super() calls in derived constructors. In this case, new.target will be
  // set to the original callee for the new expression which triggered the
  // constructor now invoking super. This means that new.target will be checked.
  // Unfortunately, this check for constructors is done *after* this
  // instruction, so we can't add an assert that newTarget is a constructor
  // here. For example, using `new` on an arrow function, newTarget is not a
  // constructor, but we don't find that out until after this instruction.

  struct : public Locals {
    PinnedValue<Callable> newTarget;
    // This is the .prototype of new.target
    PinnedValue<JSObject> newTargetPrototype;
  } lv;
  LocalsRAII lraii(runtime, &lv);
  lv.newTarget = correctNewTarget;

  auto *cacheEntry = curCodeBlock->getReadCacheEntry(cacheIdx);
  CompressedPointer clazzPtr{lv.newTarget->getClassGCPtr()};
  // If we have a cache hit, reuse the cached offset and immediately
  // return the property.
  if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
    auto shvPrototype = JSObject::getNamedSlotValueUnsafe(
        *lv.newTarget, runtime, cacheEntry->slot);
    if (LLVM_LIKELY(shvPrototype.isObject())) {
      lv.newTargetPrototype = vmcast<JSObject>(shvPrototype.getObject(runtime));
    } else {
      lv.newTargetPrototype = runtime.objectPrototype;
    }
  } else {
    GCScopeMarkerRAII marker{runtime};
    auto newTargetProtoRes = JSObject::getNamed_RJS(
        lv.newTarget,
        runtime,
        Predefined::getSymbolID(Predefined::prototype),
        PropOpFlags(),
        cacheIdx != hbc::PROPERTY_CACHING_DISABLED ? cacheEntry : nullptr);
    if (LLVM_UNLIKELY(newTargetProtoRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if ((*newTargetProtoRes)->isObject()) {
      lv.newTargetPrototype = vmcast<JSObject>(newTargetProtoRes->get());
    } else {
      lv.newTargetPrototype = runtime.objectPrototype;
    }
  }

  return JSObject::create(runtime, lv.newTargetPrototype).getHermesValue();
}

ExecutionStatus Interpreter::implCallBuiltin(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    CodeBlock *curCodeBlock,
    uint32_t op3) {
  const Inst *ip = runtime.getCurrentIP();
  uint8_t methodIndex = ip->iCallBuiltin.op2;
  Callable *callable = runtime.getBuiltinCallable(methodIndex);
  assert(
      isNativeBuiltin(methodIndex) &&
      "CallBuiltin must take a native builtin.");
  NativeFunction *nf = vmcast<NativeFunction>(callable);

  auto newFrame = StackFramePtr::initFrame(
      runtime.stackPointer_,
      FRAME,
      ip,
      curCodeBlock,
      nullptr,
      op3 - 1,
      nf,
      false);
  // "thisArg" is implicitly assumed to "undefined".
  newFrame.getThisArgRef() = HermesValue::encodeUndefinedValue();

  SLOW_DEBUG(dumpCallArguments(llvh::dbgs(), runtime, newFrame));

  auto resPH = NativeFunction::_nativeCall(nf, runtime);
  if (LLVM_UNLIKELY(resPH == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  O1REG(CallBuiltin) = std::move(resPH->get());
  SLOW_DEBUG(
      llvh::dbgs() << "native return value r" << (unsigned)ip->iCallBuiltin.op1
                   << "=" << DumpHermesValue(O1REG(CallBuiltin)) << "\n");
  return ExecutionStatus::RETURNED;
}

ExecutionStatus Interpreter::declareGlobalVarImpl(
    Runtime &runtime,
    CodeBlock *curCodeBlock,
    const Inst *ip) {
  GCScopeMarkerRAII mark{runtime};
  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.configurable = 0;
  // Do not overwrite existing globals with undefined.
  dpf.setValue = 0;

  CallResult<bool> res = JSObject::defineOwnProperty(
      runtime.getGlobal(),
      runtime,
      ID(ip->iDeclareGlobalVar.op1),
      dpf,
      Runtime::getUndefinedValue(),
      PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
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
    if (!JSObject::getOwnNamedDescriptor(
            runtime.getGlobal(),
            runtime,
            ID(ip->iDeclareGlobalVar.op1),
            desc)) {
      return ExecutionStatus::EXCEPTION;
    }
    runtime.clearThrownValue();
  }
  return ExecutionStatus::RETURNED;
}

using BigIntBinaryOp = CallResult<HermesValue>(
    Runtime &,
    Handle<BigIntPrimitive>,
    Handle<BigIntPrimitive>);

static CallResult<HermesValue> doBigIntBinOp(
    Runtime &runtime,
    BigIntBinaryOp Oper,
    Handle<BigIntPrimitive> lhs,
    Handle<> rhs) {
  // Cannot use ToBigInt here as it would incorrectly allow boolean/string rhs.
  CallResult<HermesValue> res = toNumeric_RJS(runtime, rhs);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!res->isBigInt()) {
    return runtime.raiseTypeErrorForValue("Cannot convert ", rhs, " to BigInt");
  }
  return Oper(runtime, lhs, runtime.makeHandle(res->getBigInt()));
}

namespace {
/// BigIntOper maps the \param Oper (a Number operation) to its respective
/// BigIntPrimitive counterpart.
template <auto Oper>
int BigIntOper;

template <>
constexpr auto &BigIntOper<doDiv> = BigIntPrimitive::divide;

template <>
constexpr auto &BigIntOper<doMod> = BigIntPrimitive::remainder;

template <>
constexpr auto &BigIntOper<doSub> = BigIntPrimitive::subtract;

template <>
constexpr auto &BigIntOper<doMul> = BigIntPrimitive::multiply;

template <>
constexpr auto &BigIntOper<doBitAnd> = BigIntPrimitive::bitwiseAND;

template <>
constexpr auto &BigIntOper<doBitOr> = BigIntPrimitive::bitwiseOR;

template <>
constexpr auto &BigIntOper<doBitXor> = BigIntPrimitive::bitwiseXOR;

template <>
constexpr auto &BigIntOper<doLShift> = BigIntPrimitive::leftShift;

template <>
constexpr auto &BigIntOper<doRShift> = BigIntPrimitive::signedRightShift;

template <>
constexpr auto &BigIntOper<doURshift> = BigIntPrimitive::unsignedRightShift;

template <>
constexpr auto &BigIntOper<doInc> = BigIntPrimitive::inc;

template <>
constexpr auto &BigIntOper<doDec> = BigIntPrimitive::dec;

} // namespace

template <auto Oper>
CallResult<HermesValue>
doOperSlowPath_RJS(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
  CallResult<HermesValue> res =
      toPrimitive_RJS(runtime, lhs, PreferredType::NUMBER);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_LIKELY(!res->isBigInt())) {
    res = toNumber_RJS(runtime, runtime.makeHandle(*res));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    double left = res->getDouble();
    res = toNumber_RJS(runtime, rhs);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeTrustedNumberValue(Oper(left, res->getDouble()));
  }
  return doBigIntBinOp(
      runtime, BigIntOper<Oper>, runtime.makeHandle(res->getBigInt()), rhs);
}

template CallResult<HermesValue>
doOperSlowPath_RJS<doDiv>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath_RJS<doMod>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath_RJS<doMul>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath_RJS<doSub>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template <auto Oper>
CallResult<HermesValue>
doBitOperSlowPath_RJS(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
  CallResult<HermesValue> res =
      toPrimitive_RJS(runtime, lhs, PreferredType::NUMBER);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_LIKELY(!res->isBigInt())) {
    res = toInt32_RJS(runtime, runtime.makeHandle(*res));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    const int32_t left = res->getNumberAs<int32_t>();
    res = toInt32_RJS(runtime, std::move(rhs));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeTrustedNumberValue(
        Oper(left, res->getNumberAs<int32_t>()));
  }
  return doBigIntBinOp(
      runtime, BigIntOper<Oper>, runtime.makeHandle(res->getBigInt()), rhs);
}

template CallResult<HermesValue>
doBitOperSlowPath_RJS<doBitAnd>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doBitOperSlowPath_RJS<doBitOr>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doBitOperSlowPath_RJS<doBitXor>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

namespace {
/// ToIntegral maps the \param Oper shift operation (on Number) to the function
/// used to convert the operation's lhs operand to integer.
template <auto Oper>
inline int ToIntegral;

// For LShift, we need to use toUInt32 first because lshift on negative
// numbers is undefined behavior in theory.
template <>
inline constexpr auto &ToIntegral<doLShift> = toUInt32_RJS;

template <>
inline constexpr auto &ToIntegral<doRShift> = toInt32_RJS;

template <>
inline constexpr auto &ToIntegral<doURshift> = toUInt32_RJS;
} // namespace

template <auto Oper>
CallResult<HermesValue>
doShiftOperSlowPath_RJS(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
  CallResult<HermesValue> res =
      toPrimitive_RJS(runtime, std::move(lhs), PreferredType::NUMBER);

  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_LIKELY(!res->isBigInt())) {
    res = ToIntegral<Oper>(runtime, runtime.makeHandle(*res));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto lnum = hermes::truncateToInt32(res->getNumber());
    res = toUInt32_RJS(runtime, rhs);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto rnum = static_cast<uint32_t>(res->getNumber()) & 0x1f;
    return HermesValue::encodeTrustedNumberValue((*Oper)(lnum, rnum));
  }
  return doBigIntBinOp(
      runtime,
      BigIntOper<Oper>,
      runtime.makeHandle(res->getBigInt()),
      std::move(rhs));
}

template CallResult<HermesValue>
doShiftOperSlowPath_RJS<doLShift>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doShiftOperSlowPath_RJS<doRShift>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue> doShiftOperSlowPath_RJS<doURshift>(
    Runtime &runtime,
    Handle<> lhs,
    Handle<> rhs);

template <auto Oper>
CallResult<HermesValue> doIncDecOperSlowPath_RJS(
    Runtime &runtime,
    Handle<> src) {
  CallResult<HermesValue> res =
      toPrimitive_RJS(runtime, std::move(src), PreferredType::NUMBER);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (LLVM_LIKELY(!res->isBigInt())) {
    res = toNumber_RJS(runtime, runtime.makeHandle(*res));
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return HermesValue::encodeTrustedNumberValue(Oper(res->getNumber()));
  }

  return BigIntOper<Oper>(runtime, runtime.makeHandle(res->getBigInt()));
}

template CallResult<HermesValue> doIncDecOperSlowPath_RJS<doInc>(
    Runtime &runtime,
    Handle<> src);

template CallResult<HermesValue> doIncDecOperSlowPath_RJS<doDec>(
    Runtime &runtime,
    Handle<> src);

CallResult<HermesValue> doBitNotSlowPath_RJS(Runtime &runtime, Handle<> src) {
  // Try converting src to a numeric.
  auto numRes = toNumeric_RJS(runtime, src);
  if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  // Test for BigInt since it is cheaper than testing for number. If it is a
  // number, truncate it and perform bitwise not.
  if (LLVM_LIKELY(!numRes->isBigInt()))
    return HermesValue::encodeTrustedNumberValue(
        ~hermes::truncateToInt32(numRes->getNumber()));

  // The result is a BigInt, perform a BigInt bitwise not.
  auto bigint = runtime.makeHandle(numRes->getBigInt());
  return BigIntPrimitive::unaryNOT(runtime, bigint);
}

CallResult<HermesValue> doNegateSlowPath_RJS(Runtime &runtime, Handle<> src) {
  // Try converting src to a numeric.
  auto numRes = toNumeric_RJS(runtime, src);
  if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  // Test for BigInt since it is cheaper than testing for number. If it is a
  // number, negate it and return.
  if (LLVM_LIKELY(!numRes->isBigInt()))
    return HermesValue::encodeTrustedNumberValue(-numRes->getNumber());

  // The result is a BigInt, perform a BigInt unary minus.
  auto bigint = runtime.makeHandle(numRes->getBigInt());
  return BigIntPrimitive::unaryMinus(runtime, bigint);
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
