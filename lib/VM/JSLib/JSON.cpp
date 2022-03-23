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
#include "hermes/VM/SingleObject.h"
#include "hermes/VM/StringPrimitive.h"

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
