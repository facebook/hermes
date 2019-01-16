/*
RUN: %hermes -O -target=c++ -dump-bytecode -standalone-c++ %s | %S/Run.sh ~ | %FileCheck --match-full-lines %s
RUN: %hermes -O -target=c++ -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix CPP %s
REQUIRES: cpp
*/

function f1() {
  var a = [5];
  return a[0];
}
print(f1());
// CHECK: 5

function f2() {
  var a = [7];
  return a['0'];
}
print(f2());
// CHECK: 7

function f3() {
  var a = {a: 3, b: 5};
  delete a.b;
  return a['a'];
}
print(f3());
// CHECK: 3

function f4() {
  var a = "";
  return a['a'];
}
print(f4());
// CHECK: undefined

function f5() {
  var a = "";
  return a[0];
}
print(f5());
// CHECK: undefined

function f6() {
  var a = [1, 2];
  return a['length'];
}
print(f6());
// CHECK: 2

function f7(a) {
  return a['length'];
}
print(f7('foo'));
// CHECK: 3
print(f7({}));
// CHECK: undefined

function f8(a) {
  return delete a.x;
}
print(f8('bar'));
// CHECK: true
var x = {x: 5}
print(f8(x));
// CHECK: true
print(x.x);
// CHECK: undefined

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _1_f1(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _2_f2(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _3_f3(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _4_f4(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _5_f5(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _6_f6(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _7_f7(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _8_f8(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(12 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f1", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f2", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f3", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f4", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f5", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f6", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2))),
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
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 54);
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:         ASCIIRef("f1", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _1_f1,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f1", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f1", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f2", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _2_f2,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f2", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f2", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f3", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _3_f3,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f3", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f3", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f4", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _4_f4,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f4", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f4", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f5", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _5_f5,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f5", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f5", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f6", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _6_f6,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f6", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f6", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f7", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _7_f7,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         1,
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:         ASCIIRef("f8", static_cast<size_t>(2)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         nullptr,
// CPP:         _8_f8,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         1,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         0);
// CPP:     frame.getLocalVarRef(1) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(8);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(9);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(10);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f1", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f1", static_cast<size_t>(2)));
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
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(11);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(12);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f2", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f2", static_cast<size_t>(2)));
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
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
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
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(13);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(14);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f3", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f3", static_cast<size_t>(2)));
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
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
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
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(15);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(16);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f4", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f4", static_cast<size_t>(2)));
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
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
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
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f5", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f5", static_cast<size_t>(2)));
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
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
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
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f6", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f6", static_cast<size_t>(2)));
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
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(6));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
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
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(23);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(24);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f7", static_cast<size_t>(2)));
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
// CPP:   frame.getLocalVarRef(5) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(25);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(26);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     JSObject::defineNewOwnProperty(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1))),
// CPP:         PropertyFlags::defaultNewNamedPropertyFlags(),
// CPP:         Handle<>(&frame.getLocalVarRef(3)));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(27);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(1));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f8", static_cast<size_t>(2)));
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
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(30);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(3);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(31);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(32);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(33);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(5) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(6));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(5);
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

// CPP: static CallResult<HermesValue> _1_f1(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(2 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 1, 1);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(0u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u))));
// CPP:     frame.getLocalVarRef(1) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(1)), runtime, Handle<>(runtime, frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _2_f2(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(2 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 1, 1);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(0u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u))));
// CPP:     frame.getLocalVarRef(1) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(1)), runtime, Handle<>(runtime, frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _3_f3(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(2 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     JSObject::defineNewOwnProperty(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
// CPP:         PropertyFlags::defaultNewNamedPropertyFlags(),
// CPP:         Handle<>(&frame.getLocalVarRef(1)));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     JSObject::defineNewOwnProperty(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))),
// CPP:         PropertyFlags::defaultNewNamedPropertyFlags(),
// CPP:         Handle<>(&frame.getLocalVarRef(1)));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(34);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _4_f4(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _5_f5(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(2 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(1)), Handle<>(runtime, frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _6_f6(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 2, 2);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(0u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u))));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u))));
// CPP:     frame.getLocalVarRef(0) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(35);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _7_f7(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = args.getArg(0);
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(36);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(0) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _8_f8(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(1 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   frame.getLocalVarRef(0) = args.getArg(0);
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(0)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       (void)amendPropAccessErrorMsgWithPropName(runtime, Handle<>(&frame.getLocalVarRef(0)), "delete", runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1))));
// CPP:     return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }
