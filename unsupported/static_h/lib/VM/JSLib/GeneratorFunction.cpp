/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 25.2.1 The GeneratorFunction constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

namespace hermes {
namespace vm {

Handle<JSObject> createGeneratorFunctionConstructor(Runtime &runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime.generatorFunctionPrototype);

  auto cons = runtime.makeHandle(NativeConstructor::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionConstructor),
      nullptr,
      generatorFunctionConstructor,
      1,
      NativeConstructor::creatorFunction<JSGeneratorFunction>,
      CellKind::JSGeneratorFunctionKind));

  auto st = Callable::defineNameLengthAndPrototype(
      cons,
      runtime,
      Predefined::getSymbolID(Predefined::GeneratorFunction),
      1,
      proto,
      Callable::WritablePrototype::No,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;

  // The initial value of GeneratorFunction.prototype.constructor is the
  // intrinsic object %GeneratorFunction%.
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::constructor),
      cons,
      dpf);

  // The value of GeneratorFunction.prototype.prototype is the
  // %GeneratorPrototype% intrinsic object.
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::prototype),
      Handle<>(&runtime.generatorPrototype),
      dpf);

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::GeneratorFunction),
      dpf);

  return cons;
}

CallResult<HermesValue>
generatorFunctionConstructor(void *, Runtime &runtime, NativeArgs args) {
  return createDynamicFunction(runtime, args, DynamicFunctionKind::Generator);
}

} // namespace vm
} // namespace hermes
