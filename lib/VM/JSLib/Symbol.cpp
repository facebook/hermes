/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 19.4 Initialize the Symbol constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {
/// @name Symbol
/// @{

/// ES6.0 19.4.1. Symbol([description]).
static CallResult<HermesValue>
symbolConstructor(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 19.4.2.1. Symbol.for.
static CallResult<HermesValue>
symbolFor(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 19.4.2.1. Symbol.keyFor.
static CallResult<HermesValue>
symbolKeyFor(void *, Runtime *runtime, NativeArgs args);

/// @}

/// @name Symbol.prototype
/// @{

/// ES6.0 19.4.3.2.
static CallResult<HermesValue>
symbolPrototypeToString(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 19.4.3.3.
static CallResult<HermesValue>
symbolPrototypeValueOf(void *, Runtime *runtime, NativeArgs args);

/// @}

Handle<JSObject> createSymbolConstructor(Runtime *runtime) {
  auto symbolPrototype = Handle<JSObject>::vmcast(&runtime->symbolPrototype);

  auto cons = defineSystemConstructor<JSSymbol>(
      runtime,
      runtime->getPredefinedSymbolID(Predefined::Symbol),
      symbolConstructor,
      symbolPrototype,
      0,
      CellKind::SymbolObjectKind);

  defineMethod(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::predefinedFor),
      nullptr,
      symbolFor,
      1);
  defineMethod(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::keyFor),
      nullptr,
      symbolKeyFor,
      1);

  // Well-known symbols.
  DefinePropertyFlags dpf{};
  dpf.writable = 0;
  dpf.enumerable = 0;
  dpf.configurable = 0;
  dpf.setWritable = 0;
  dpf.setEnumerable = 0;
  dpf.setConfigurable = 0;
  dpf.setValue = 1;

  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::hasInstance),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolHasInstance)),
      dpf);
  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::iterator),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolIterator)),
      dpf);
  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::isConcatSpreadable),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolIsConcatSpreadable)),
      dpf);
  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::toPrimitive),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolToPrimitive)),
      dpf);
  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::toStringTag),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag)),
      dpf);

  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::match),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolMatch)),
      dpf);

  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::search),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolSearch)),
      dpf);

  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::replace),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolReplace)),
      dpf);

  defineProperty(
      runtime,
      cons,
      runtime->getPredefinedSymbolID(Predefined::split),
      runtime->makeHandle(
          runtime->getPredefinedSymbolID(Predefined::SymbolSplit)),
      dpf);

  // Symbol.prototype.xxx methods.
  void *ctx = nullptr;
  defineMethod(
      runtime,
      symbolPrototype,
      runtime->getPredefinedSymbolID(Predefined::toString),
      ctx,
      symbolPrototypeToString,
      0);
  defineMethod(
      runtime,
      symbolPrototype,
      runtime->getPredefinedSymbolID(Predefined::valueOf),
      ctx,
      symbolPrototypeValueOf,
      0);

  dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      symbolPrototype,
      runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::Symbol),
      dpf);
  (void)defineMethod(
      runtime,
      symbolPrototype,
      runtime->getPredefinedSymbolID(Predefined::SymbolToPrimitive),
      runtime->getPredefinedSymbolID(Predefined::squareSymbolToPrimitive),
      nullptr,
      symbolPrototypeValueOf,
      1,
      dpf);

  return cons;
}

static CallResult<HermesValue>
symbolConstructor(void *, Runtime *runtime, NativeArgs args) {
  if (args.isConstructorCall()) {
    return runtime->raiseTypeError("Symbol is not a constructor");
  }

  MutableHandle<StringPrimitive> descString{runtime};
  if (args.getArg(0).isUndefined()) {
    // If description is undefined, the descString will eventually be "".
    descString = runtime->getPredefinedString(Predefined::emptyString);
  } else {
    auto descStringRes = toString(runtime, args.getArgHandle(runtime, 0));
    if (LLVM_UNLIKELY(descStringRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    descString = descStringRes->get();
  }

  auto symbolRes =
      runtime->getIdentifierTable().createNotUniquedSymbol(runtime, descString);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeSymbolValue(*symbolRes);
}

static CallResult<HermesValue>
symbolFor(void *, Runtime *runtime, NativeArgs args) {
  auto cr = toString(runtime, args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto key = toHandle(runtime, std::move(*cr));

  auto symbolRes = runtime->getSymbolRegistry().getSymbolForKey(runtime, key);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeSymbolValue(*symbolRes);
}

static CallResult<HermesValue>
symbolKeyFor(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(!args.getArg(0).isSymbol())) {
    return runtime->raiseTypeError(
        "Symbol.keyFor() requires a symbol argument");
  }

  auto sym = Handle<SymbolID>::vmcast(args.getArgHandle(runtime, 0));

  if (runtime->getSymbolRegistry().hasSymbol(sym.get())) {
    return HermesValue::encodeStringValue(
        runtime->getStringPrimFromSymbolID(*sym));
  }

  return HermesValue::encodeUndefinedValue();
}

static CallResult<HermesValue>
symbolPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  MutableHandle<SymbolID> sym{runtime};
  if (args.getThisArg().isSymbol()) {
    sym = args.vmcastThis<SymbolID>().get();
  } else if (auto symHandle = args.dyncastThis<JSSymbol>(runtime)) {
    sym = JSSymbol::getPrimitiveSymbol(*symHandle).get();
  } else {
    return runtime->raiseTypeError(
        "Symbol.prototype.toString can only be called on Symbol");
  }

  auto str = symbolDescriptiveString(runtime, sym);
  if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return str->getHermesValue();
}

static CallResult<HermesValue>
symbolPrototypeValueOf(void *, Runtime *runtime, NativeArgs args) {
  if (args.getThisArg().isSymbol()) {
    return args.getThisArg();
  }

  if (auto jsSymbol = args.dyncastThis<JSSymbol>(runtime)) {
    return JSSymbol::getPrimitiveSymbol(*jsSymbol).getHermesValue();
  }

  return runtime->raiseTypeError(
      "Symbol.prototype.valueOf can only be called on Symbol");
}

} // namespace vm
} // namespace hermes
