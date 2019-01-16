/*
RUN: %hermes -O -target=c++ -dump-bytecode -standalone-c++ %s | %S/Run.sh ~ | %FileCheck --match-full-lines %s
RUN: %hermes -O -target=c++ -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix CPP %s
REQUIRES: cpp
*/

function multi_func_1() {
  return multi_func_2();
}

function multi_func_2() {
  if (multi_func_3()) return 2;
}

function multi_func_3() {
  return 1;
}

print(multi_func_1());
// CHECK: 2

print(Math.round(1.1));
// CHECK: 1

function method() {
  var x = {};
  x["x"] = 2;
  return x.propertyIsEnumerable("x");
}
print(method());
// CHECK: true

function nested_calls() {
  var x = Math.round(Math.max(Math.PI, Math.E));
  return x;
}
print(nested_calls());
// CHECK: 3

function nested_function() {
  function inner() {
    return 8;
  }
  return inner();
}
print(nested_function());
// CHECK: 8

var stop = 0;
function recursion() {
  if (stop) return 5;
  stop = 1;
  return recursion();
}
print(recursion());
// CHECK: 5

function closure() {
  var x = 7;
  function inner() {
    return x;
  }
  return inner();
}
print(closure());
// CHECK: 7

function closure_2() {
  var x = 4;
  function inner() {
    return x;
  }
  return inner;
}
print(closure_2()());
// CHECK: 4

function closure_3() {
  var x = 2;
  function inner() {
    var y = 3;
    function inner_inner() {
      return Math.pow(x, y);
    }
    return Math.imul(inner_inner(), 2);
  }
  return Math.imul(inner(), 2);
}
print(closure_3());
// CHECK: 32

