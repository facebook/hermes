/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

namespace hermes {
namespace vm {

Handle<JSObject> createTextEncoderConstructor(Runtime &runtime) {
  auto textEncoderPrototype =
      Handle<JSObject>::vmcast(&runtime.textEncoderPrototype);

  // Per https://webidl.spec.whatwg.org/#javascript-binding, @@toStringTag
  // should be writable=false, enumerable=false, and configurable=true.
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();
  dpf.writable = 0;
  defineProperty(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::TextEncoder),
      dpf);

  // Based on
  // Object.getOwnPropertyDescriptor(TextEncoder.prototype, 'encoding'), both
  // Chrome and Safari have the 'encoding' property as enumerable and
  // configurable. We set things up to be the same.
  defineAccessor(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::encoding),
      nullptr,
      textEncoderPrototypeEncoding,
      nullptr,
      /* enumerable */ true,
      /* configurable */ true);

  auto cons = defineSystemConstructor<JSObject>(
      runtime,
      Predefined::getSymbolID(Predefined::TextEncoder),
      textEncoderConstructor,
      textEncoderPrototype,
      0,
      CellKind::JSObjectKind);

  defineProperty(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue>
textEncoderConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError(
        "TextEncoder must be called as a constructor");
  }

  auto selfHandle = args.vmcastThis<JSObject>();

  auto valueHandle = Runtime::getUndefinedValue();
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextEncoderType),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              valueHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return selfHandle.getHermesValue();
}

CallResult<HermesValue>
textEncoderPrototypeEncoding(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();

  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      selfHandle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextEncoderType),
      desc);
  if (LLVM_UNLIKELY(!exists)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encoding called on non-TextEncoder object");
  }

  return HermesValue::encodeStringValue(
      runtime.getPredefinedString(Predefined::utf8));
}

} // namespace vm
} // namespace hermes
