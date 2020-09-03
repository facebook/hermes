/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "vm"
#include "JSLib/JSLibInternal.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/StringPrimitive.h"

#include "Interpreter-internal.h"

using namespace hermes::inst;

namespace hermes {
namespace vm {

ExecutionStatus Interpreter::caseDirectEval(
    Runtime *runtime,
    PinnedHermesValue *frameRegs,
    const Inst *ip) {
  auto *result = &O1REG(DirectEval);
  auto *input = &O2REG(DirectEval);

  GCScopeMarkerRAII gcMarker{runtime};

  // Check to see if global eval() has been overriden, in which case call it as
  // as normal function.
  auto global = runtime->getGlobal();
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
          runtime->makeHandle<Callable>(existingEvalCallable),
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
    return runtime->raiseTypeErrorForValue(
        runtime->makeHandle(std::move(*existingEval)), " is not a function");
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
    Runtime *runtime,
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
    Runtime *runtime,
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

  auto accessor = runtime->makeHandle<PropertyAccessor>(*res);

  return JSObject::defineOwnComputed(
             Handle<JSObject>::vmcast(&O1REG(PutOwnGetterSetterByVal)),
             runtime,
             Handle<>(&O2REG(PutOwnGetterSetterByVal)),
             dpFlags,
             accessor)
      .getStatus();
}

ExecutionStatus Interpreter::caseIteratorBegin(
    Runtime *runtime,
    PinnedHermesValue *frameRegs,
    const inst::Inst *ip) {
  if (LLVM_LIKELY(vmisa<JSArray>(O2REG(IteratorBegin)))) {
    // Attempt to get the fast path for array iteration.
    NamedPropertyDescriptor desc;
    JSObject *propObj = JSObject::getNamedDescriptor(
        Handle<JSArray>::vmcast(&O2REG(IteratorBegin)),
        runtime,
        Predefined::getSymbolID(Predefined::SymbolIterator),
        desc);
    if (propObj) {
      HermesValue slotValue =
          JSObject::getNamedSlotValue(propObj, runtime, desc);
      if (slotValue.getRaw() == runtime->arrayPrototypeValues.getRaw()) {
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
    Runtime *runtime,
    PinnedHermesValue *frameRegs,
    const inst::Inst *ip) {
  if (LLVM_LIKELY(O2REG(IteratorNext).isNumber())) {
    JSArray::size_type i =
        O2REG(IteratorNext).getNumberAs<JSArray::size_type>();
    if (i >= JSArray::getLength(vmcast<JSArray>(O3REG(IteratorNext)))) {
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
      HermesValue value = arr->at(runtime, i);
      if (LLVM_LIKELY(!value.isEmpty())) {
        O1REG(IteratorNext) = value;
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

  IteratorRecord iterRecord{Handle<JSObject>::vmcast(&O2REG(IteratorNext)),
                            Handle<Callable>::vmcast(&O3REG(IteratorNext))};

  CallResult<PseudoHandle<JSObject>> resultObjRes =
      iteratorNext(runtime, iterRecord, llvh::None);
  if (LLVM_UNLIKELY(resultObjRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> resultObj = runtime->makeHandle(std::move(*resultObjRes));
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

} // namespace vm
} // namespace hermes
