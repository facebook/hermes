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

Handle<NativeConstructor> createSymbolConstructor(Runtime &runtime) {
  auto symbolPrototype = Handle<JSObject>::vmcast(&runtime.symbolPrototype);

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::Symbol),
      symbolConstructor,
      symbolPrototype,
      0);

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

  struct : public Locals {
    PinnedValue<SymbolID> symbolHandle;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolHasInstance);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::hasInstance),
      lv.symbolHandle,
      dpf);
  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolIterator);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::iterator),
      lv.symbolHandle,
      dpf);
  lv.symbolHandle =
      Predefined::getSymbolID(Predefined::SymbolIsConcatSpreadable);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isConcatSpreadable),
      lv.symbolHandle,
      dpf);
  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolToPrimitive);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::toPrimitive),
      lv.symbolHandle,
      dpf);
  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolToStringTag);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::toStringTag),
      lv.symbolHandle,
      dpf);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolMatch);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::match),
      lv.symbolHandle,
      dpf);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolMatchAll);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::matchAll),
      lv.symbolHandle,
      dpf);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolSearch);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::search),
      lv.symbolHandle,
      dpf);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolReplace);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::replace),
      lv.symbolHandle,
      dpf);

  lv.symbolHandle = Predefined::getSymbolID(Predefined::SymbolSplit);
  defineProperty(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::split),
      lv.symbolHandle,
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

CallResult<HermesValue> symbolConstructor(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  if (args.isConstructorCall()) {
    return runtime.raiseTypeError("Symbol is not a constructor");
  }

  struct : public Locals {
    PinnedValue<StringPrimitive> descString;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  if (args.getArg(0).isUndefined()) {
    // If description is undefined, the descString will eventually be "".
    lv.descString = runtime.getPredefinedString(Predefined::emptyString);
  } else {
    auto descStringRes = toString_RJS(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(descStringRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.descString = descStringRes->get();
  }

  auto symbolRes = runtime.getIdentifierTable().createNotUniquedSymbol(
      runtime, lv.descString);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeSymbolValue(*symbolRes);
}

CallResult<HermesValue> symbolFor(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  auto cr = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  struct : public Locals {
    PinnedValue<StringPrimitive> key;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  lv.key = std::move(*cr);
  auto symbolRes = runtime.getSymbolRegistry().getSymbolForKey(runtime, lv.key);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeSymbolValue(*symbolRes);
}

CallResult<HermesValue> symbolKeyFor(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
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
CallResult<HermesValue> symbolPrototypeDescriptionGetter(
    void *,
    Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  struct : public Locals {
    PinnedValue<SymbolID> sym;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  // 1. Let s be the this value.
  // 2. Let sym be ? thisSymbolValue(s).
  if (args.getThisArg().isSymbol()) {
    lv.sym = args.vmcastThis<SymbolID>().get();
  } else if (auto symHandle = args.dyncastThis<JSSymbol>()) {
    lv.sym = symHandle->getPrimitiveSymbol();
  } else {
    return runtime.raiseTypeError(
        "Symbol.prototype.description can only be called on Symbol");
  }

  // 3. Return sym.[[Description]].
  StringPrimitive *desc = runtime.getStringPrimFromSymbolID(lv.sym.get());
  return HermesValue::encodeStringValue(desc);
}

/// ES10 19.4.3.3 Symbol.prototype.toString ( )
CallResult<HermesValue> symbolPrototypeToString(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  // 1. Let sym be ? thisSymbolValue(this value).
  struct : public Locals {
    PinnedValue<SymbolID> sym;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  if (args.getThisArg().isSymbol()) {
    lv.sym = args.vmcastThis<SymbolID>().get();
  } else if (auto symHandle = args.dyncastThis<JSSymbol>()) {
    lv.sym = symHandle->getPrimitiveSymbol();
  } else {
    return runtime.raiseTypeError(
        "Symbol.prototype.toString can only be called on Symbol");
  }

  // 2. Return SymbolDescriptiveString(sym).
  auto str = symbolDescriptiveString(runtime, lv.sym);
  if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return str->getHermesValue();
}

/// ES10 19.4.3.4 Symbol.prototype.valueOf ( )
CallResult<HermesValue> symbolPrototypeValueOf(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
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
