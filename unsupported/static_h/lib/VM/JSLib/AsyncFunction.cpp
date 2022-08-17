/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES12 26.7.1 The AsyncFunction constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

namespace hermes {
namespace vm {

/// 26.7.1.1 AsyncFunction ( p1, p2, … , pn, body )
CallResult<HermesValue>
asyncFunctionConstructor(void *, Runtime &runtime, NativeArgs args) {
  /// 3. Return CreateDynamicFunction(C, NewTarget, async, args).
  return createDynamicFunction(runtime, args, DynamicFunctionKind::Async);
}

Handle<JSObject> createAsyncFunctionConstructor(Runtime &runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime.asyncFunctionPrototype);

  /// 26.7.2 Properties of the AsyncFunction Constructor
  /// has a [[Prototype]] internal slot whose value is %Function%.
  auto cons = runtime.makeHandle(NativeConstructor::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionConstructor),
      nullptr,
      asyncFunctionConstructor,
      1,
      NativeConstructor::creatorFunction<JSAsyncFunction>,
      CellKind::JSAsyncFunctionKind));

  /// has a "name" property whose value is "AsyncFunction".
  /// 26.7.2.1 AsyncFunction.length
  /// 26.7.2.2 AsyncFunction.prototype
  auto st = Callable::defineNameLengthAndPrototype(
      cons,
      runtime,
      Predefined::getSymbolID(Predefined::AsyncFunction),
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

  /// 26.7.3 Properties of the AsyncFunction Prototype Object

  /// 26.7.3.1 AsyncFunction.prototype.constructor
  /// The initial value of is %AsyncFunction%.
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::constructor),
      cons,
      dpf);

  /// 26.7.3.2 AsyncFunction.prototype [ @@toStringTag ]
  /// The initial value is the String value "AsyncFunction".
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::AsyncFunction),
      dpf);

  return cons;
}

} // namespace vm
} // namespace hermes
