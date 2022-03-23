/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {
#ifdef HERMES_ENABLE_IR_INSTRUMENTATION

/// Simply return undefined. This is used by default pre-hooks that return a
/// dummy cookie.
CallResult<HermesValue> instrumentDoNothing(void *, Runtime &, NativeArgs) {
  return HermesValue::encodeUndefinedValue();
}

/// Simply return the Nth argument. This is used by default post-hooks that
/// return their result argument.
CallResult<HermesValue>
instrumentReturnNth(void *context, Runtime &runtime, NativeArgs args) {
  unsigned argument = (unsigned)(uintptr_t)context;
  return *args.getArgHandle(argument);
}

static void definePre(
    Runtime &runtime,
    Handle<JSObject> object,
    const char *const name,
    int params) {
  auto symbol = runtime.getIdentifierTable().getSymbolHandle(
      runtime, ASCIIRef{name, strlen(name)});
  defineMethod(runtime, object, **symbol, nullptr, instrumentDoNothing, params);
}

static void definePost(
    Runtime &runtime,
    Handle<JSObject> object,
    const char *const name,
    int toReturn,
    int params) {
  assert(toReturn < params);
  auto symbol = runtime.getIdentifierTable().getSymbolHandle(
      runtime, ASCIIRef{name, strlen(name)});
  defineMethod(
      runtime,
      object,
      **symbol,
      (void *)(uintptr_t)toReturn,
      instrumentReturnNth,
      params);
}

Handle<JSObject> createInstrumentObject(Runtime &runtime) {
  auto obj = runtime.makeHandle(JSObject::create(runtime));
  // iid, operator, left, right
  definePre(runtime, obj, "preBinary", 4);
  // iid, cookie, operator, result, left, right
  definePost(runtime, obj, "postBinary", 3, 6);
  // iid, operator, argument
  definePre(runtime, obj, "preUnary", 3);
  // iid, cookie, operator, result, argument
  definePost(runtime, obj, "postUnary", 3, 5);
  // iid, operator, left, right
  definePre(runtime, obj, "preAssignment", 4);
  // iid, cookie, operator, result, left, right
  definePost(runtime, obj, "postAssignment", 3, 6);
  return obj;
}
#endif // HERMES_ENABLE_IR_INSTRUMENTATION
} // namespace vm
} // namespace hermes
