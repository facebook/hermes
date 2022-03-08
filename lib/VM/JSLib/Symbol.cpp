/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

Handle<JSObject> createSymbolConstructor(Runtime &runtime) {
  auto symbolPrototype = Handle<JSObject>::vmcast(&runtime.symbolPrototype);

  auto cons = defineSystemConstructor<JSSymbol>(
      runtime,
      Predefined::getSymbolID(Predefined::Symbol),
      symbolConstructor,
      symbolPrototype,
      0,
      CellKind::JSSymbolKind);

  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::predefinedFor),
      nullptr,
      symbolFor,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::keyFor),
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
      Predefined::getSymbolID(Predefined::hasInstance),
      runtime.makeHandle(
          Predefined::getSymbolID(Predefined::SymbolHasInstance)),
      dpf);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::iterator),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolIterator)),
      dpf);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isConcatSpreadable),
      runtime.makeHandle(
          Predefined::getSymbolID(Predefined::SymbolIsConcatSpreadable)),
      dpf);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::toPrimitive),
      runtime.makeHandle(
          Predefined::getSymbolID(Predefined::SymbolToPrimitive)),
      dpf);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::toStringTag),
      runtime.makeHandle(
          Predefined::getSymbolID(Predefined::SymbolToStringTag)),
      dpf);

  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::match),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolMatch)),
      dpf);

  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::matchAll),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolMatchAll)),
      dpf);

  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::search),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolSearch)),
      dpf);

  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::replace),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolReplace)),
      dpf);

  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::split),
      runtime.makeHandle(Predefined::getSymbolID(Predefined::SymbolSplit)),
      dpf);

  // Symbol.prototype.xxx methods.
  void *ctx = nullptr;
  defineAccessor(
      runtime,
      symbolPrototype,
      Predefined::getSymbolID(Predefined::description),
      ctx,
      symbolPrototypeDescriptionGetter,
      nullptr,
      false,
      true);
  defineMethod(
      runtime,
      symbolPrototype,
      Predefined::getSymbolID(Predefined::toString),
      ctx,
      symbolPrototypeToString,
      0);
  defineMethod(
      runtime,
      symbolPrototype,
      Predefined::getSymbolID(Predefined::valueOf),
      ctx,
      symbolPrototypeValueOf,
      0);

  dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      symbolPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::Symbol),
      dpf);
  (void)defineMethod(
      runtime,
      symbolPrototype,
      Predefined::getSymbolID(Predefined::SymbolToPrimitive),
      Predefined::getSymbolID(Predefined::squareSymbolToPrimitive),
      nullptr,
      symbolPrototypeValueOf,
      1,
      dpf);

  return cons;
}

CallResult<HermesValue>
symbolConstructor(void *, Runtime &runtime, NativeArgs args) {
  if (args.isConstructorCall()) {
    return runtime.raiseTypeError("Symbol is not a constructor");
  }

  MutableHandle<StringPrimitive> descString{runtime};
  if (args.getArg(0).isUndefined()) {
    // If description is undefined, the descString will eventually be "".
    descString = runtime.getPredefinedString(Predefined::emptyString);
  } else {
    auto descStringRes = toString_RJS(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(descStringRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    descString = descStringRes->get();
  }

  auto symbolRes =
      runtime.getIdentifierTable().createNotUniquedSymbol(runtime, descString);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeSymbolValue(*symbolRes);
}

CallResult<HermesValue> symbolFor(void *, Runtime &runtime, NativeArgs args) {
  auto cr = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto key = runtime.makeHandle(std::move(*cr));

  auto symbolRes = runtime.getSymbolRegistry().getSymbolForKey(runtime, key);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeSymbolValue(*symbolRes);
}

CallResult<HermesValue>
symbolKeyFor(void *, Runtime &runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(!args.getArg(0).isSymbol())) {
    return runtime.raiseTypeError("Symbol.keyFor() requires a symbol argument");
  }

  auto sym = Handle<SymbolID>::vmcast(args.getArgHandle(0));

  if (runtime.getSymbolRegistry().hasSymbol(sym.get())) {
    return HermesValue::encodeStringValue(
        runtime.getStringPrimFromSymbolID(*sym));
  }

  return HermesValue::encodeUndefinedValue();
}

/// ES10.0 19.4.3.2 get Symbol.prototype.description
/// TODO(T79770380): make the Symbol(undefined) case spec-conformant.
CallResult<HermesValue>
symbolPrototypeDescriptionGetter(void *, Runtime &runtime, NativeArgs args) {
  MutableHandle<SymbolID> sym{runtime};
  // 1. Let s be the this value.
  // 2. Let sym be ? thisSymbolValue(s).
  if (args.getThisArg().isSymbol()) {
    sym = args.vmcastThis<SymbolID>().get();
  } else if (auto symHandle = args.dyncastThis<JSSymbol>()) {
    sym = symHandle->getPrimitiveSymbol();
  } else {
    return runtime.raiseTypeError(
        "Symbol.prototype.description can only be called on Symbol");
  }

  // 3. Return sym.[[Description]].
  StringPrimitive *desc = runtime.getStringPrimFromSymbolID(*sym);
  return HermesValue::encodeStringValue(desc);
}

/// ES10 19.4.3.3 Symbol.prototype.toString ( )
CallResult<HermesValue>
symbolPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let sym be ? thisSymbolValue(this value).
  MutableHandle<SymbolID> sym{runtime};
  if (args.getThisArg().isSymbol()) {
    sym = args.vmcastThis<SymbolID>().get();
  } else if (auto symHandle = args.dyncastThis<JSSymbol>()) {
    sym = symHandle->getPrimitiveSymbol();
  } else {
    return runtime.raiseTypeError(
        "Symbol.prototype.toString can only be called on Symbol");
  }

  // 2. Return SymbolDescriptiveString(sym).
  auto str = symbolDescriptiveString(runtime, sym);
  if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return str->getHermesValue();
}

/// ES10 19.4.3.4 Symbol.prototype.valueOf ( )
CallResult<HermesValue>
symbolPrototypeValueOf(void *, Runtime &runtime, NativeArgs args) {
  // 1. Return ? thisSymbolValue(this value).
  if (args.getThisArg().isSymbol()) {
    return args.getThisArg();
  }
  if (auto jsSymbol = args.dyncastThis<JSSymbol>()) {
    return jsSymbol->getPrimitiveSymbol().getHermesValue();
  }
  return runtime.raiseTypeError(
      "Symbol.prototype.valueOf can only be called on Symbol");
}

} // namespace vm
} // namespace hermes
