/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 25.3.1 The %GeneratorPrototype% Object
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/JSGenerator.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

void populateGeneratorPrototype(Runtime &runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime.generatorPrototype);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::next),
      nullptr,
      generatorPrototypeNext,
      1);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::returnStr),
      /* isThrow */ (void *)false,
      generatorPrototypeReturnOrThrow,
      1);
  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::throwStr),
      /* isThrow */ (void *)true,
      generatorPrototypeReturnOrThrow,
      1);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;

  // The initial value of GeneratorFunction.prototype.constructor is the
  // intrinsic object %Generator% (which is GeneratorFunction.prototype).
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::constructor),
      Handle<>(&runtime.generatorFunctionPrototype),
      dpf);

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::Generator),
      dpf);
}

// ES6.0 25.3.3.2.
static CallResult<Handle<JSGenerator>> generatorValidate(
    Runtime &runtime,
    Handle<> value) {
  auto generator = Handle<JSGenerator>::dyn_vmcast(value);
  if (!generator) {
    return runtime.raiseTypeError(
        "Generator functions must be called on generators");
  }

  if (JSGenerator::getInnerFunction(runtime, *generator)->getState() ==
      GeneratorInnerFunction::State::Executing) {
    return runtime.raiseTypeError(
        "Generator functions may not be called on executing generators");
  }

  return generator;
}

// ES6.0 25.3.3.3.
// Placed separately from generatorPrototypeNext for readability and for simpler
// comparison to the spec.
static CallResult<Handle<JSObject>> generatorResume(
    Runtime &runtime,
    Handle<GeneratorInnerFunction> generator,
    Handle<> value) {
  if (generator->getState() == GeneratorInnerFunction::State::Completed) {
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true);
  }
  auto valueRes = GeneratorInnerFunction::callInnerFunction(
      generator, runtime, value, GeneratorInnerFunction::Action::Next);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    generator->setState(GeneratorInnerFunction::State::Completed);
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(generator->isDelegated())) {
    generator->setIsDelegated(false);
    return runtime.makeHandle<JSObject>(std::move(*valueRes));
  }
  return createIterResultObject(
      runtime,
      runtime.makeHandle(std::move(*valueRes)),
      generator->getState() == GeneratorInnerFunction::State::Completed);
}

CallResult<HermesValue>
generatorPrototypeNext(void *, Runtime &runtime, NativeArgs args) {
  auto generatorRes = generatorValidate(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(generatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto result = generatorResume(
      runtime,
      runtime.makeHandle(
          JSGenerator::getInnerFunction(runtime, generatorRes->get())),
      args.getArgHandle(0));
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return result->getHermesValue();
}

// ES6.0 25.3.3.4.
// Placed separately from generatorPrototypeReturnOrThrow for readability
// and for simpler comparison to the spec.
static CallResult<Handle<JSObject>> generatorResumeAbrupt(
    Runtime &runtime,
    Handle<GeneratorInnerFunction> generator,
    Handle<> value,
    bool isThrow) {
  if (generator->getState() == GeneratorInnerFunction::State::SuspendedStart) {
    // If state is "suspendedStart", then
    // Set generator’s [[GeneratorState]] internal slot to "completed".
    generator->setState(GeneratorInnerFunction::State::Completed);
  }

  if (generator->getState() == GeneratorInnerFunction::State::Completed) {
    if (!isThrow) {
      // abruptCompletion.[[type]] is return.
      return createIterResultObject(runtime, value, true);
    }
    runtime.setThrownValue(*value);
    return ExecutionStatus::EXCEPTION;
  }

  // Assert: state is "suspendedYield".
  assert(
      generator->getState() == GeneratorInnerFunction::State::SuspendedYield);

  auto action = isThrow ? GeneratorInnerFunction::Action::Throw
                        : GeneratorInnerFunction::Action::Return;
  auto valueRes = GeneratorInnerFunction::callInnerFunction(
      generator, runtime, value, action);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    generator->setState(GeneratorInnerFunction::State::Completed);
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(generator->isDelegated())) {
    generator->setIsDelegated(false);
    return runtime.makeHandle<JSObject>(std::move(*valueRes));
  }
  return createIterResultObject(
      runtime,
      runtime.makeHandle(std::move(*valueRes)),
      generator->getState() == GeneratorInnerFunction::State::Completed);
}

CallResult<HermesValue>
generatorPrototypeReturnOrThrow(void *ctx, Runtime &runtime, NativeArgs args) {
  bool isThrow = static_cast<bool>(ctx);

  auto generatorRes = generatorValidate(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(generatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto result = generatorResumeAbrupt(
      runtime,
      runtime.makeHandle(
          JSGenerator::getInnerFunction(runtime, generatorRes->get())),
      args.getArgHandle(0),
      isThrow);
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return result->getHermesValue();
}

} // namespace vm
} // namespace hermes
