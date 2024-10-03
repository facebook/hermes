/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.7 Initialize the Boolean constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"

namespace hermes {
namespace vm {

Handle<NativeConstructor> createBooleanConstructor(Runtime &runtime) {
  auto booleanPrototype = Handle<JSBoolean>::vmcast(&runtime.booleanPrototype);

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::Boolean),
      booleanConstructor,
      booleanPrototype,
      1);

  // Boolean.prototype.xxx methods.
  defineMethod(
      runtime,
      booleanPrototype,
      Predefined::getSymbolID(Predefined::toString),
      nullptr,
      booleanPrototypeToString,
      0);
  defineMethod(
      runtime,
      booleanPrototype,
      Predefined::getSymbolID(Predefined::valueOf),
      nullptr,
      booleanPrototypeValueOf,
      0);

  return cons;
}

CallResult<HermesValue>
booleanConstructor(void *, Runtime &runtime, NativeArgs args) {
  bool value = toBoolean(args.getArg(0));

  if (!args.isConstructorCall()) {
    return HermesValue::encodeBoolValue(value);
  }

  if (LLVM_LIKELY(
          args.getNewTarget().getRaw() ==
          runtime.booleanConstructor.getHermesValue().getRaw())) {
    return JSBoolean::create(runtime, value, runtime.booleanPrototype)
        .getHermesValue();
  }
  struct : public Locals {
    PinnedValue<JSObject> selfParent;
  } lv;
  LocalsRAII lraii(runtime, &lv);
  CallResult<PseudoHandle<JSObject>> thisParentRes =
      NativeConstructor::parentForNewThis_RJS(
          runtime,
          Handle<Callable>::vmcast(&args.getNewTarget()),
          runtime.booleanPrototype);
  if (LLVM_UNLIKELY(thisParentRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.selfParent = std::move(*thisParentRes);
  return JSBoolean::create(runtime, value, lv.selfParent).getHermesValue();
}

CallResult<HermesValue>
booleanPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  bool value;
  if (args.getThisArg().isBool()) {
    value = args.getThisArg().getBool();
  } else {
    auto *boolPtr = dyn_vmcast<JSBoolean>(args.getThisArg());
    if (!boolPtr) {
      return runtime.raiseTypeError(
          "Boolean.prototype.valueOf() can only be used on Boolean");
    }
    value = boolPtr->getPrimitiveBoolean();
  }
  return HermesValue::encodeStringValue(
      value ? runtime.getPredefinedString(Predefined::trueStr)
            : runtime.getPredefinedString(Predefined::falseStr));
}

CallResult<HermesValue>
booleanPrototypeValueOf(void *, Runtime &runtime, NativeArgs args) {
  if (args.getThisArg().isBool()) {
    return args.getThisArg();
  }
  auto *boolPtr = dyn_vmcast<JSBoolean>(args.getThisArg());
  if (!boolPtr) {
    return runtime.raiseTypeError(
        "Boolean.prototype.valueOf() can only be used on Boolean");
  }
  return HermesValue::encodeBoolValue(boolPtr->getPrimitiveBoolean());
}

} // namespace vm
} // namespace hermes
