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
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/Runtime-inline.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

#include "Interpreter-internal.h"

namespace hermes {
namespace vm {

void Interpreter::saveGenerator(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const Inst *resumeIP) {
  auto *innerFn =
      vmcast<GeneratorInnerFunction>(FRAME.getCalleeClosureUnsafe());
  innerFn->saveStack(runtime);
  innerFn->setNextIP(runtime, resumeIP);
  innerFn->setState(GeneratorInnerFunction::State::SuspendedYield);
}

ExecutionStatus Interpreter::caseDirectEval(
    Runtime &runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  auto *result = &O1REG(DirectEval);
  auto *input = &O2REG(DirectEval);

  GCScopeMarkerRAII gcMarker{runtime};

  // Check to see if global eval() has been overriden, in which case call it as
  // as normal function.
  auto global = runtime.getGlobal();
  auto existingEval = global->getNamed_RJS(
      global, runtime, Predefined::getSymbolID(Predefined::eval));
  if (LLVM_UNLIKELY(existingEval == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto *nativeExistingEval = dyn_vmcast<NativeFunction>(existingEval->get());
  if (LLVM_UNLIKELY(
          !nativeExistingEval ||
          nativeExistingEval->getFunctionPtr() != hermes::vm::eval)) {
    if (auto *existingEvalCallable =
            dyn_vmcast<Callable>(existingEval->get())) {
      auto evalRes = existingEvalCallable->executeCall1(
          runtime.makeHandle<Callable>(existingEvalCallable),
          runtime,
          Runtime::getUndefinedValue(),
          *input);
      if (LLVM_UNLIKELY(evalRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      *result = evalRes->get();
      evalRes->invalidate();
      return ExecutionStatus::RETURNED;
    }
    return runtime.raiseTypeErrorForValue(
        runtime.makeHandle(std::move(*existingEval)), " is not a function");
  }

  if (!input->isString()) {
    *result = *input;
    return ExecutionStatus::RETURNED;
  }

  // Create a dummy scope, so that the local eval executes in its own scope
  // (as per the spec for strict callers, which is the only thing we support).

  ScopeChain scopeChain{};
  scopeChain.functions.emplace_back();

  auto cr = vm::directEval(
      runtime, Handle<StringPrimitive>::vmcast(input), scopeChain, false);
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

  auto res = PropertyAccessor::create(runtime, getter, setter);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  auto accessor = runtime.makeHandle<PropertyAccessor>(*res);

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
              slotValue->getRaw() == runtime.arrayPrototypeValues.getRaw())) {
        O1REG(IteratorBegin) = HermesValue::encodeNumberValue(0);
        return ExecutionStatus::RETURNED;
      }
    }
  }
  GCScopeMarkerRAII marker{runtime};
  CallResult<IteratorRecord> iterRecord =
      getIterator(runtime, Handle<>(&O2REG(IteratorBegin)));
  if (LLVM_UNLIKELY(iterRecord == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  O1REG(IteratorBegin) = iterRecord->iterator.getHermesValue();
  O2REG(IteratorBegin) = iterRecord->nextMethod.getHermesValue();
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
        O1REG(IteratorNext) = value.unboxToHV(runtime);
        O2REG(IteratorNext) = HermesValue::encodeNumberValue(i + 1);
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
    O1REG(IteratorNext) = valueRes->get();
    O2REG(IteratorNext) = HermesValue::encodeNumberValue(i + 1);
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

  GCScopeMarkerRAII marker{runtime};

  IteratorRecord iterRecord{
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
  O1REG(GetPNameList) = arr.getHermesValue();
  O3REG(GetPNameList) = HermesValue::encodeNumberValue(beginIndex);
  O4REG(GetPNameList) = HermesValue::encodeNumberValue(endIndex);
  return ExecutionStatus::RETURNED;
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
      runtime.stackPointer_, FRAME, ip, curCodeBlock, op3 - 1, nf, false);
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
doOperSlowPath(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
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
    return HermesValue::encodeDoubleValue(Oper(left, res->getDouble()));
  }
  return doBigIntBinOp(
      runtime, BigIntOper<Oper>, runtime.makeHandle(res->getBigInt()), rhs);
}

template CallResult<HermesValue>
doOperSlowPath<doDiv>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath<doMod>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath<doMul>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doOperSlowPath<doSub>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template <auto Oper>
CallResult<HermesValue>
doBitOperSlowPath(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
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
    return HermesValue::encodeNumberValue(
        Oper(left, res->getNumberAs<int32_t>()));
  }
  return doBigIntBinOp(
      runtime, BigIntOper<Oper>, runtime.makeHandle(res->getBigInt()), rhs);
}

template CallResult<HermesValue>
doBitOperSlowPath<doBitAnd>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doBitOperSlowPath<doBitOr>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doBitOperSlowPath<doBitXor>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

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
doShiftOperSlowPath(Runtime &runtime, Handle<> lhs, Handle<> rhs) {
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
    return HermesValue::encodeDoubleValue((*Oper)(lnum, rnum));
  }
  return doBigIntBinOp(
      runtime,
      BigIntOper<Oper>,
      runtime.makeHandle(res->getBigInt()),
      std::move(rhs));
}

template CallResult<HermesValue>
doShiftOperSlowPath<doLShift>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doShiftOperSlowPath<doRShift>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template CallResult<HermesValue>
doShiftOperSlowPath<doURshift>(Runtime &runtime, Handle<> lhs, Handle<> rhs);

template <auto Oper>
CallResult<HermesValue> doIncDecOperSlowPath(Runtime &runtime, Handle<> src) {
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
    return HermesValue::encodeNumberValue(Oper(res->getNumber()));
  }

  return BigIntOper<Oper>(runtime, runtime.makeHandle(res->getBigInt()));
}

template CallResult<HermesValue> doIncDecOperSlowPath<doInc>(
    Runtime &runtime,
    Handle<> src);

template CallResult<HermesValue> doIncDecOperSlowPath<doDec>(
    Runtime &runtime,
    Handle<> src);

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
