/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "JSLibInternal.h"

#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

Handle<NativeConstructor> defineSystemConstructor(
    Runtime *runtime,
    SymbolID name,
    NativeFunctionPtr nativeFunctionPtr,
    Handle<JSObject> prototypeObjectHandle,
    Handle<JSObject> constructorProtoObjectHandle,
    unsigned paramCount,
    NativeConstructor::CreatorFunction *creator,
    CellKind targetKind) {
  auto constructor = toHandle(
      runtime,
      NativeConstructor::create(
          runtime,
          constructorProtoObjectHandle,
          nullptr,
          nativeFunctionPtr,
          paramCount,
          creator,
          targetKind));

  auto st = Callable::defineNameLengthAndPrototype(
      constructor,
      runtime,
      name,
      paramCount,
      prototypeObjectHandle,
      Callable::WritablePrototype::No,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  // Define the global.
  DefinePropertyFlags dpf{};

  dpf.setEnumerable = 1;
  dpf.setWritable = 1;
  dpf.setConfigurable = 1;
  dpf.setValue = 1;
  dpf.enumerable = 0;
  dpf.writable = 1;
  dpf.configurable = 1;

  auto res = JSObject::defineOwnProperty(
      runtime->getGlobal(), runtime, name, dpf, constructor);
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
  (void)res;

  return constructor;
}

CallResult<HermesValue> defineMethod(
    Runtime *runtime,
    Handle<JSObject> objectHandle,
    SymbolID propertyName,
    SymbolID methodName,
    void *context,
    NativeFunctionPtr nativeFunctionPtr,
    unsigned paramCount,
    DefinePropertyFlags dpf) {
  GCScope gcScope{runtime};

  auto method = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime->functionPrototype),
      context,
      nativeFunctionPtr,
      methodName,
      paramCount,
      runtime->makeNullHandle<JSObject>());

  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, propertyName, dpf, method);
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");

  return method.getHermesValue();
}

Handle<NativeConstructor> defineSystemConstructor(
    Runtime *runtime,
    SymbolID name,
    NativeFunctionPtr nativeFunctionPtr,
    Handle<JSObject> prototypeObjectHandle,
    unsigned paramCount,
    NativeConstructor::CreatorFunction *creator,
    CellKind targetKind) {
  return defineSystemConstructor(
      runtime,
      name,
      nativeFunctionPtr,
      prototypeObjectHandle,
      Handle<JSObject>::vmcast(&runtime->functionPrototype),
      paramCount,
      creator,
      targetKind);
}

void defineMethod(
    Runtime *runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    void *context,
    NativeFunctionPtr nativeFunctionPtr,
    unsigned paramCount) {
  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setWritable = 1;
  dpf.setConfigurable = 1;
  dpf.setValue = 1;
  dpf.enumerable = 0;
  dpf.writable = 1;
  dpf.configurable = 1;
  (void)defineMethod(
      runtime, objectHandle, name, context, nativeFunctionPtr, paramCount, dpf);
}

void defineAccessor(
    Runtime *runtime,
    Handle<JSObject> objectHandle,
    SymbolID propertyName,
    SymbolID methodName,
    void *context,
    NativeFunctionPtr getterFunc,
    NativeFunctionPtr setterFunc,
    bool enumerable,
    bool configurable) {
  assert(
      (getterFunc || setterFunc) &&
      "at least a getter or a setter must be specified");
  ExecutionStatus status{};
  (void)status;

  GCScope gcScope{runtime};

  StringView nameView =
      runtime->getIdentifierTable().getStringView(runtime, methodName);
  assert(nameView.isASCII() && "Only ASCII accessors are supported");

  MutableHandle<NativeFunction> getter{runtime};
  if (getterFunc) {
    // Set the name by prepending "get ".
    llvm::SmallString<32> getterName{"get "};
    llvm::raw_svector_ostream os{getterName};
    os << nameView;

    auto strRes = runtime->ignoreAllocationFailure(
        StringPrimitive::create(runtime, getterName));
    SymbolID getterFuncName =
        runtime
            ->ignoreAllocationFailure(
                runtime->getIdentifierTable().getSymbolHandleFromPrimitive(
                    runtime,
                    createPseudoHandle(vmcast<StringPrimitive>(strRes))))
            .get();

    auto funcRes = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        context,
        getterFunc,
        getterFuncName,
        0,
        runtime->makeNullHandle<JSObject>());
    getter = funcRes.get();
  }

  MutableHandle<NativeFunction> setter{runtime};
  if (setterFunc) {
    // Set the name by prepending "set ".
    llvm::SmallString<32> setterName{"set "};
    llvm::raw_svector_ostream os{setterName};
    os << nameView;

    auto strRes = runtime->ignoreAllocationFailure(
        StringPrimitive::create(runtime, setterName));
    SymbolID setterFuncName =
        runtime
            ->ignoreAllocationFailure(
                runtime->getIdentifierTable().getSymbolHandleFromPrimitive(
                    runtime,
                    createPseudoHandle(vmcast<StringPrimitive>(strRes))))
            .get();

    auto funcRes = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->functionPrototype),
        context,
        setterFunc,
        setterFuncName,
        1,
        runtime->makeNullHandle<JSObject>());
    setter = funcRes.get();
  }

  auto crtRes = PropertyAccessor::create(runtime, getter, setter);
  assert(crtRes != ExecutionStatus::EXCEPTION && "unable to define accessor");
  auto accessor = runtime->makeHandle<PropertyAccessor>(*crtRes);

  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setConfigurable = 1;
  dpf.setGetter = 1;
  dpf.setSetter = 1;
  dpf.enumerable = enumerable;
  dpf.configurable = configurable;

  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, propertyName, dpf, accessor);
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
}

void defineProperty(
    Runtime *runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    Handle<> value,
    DefinePropertyFlags dpf) {
  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, name, dpf, value, PropOpFlags());
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
}

void defineProperty(
    Runtime *runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    Handle<> value) {
  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.enumerable = 0;
  dpf.setConfigurable = 1;
  dpf.configurable = 1;
  dpf.setWritable = 1;
  dpf.writable = 1;
  dpf.setValue = 1;

  return defineProperty(runtime, objectHandle, name, value, dpf);
}

ExecutionStatus iteratorCloseAndRethrow(
    Runtime *runtime,
    Handle<JSObject> iterator) {
  auto completion = runtime->makeHandle(runtime->getThrownValue());
  runtime->clearThrownValue();
  auto status = iteratorClose(runtime, iterator, completion);
  (void)status;
  assert(
      status == ExecutionStatus::EXCEPTION && "exception swallowed mistakenly");
  return ExecutionStatus::EXCEPTION;
}

} // namespace vm
} // namespace hermes
