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
  auto *nativeExistingEval = dyn_vmcast<NativeFunction>(*existingEval);
  if (LLVM_UNLIKELY(
          !nativeExistingEval ||
          nativeExistingEval->getFunctionPtr() != hermes::vm::eval)) {
    if (auto *existingEvalCallable = dyn_vmcast<Callable>(*existingEval)) {
      auto evalRes = existingEvalCallable->executeCall1(
          runtime->makeHandle<Callable>(existingEvalCallable),
          runtime,
          Runtime::getUndefinedValue(),
          *input);
      if (LLVM_UNLIKELY(evalRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      *result = *evalRes;
      return ExecutionStatus::RETURNED;
    }
    return runtime->raiseTypeErrorForValue(
        runtime->makeHandle(*existingEval), " is not a function");
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

} // namespace vm
} // namespace hermes
