/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

/// @name Boolean
/// @{

/// ES5.1 15.6.1.1 and 15.6.2.1. Boolean() invoked as a function and as a
/// constructor.
static CallResult<HermesValue>
booleanConstructor(void *, Runtime *runtime, NativeArgs args);

/// @}

/// @name Boolean.prototype
/// @{

/// ES5.1 15.6.4.2.
static CallResult<HermesValue>
booleanPrototypeToString(void *, Runtime *runtime, NativeArgs args);

/// ES5.1 15.6.4.3.
static CallResult<HermesValue>
booleanPrototypeValueOf(void *, Runtime *runtime, NativeArgs args);

/// @}

Handle<JSObject> createBooleanConstructor(Runtime *runtime) {
  auto booleanPrototype = Handle<JSBoolean>::vmcast(&runtime->booleanPrototype);

  auto cons = defineSystemConstructor<JSBoolean>(
      runtime,
      Predefined::getSymbolID(Predefined::Boolean),
      booleanConstructor,
      booleanPrototype,
      1,
      CellKind::BooleanObjectKind);

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

static CallResult<HermesValue>
booleanConstructor(void *, Runtime *runtime, NativeArgs args) {
  bool value = toBoolean(args.getArg(0));

  if (args.isConstructorCall()) {
    auto *self = vmcast<JSBoolean>(args.getThisArg());
    JSBoolean::setPrimitiveValue(
        self, runtime, HermesValue::encodeBoolValue(value));
    return args.getThisArg();
  }

  return HermesValue::encodeBoolValue(value);
}

static CallResult<HermesValue>
booleanPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  bool value;
  if (args.getThisArg().isBool()) {
    value = args.getThisArg().getBool();
  } else {
    auto *boolPtr = dyn_vmcast<JSBoolean>(args.getThisArg());
    if (!boolPtr) {
      return runtime->raiseTypeError(
          "Boolean.prototype.valueOf() can only be used on Boolean");
    }
    value = JSBoolean::getPrimitiveValue(boolPtr, runtime).getBool();
  }
  return HermesValue::encodeStringValue(
      value ? runtime->getPredefinedString(Predefined::trueStr)
            : runtime->getPredefinedString(Predefined::falseStr));
}

static CallResult<HermesValue>
booleanPrototypeValueOf(void *, Runtime *runtime, NativeArgs args) {
  if (args.getThisArg().isBool()) {
    return args.getThisArg();
  }
  auto *boolPtr = dyn_vmcast<JSBoolean>(args.getThisArg());
  if (!boolPtr) {
    return runtime->raiseTypeError(
        "Boolean.prototype.valueOf() can only be used on Boolean");
  }
  return JSBoolean::getPrimitiveValue(boolPtr, runtime);
}

} // namespace vm
} // namespace hermes
