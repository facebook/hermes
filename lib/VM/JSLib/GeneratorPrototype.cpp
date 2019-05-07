/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

/// ES6.0 25.3.1.2.
static CallResult<HermesValue>
generatorPrototypeNext(void *, Runtime *runtime, NativeArgs args);

void populateGeneratorPrototype(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->generatorPrototype);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::next),
      nullptr,
      generatorPrototypeNext,
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
      Handle<>(&runtime->generatorFunctionPrototype),
      dpf);

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::Generator),
      dpf);
}

// ES6.0 25.3.3.2.
static CallResult<Handle<JSGenerator>> generatorValidate(
    Runtime *runtime,
    Handle<> value) {
  auto generator = Handle<JSGenerator>::dyn_vmcast(runtime, value);
  if (!generator) {
    return runtime->raiseTypeError(
        "Generator functions must be called on generators");
  }

  if (JSGenerator::getInnerFunction(*generator)->getState() ==
      GeneratorInnerFunction::State::Executing) {
    return runtime->raiseTypeError(
        "Generator functions may not be called on executing generators");
  }

  return generator;
}

// ES6.0 25.3.3.3.
// Placed separately from generatorPrototypeNext for readability and for simpler
// comparison to the spec.
static CallResult<Handle<JSObject>> generatorResume(
    Runtime *runtime,
    Handle<GeneratorInnerFunction> generator,
    Handle<> value) {
  if (generator->getState() == GeneratorInnerFunction::State::Completed) {
    return createIterResultObject(runtime, runtime->getUndefinedValue(), true);
  }
  auto valueRes = GeneratorInnerFunction::callInnerFunction(
      generator, runtime, value, GeneratorInnerFunction::Action::Next);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    generator->setState(GeneratorInnerFunction::State::Completed);
    return ExecutionStatus::EXCEPTION;
  }
  return createIterResultObject(
      runtime,
      runtime->makeHandle(*valueRes),
      generator->getState() == GeneratorInnerFunction::State::Completed);
}

static CallResult<HermesValue>
generatorPrototypeNext(void *, Runtime *runtime, NativeArgs args) {
  auto generatorRes = generatorValidate(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(generatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto result = generatorResume(
      runtime,
      toHandle(runtime, JSGenerator::getInnerFunction(generatorRes->get())),
      args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return result->getHermesValue();
}

} // namespace vm
} // namespace hermes
