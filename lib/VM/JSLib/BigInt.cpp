/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// The BigInt constructor (https://tc39.es/ecma262/#sec-bigint-constructor).
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"

namespace hermes {
namespace vm {

Handle<JSObject> createBigIntConstructor(Runtime &runtime) {
  auto bigintPrototype = Handle<JSBigInt>::vmcast(&runtime.bigintPrototype);

  auto cons = defineSystemConstructor<JSBigInt>(
      runtime,
      Predefined::getSymbolID(Predefined::BigInt),
      bigintConstructor,
      bigintPrototype,
      1,
      CellKind::JSBigIntKind);

  // BigInt.prototype.xxx methods.
  // https://tc39.es/ecma262/#sec-properties-of-the-bigint-prototype-object
  void *ctx = nullptr;
  defineMethod(
      runtime,
      bigintPrototype,
      Predefined::getSymbolID(Predefined::toString),
      ctx,
      bigintPrototypeToString,
      0);
  defineMethod(
      runtime,
      bigintPrototype,
      Predefined::getSymbolID(Predefined::valueOf),
      ctx,
      bigintPrototypeValueOf,
      0);
  defineMethod(
      runtime,
      bigintPrototype,
      Predefined::getSymbolID(Predefined::toLocaleString),
      ctx,
      bigintPrototypeToLocaleString,
      1);

  // BigInt.xxx() methods.
  // https://tc39.es/ecma262/#sec-properties-of-the-bigint-constructor
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::asIntN),
      ctx,
      bigintAsIntN,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::asUintN),
      ctx,
      bigintAsUintN,
      2);

  // BigInt.xxx properties
  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      bigintPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::BigInt),
      dpf);

  return cons;
}

CallResult<HermesValue>
bigintConstructor(void *, Runtime &runtime, NativeArgs args) {
  // The bigint constructor is not a constructor according to
  // https://262.ecma-international.org/#sec-bigint-constructor
  if (args.isConstructorCall()) {
    return runtime.raiseTypeError("BigInt is not a constructor");
  }

  auto hArg0 = runtime.makeHandle(args.getArg(0));
  auto prim = toPrimitive_RJS(runtime, hArg0, PreferredType::NUMBER);
  if (LLVM_UNLIKELY(prim == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (prim->isNumber()) {
    return numberToBigInt(runtime, prim->getNumber());
  }

  return toBigInt_RJS(runtime, hArg0);
}

CallResult<HermesValue>
bigintPrototypeToLocaleString(void *ctx, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
bigintPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
bigintPrototypeValueOf(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
bigintAsIntN(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
bigintAsUintN(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
