/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.12 Populate the JSON object.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"
#include "hermes/VM/JSLib/RuntimeJSONUtils.h"
#include "hermes/VM/JSLib/RuntimeFastJSONUtils.h"
#include "hermes/VM/SingleObject.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

Handle<JSObject> createJSONObject(Runtime &runtime) {
  auto objRes = JSJSON::create(
      runtime, Handle<JSObject>::vmcast(&runtime.objectPrototype));
  assert(
      objRes != ExecutionStatus::EXCEPTION && "unable to define JSON object");
  auto json = runtime.makeHandle<JSJSON>(*objRes);

  defineMethod(
      runtime,
      json,
      Predefined::getSymbolID(Predefined::parse),
      nullptr,
      jsonParse,
      2);
  defineMethod(
      runtime,
      json,
      Predefined::getSymbolID(Predefined::fastParse),
      nullptr,
      jsonFastParse,
      2);
  defineMethod(
      runtime,
      json,
      Predefined::getSymbolID(Predefined::debugToAscii),
      nullptr,
      jsonDebugToAscii,
      1);
  defineMethod(
      runtime,
      json,
      Predefined::getSymbolID(Predefined::stringify),
      nullptr,
      jsonStringify,
      3);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      json,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::JSON),
      dpf);

  return json;
}

CallResult<HermesValue> jsonParse(void *, Runtime &runtime, NativeArgs args) {
  auto res = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return runtimeJSONParse(
      runtime,
      runtime.makeHandle(std::move(*res)),
      Handle<Callable>::dyn_vmcast(args.getArgHandle(1)));
}

CallResult<HermesValue> jsonFastParse(void *, Runtime &runtime, NativeArgs args) {
  auto res = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return runtimeFastJSONParse(
      runtime,
      runtime.makeHandle(std::move(*res)),
      Handle<Callable>::dyn_vmcast(args.getArgHandle(1)));
}

// This ensures that passed string is encoded in ASCII. This simulates what would realistically
// happen if string was created via jsi::String::createFromUtf8 / ::createFromAscii (but doesn't happen
// in the demo because we're just concatenating JS strings)
CallResult<HermesValue> jsonDebugToAscii(void *, Runtime &runtime, NativeArgs args) {
  auto res = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto str = runtime.makeHandle(std::move(*res));

  if (str->isASCII()) {
    return str.getHermesValue();
  }

  SmallU16String<32> storage;
  StringPrimitive::createStringView(runtime, str)
        .appendUTF16String(storage);

  auto utf16 = UTF16Ref{storage.begin(), storage.size()};
  auto strEfficient = StringPrimitive::createEfficient(runtime, utf16);
  if (LLVM_UNLIKELY(strEfficient == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!vmcast<StringPrimitive>(*strEfficient)->isASCII())) {
    return runtime.raiseRangeError("String is not actually ASCII");
  }
  return strEfficient;
}

CallResult<HermesValue>
jsonStringify(void *, Runtime &runtime, NativeArgs args) {
  return runtimeJSONStringify(
      runtime,
      args.getArgHandle(0),
      args.getArgHandle(1),
      args.getArgHandle(2));
}

} // namespace vm
} // namespace hermes
