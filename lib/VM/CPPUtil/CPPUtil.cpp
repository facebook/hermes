#include "hermes/VM/CPPUtil/CPPUtil.h"

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm;

namespace hermes {
namespace vm {

double makeDouble(uint64_t n) {
  double d;
  ::memcpy(&d, &n, sizeof(d));
  return d;
}

ExecutionStatus putGetterSetter(
    Runtime *runtime,
    Handle<JSObject> object,
    SymbolID name,
    Handle<> getterHandle,
    Handle<> setterHandle) {
  PropertyFlags pf{};
  pf.enumerable = 1;
  pf.configurable = 1;
  Handle<Callable> getter = runtime->makeNullHandle<Callable>();
  Handle<Callable> setter = runtime->makeNullHandle<Callable>();
  if (LLVM_LIKELY(!getterHandle->isUndefined())) {
    pf.accessor = 1;
    getter = Handle<Callable>::vmcast(getterHandle);
  }
  if (LLVM_LIKELY(!setterHandle->isUndefined())) {
    pf.accessor = 1;
    setter = Handle<Callable>::vmcast(setterHandle);
  }
  auto crtRes = PropertyAccessor::create(runtime, getter, setter);
  if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto accessor = runtime->makeHandle<PropertyAccessor>(*crtRes);

  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(object, runtime, name, pf, accessor) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue>
getIsIn(Runtime *runtime, Handle<> name, Handle<> object) {
  if (LLVM_UNLIKELY(!object->isObject())) {
    return runtime->raiseTypeError("right operand of 'in' is not an object");
  }
  MutableHandle<JSObject> inObject{runtime};
  ComputedPropertyDescriptor desc;
  if (JSObject::getComputedDescriptor(
          Handle<JSObject>::vmcast(object),
          runtime,
          Handle<>(name),
          inObject,
          desc) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(!!inObject);
}

} // namespace vm
} // namespace hermes
