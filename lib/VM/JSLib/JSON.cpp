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

/// @name JSON
/// @{

/// 15.12.2 parse(text[, reviver])
static CallResult<HermesValue>
jsonParse(void *, Runtime *runtime, NativeArgs args);

/// 15.12.3 stringify(value[, replacer[, space]])
static CallResult<HermesValue>
jsonStringify(void *, Runtime *runtime, NativeArgs args);
/// @}

Handle<JSObject> createJSONObject(Runtime *runtime) {
  auto objRes = JSJSON::create(
      runtime, Handle<JSObject>::vmcast(&runtime->objectPrototype));
  assert(
      objRes != ExecutionStatus::EXCEPTION && "unable to define JSON object");
  auto json = runtime->makeHandle<JSJSON>(*objRes);

  defineMethod(
      runtime,
      json,
      runtime->getPredefinedSymbolID(Predefined::parse),
      nullptr,
      jsonParse,
      2);
  defineMethod(
      runtime,
      json,
      runtime->getPredefinedSymbolID(Predefined::stringify),
      nullptr,
      jsonStringify,
      3);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      json,
      runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::JSON),
      dpf);

  return json;
}

static CallResult<HermesValue>
jsonParse(void *, Runtime *runtime, NativeArgs args) {
  auto res = toString(runtime, args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return runtimeJSONParse(
      runtime,
      toHandle(runtime, std::move(*res)),
      Handle<Callable>::dyn_vmcast(runtime, args.getArgHandle(runtime, 1)));
}

static CallResult<HermesValue>
jsonStringify(void *, Runtime *runtime, NativeArgs args) {
  return runtimeJSONStringify(
      runtime,
      args.getArgHandle(runtime, 0),
      args.getArgHandle(runtime, 1),
      args.getArgHandle(runtime, 2));
}

} // namespace vm
} // namespace hermes
