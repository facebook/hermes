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
      reinterpret_cast<void *>(&BigIntPrimitive::asIntN),
      bigintTruncate,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::asUintN),
      reinterpret_cast<void *>(&BigIntPrimitive::asUintN),
      bigintTruncate,
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
  auto bigint = thisBigIntValue(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(bigint == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Call toString, as JSC does.
  // TODO(T120187933): Format string according to locale.
  auto res = toString_RJS(runtime, runtime.makeHandle(*bigint));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return res->getHermesValue();
}

CallResult<HermesValue>
bigintPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let x be ? thisBigIntValue(this value).
  auto x = thisBigIntValue(runtime, args.getThisHandle());

  if (LLVM_UNLIKELY(x == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  Handle<BigIntPrimitive> xHandle = runtime.makeHandle(x->getBigInt());

  // 2. If radix is undefined, let radixMV be 10.
  uint32_t radixMV = 10;

  // 3. Else, let radixMV be ? ToIntegerOrInfinity(radix).
  auto radixArg = args.getArgHandle(0);
  if (!radixArg->isUndefined()) {
    auto r = toIntegerOrInfinity(runtime, radixArg);
    if (LLVM_UNLIKELY(r == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    assert(r->isNumber());
    // 4. If radixMV < 2 or radixMV > 36, throw a RangeError exception.
    if (r->getNumber() < 2 || r->getNumber() > 36) {
      return runtime.raiseRangeError(
          "radix out-of-range in BigInt.prototype.toString");
    }

    radixMV = r->getNumber();
  }

  // 5. If radixMV = 10, return ! ToString(x).
  // 6. Return the String representation of x.
  return xHandle->toString(runtime, radixMV);
}

CallResult<HermesValue>
bigintPrototypeValueOf(void *, Runtime &runtime, NativeArgs args) {
  return thisBigIntValue(runtime, args.getThisHandle());
}

using TruncateOp =
    CallResult<HermesValue> (*)(Runtime &, uint64_t, Handle<BigIntPrimitive>);

CallResult<HermesValue>
bigintTruncate(void *ctx, Runtime &runtime, NativeArgs args) {
  auto bitsRes = toIndex(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(bitsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint64_t bits = bitsRes->getNumberAs<uint64_t>();

  auto bigint = toBigInt_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(bigint == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto op = reinterpret_cast<TruncateOp>(ctx);
  return (*op)(runtime, bits, runtime.makeHandle(bigint->getBigInt()));
}

} // namespace vm
} // namespace hermes