function closure_4() {
  var x = 2;
  function pow() {
    x = Math.pow(x, 2);
  }
  function get() {
    return x;
  }
  var obj = {};
  obj.pow = pow;
  obj.get = get;
  return obj;
}
c = closure_4();
print(c.get());
// CHECK: 2
c.pow();
print(c.get());
// CHECK: 4
c.pow();
print(c.get());
// CHECK: 16

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _1_multi_func_1(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _2_multi_func_2(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _3_multi_func_3(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _4_method(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _5_nested_calls(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _6_nested_function(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _7_inner(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _8_recursion(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _9_closure(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _10_inner(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _11_closure_2(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _12_inner(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _13_closure_3(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _14_inner(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _15_inner_inner(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _16_closure_4(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _17_pow(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _18_get(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(13 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP:   {
// CPP:     DefinePropertyFlags dpf{};
// CPP:     dpf.setWritable = 1;
// CPP:     dpf.setConfigurable = 1;
// CPP:     dpf.setEnumerable = 1;
// CPP:     dpf.writable = 1;
// CPP:     dpf.enumerable = 1;
// CPP:     dpf.configurable = 0;
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_1", static_cast<size_t>(12))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_2", static_cast<size_t>(12))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_3", static_cast<size_t>(12))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("method", static_cast<size_t>(6))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_calls", static_cast<size_t>(12))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_function", static_cast<size_t>(15))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure", static_cast<size_t>(7))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_2", static_cast<size_t>(9))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_3", static_cast<size_t>(9))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:     if (JSObject::defineOwnProperty(
// CPP:             runtime->getGlobal(),
// CPP:             runtime,
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_4", static_cast<size_t>(9))),
// CPP:             dpf,
// CPP:             runtime->getUndefinedValue(),
// CPP:             PropOpFlags().plusThrowOnError()) ==
// CPP:         ExecutionStatus::EXCEPTION) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     marker.flush();
// CPP:   }
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 58);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(0);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(2));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(2)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("multi_func_1", static_cast<size_t>(12)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _1_multi_func_1,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(1);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_1", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_1", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("multi_func_2", static_cast<size_t>(12)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _2_multi_func_2,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(2);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_2", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_2", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("multi_func_3", static_cast<size_t>(12)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _3_multi_func_3,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(3);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_3", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_3", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("method", static_cast<size_t>(6)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _4_method,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(4);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("method", static_cast<size_t>(6)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("method", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("nested_calls", static_cast<size_t>(12)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _5_nested_calls,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(5);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_calls", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_calls", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("nested_function", static_cast<size_t>(15)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _6_nested_function,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(6);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_function", static_cast<size_t>(15)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_function", static_cast<size_t>(15)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("recursion", static_cast<size_t>(9)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _8_recursion,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(7);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("closure", static_cast<size_t>(7)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _9_closure,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(8);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure", static_cast<size_t>(7)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure", static_cast<size_t>(7)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("closure_2", static_cast<size_t>(9)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _11_closure_2,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(9);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_2", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_2", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("closure_3", static_cast<size_t>(9)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _13_closure_3,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(3) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(10);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(3));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_3", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(3));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(3)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_3", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("closure_4", static_cast<size_t>(9)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _16_closure_4,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(11);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_4", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(1));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(1)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_4", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(12);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(13);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_1", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_1", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(14);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(15);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(5).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(5));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(16);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("round", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("round", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(5)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(makeDouble(4607632778762754458u));
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(5);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(17);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(18);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("method", static_cast<size_t>(6)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("method", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(19);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(20);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_calls", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_calls", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(21);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(22);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_function", static_cast<size_t>(15)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested_function", static_cast<size_t>(15)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(23);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(1));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(1)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(24);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(25);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(26);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(27);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure", static_cast<size_t>(7)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure", static_cast<size_t>(7)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(28);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(29);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_2", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_2", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(30);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(31);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_3", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_3", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(32);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_4", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("closure_4", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(33);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(1));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(1)),
// CPP:             1 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(34);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(35);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(4).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(4));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(36);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(4)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(4);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(37);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(38);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(39);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(40);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(4).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(4));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(41);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(4)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(4);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(42);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(43);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(44);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(45);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(46);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(0) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(0) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(0);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(7));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _1_multi_func_1(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(8 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(47);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_2", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_2", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(2));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _2_multi_func_2(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(8 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(48);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_3", static_cast<size_t>(12)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_func_3", static_cast<size_t>(12)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(2));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(1))) { goto L2; } else { goto L1; }
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   return frame.getLocalVarRef(0);
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _3_multi_func_3(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _4_method(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(10 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(2) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(49);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(0));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(0));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(0)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(50);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("propertyIsEnumerable", static_cast<size_t>(20)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(4) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(4));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(3);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _5_nested_calls(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(14 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(51);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(52);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("round", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("round", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(53);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(5).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(5));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(54);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("max", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("max", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(5)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(55);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(56);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(7) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("PI", static_cast<size_t>(2)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(7) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(7) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("PI", static_cast<size_t>(2)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(7) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(57);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(0) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(0) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(58);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(6) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("E", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("E", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = frame.getLocalVarRef(5);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(8));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(7);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(6);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(7) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(8));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(7);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _6_nested_function(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(8 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 0);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("inner", static_cast<size_t>(5)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _7_inner,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(2));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4620693217682128896u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _7_inner(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4620693217682128896u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _8_recursion(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(8 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(59);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(1))) { goto L2; } else { goto L1; }
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(60);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(1));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(1)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("stop", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(61);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(1) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("recursion", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(2));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _9_closure(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(8 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 0);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("inner", static_cast<size_t>(5)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _10_inner,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(2));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _10_inner(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _11_closure_2(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 0);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("inner", static_cast<size_t>(5)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _12_inner,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(0) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _12_inner(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _13_closure_3(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(12 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(62);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(63);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("imul", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("imul", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 0);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("inner", static_cast<size_t>(5)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _14_inner,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(6) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(4);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _14_inner(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(12 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(64);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(65);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("imul", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("imul", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 0);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("inner_inner", static_cast<size_t>(11)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _15_inner_inner,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(6) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(4);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _15_inner_inner(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(12 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(66);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(3).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(3));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(67);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(2) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(3)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(6) = frame.getLocalVarRef(3);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(4);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _16_closure_4(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(3 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 1);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(0), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("pow", static_cast<size_t>(3)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _17_pow,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(2) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(68);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(2));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(2)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("get", static_cast<size_t>(3)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _18_get,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(69);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("get", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(1));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(1)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _17_pow(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(13 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(70);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(4) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(4).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(4));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(71);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(3) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(3) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(4)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(3) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     Environment *curEnv = frame.getCalleeClosure()->getEnvironment();
// CPP:     for (unsigned level = 0; level; --level) {
// CPP:       curEnv = curEnv->getParentEnvironment();
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeObjectValue(curEnv);
// CPP:   }
// CPP:   frame.getLocalVarRef(6) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(4);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(7));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(6);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(0), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeUndefinedValue();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _18_get(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     Environment *curEnv = frame.getCalleeClosure()->getEnvironment();
// CPP:     for (unsigned level = 0; level; --level) {
// CPP:       curEnv = curEnv->getParentEnvironment();
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = HermesValue::encodeObjectValue(curEnv);
// CPP:   }
// CPP:   frame.getLocalVarRef(0) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }
