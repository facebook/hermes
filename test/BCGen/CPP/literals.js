/*
RUN: %hermes -target=c++ -dump-bytecode -standalone-c++ %s | %S/Run.sh ~ | %FileCheck --match-full-lines %s
RUN: %hermes -target=c++ -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix CPP %s
REQUIRES: cpp
*/

var a = [1, 2, 3]
print(a[2]);
// CHECK: 3

var b = {'foo': 2, 'bar': 3, 5: 6}
print(b['foo']);
// CHECK: 2
print(b[5]);
// CHECK: 6

var c = true;
print(c);
// CHECK: true

var d = null;
print(d);
// CHECK: null

var e = undefined;
print(e);
// CHECK: undefined

var bar = "bar"
var f = ["foo",,bar]
print(f[0]);
// CHECK: foo
print(f[2]);
// CHECK: bar

var g = 0.5;
print(g);
// CHECK: 0.5

var h = 2;
print(h);
// CHECK: 2

function foo() { return 1; }

var i = [1, 2, foo(), foo() + 1, 3, 4,,];
print(i[0], i[1], i[2], i[3], i[4], i[5], i[6], i.length);
// CHECK: 1 2 1 2 3 4 undefined 7

print('' + 'foo');
// CHECK: foo

var re = /aa/;
print(re.__proto__.constructor === RegExp);
// CHECK: true
print(/abc/.toString());
// CHECK: /abc/

var m = /\w (\d+)/.exec("abc 1234");
print(m.input + ", " + m.index + ", " + m.length + ", " + m[1]);
// CHECK: abc 1234, 2, 2, 1234
print(/\w (\d+)/.exec("abc def"));
// CHECK: null

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _1_foo(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(77 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3))),
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
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(62) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(61) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(60) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(59) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(58) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(57) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(56) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(55) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(54) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(53) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(52) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(51) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(42) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(50) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(49) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(47) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(46) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(22) = HermesValue::encodeDoubleValue(makeDouble(4618441417868443648u));
// CPP:   frame.getLocalVarRef(45) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(44) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(43) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(40) = HermesValue::encodeBoolValue(1);
// CPP:   frame.getLocalVarRef(41) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(38) = HermesValue::encodeNullValue();
// CPP:   frame.getLocalVarRef(39) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(36) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(37) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(35) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(34) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(33) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(32) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(29) = HermesValue::encodeDoubleValue(makeDouble(4602678819172646912u));
// CPP:   frame.getLocalVarRef(30) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(27) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(28) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(24) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(26) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(25) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   frame.getLocalVarRef(21) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(20) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(19) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(18) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(17) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(16) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(15) = HermesValue::encodeDoubleValue(makeDouble(4618441417868443648u));
// CPP:   frame.getLocalVarRef(23) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(14) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(13) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(12) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(11) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("abc 1234", static_cast<size_t>(8)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(10) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef(", ", static_cast<size_t>(2)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(9) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef(", ", static_cast<size_t>(2)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef(", ", static_cast<size_t>(2)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("abc def", static_cast<size_t>(7)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(0);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(62));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(62));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(62)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(62)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(1);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(61));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(61));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(61)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(61)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(2);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(60));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(60));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(60)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(60)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(3);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(59));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(59));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(59)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(59)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(4);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(58));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(58));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(58)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(58)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(5);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(57));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(57));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(57)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(6);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(56));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(56));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(56)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(56)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(7);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(55));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(55));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(55)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(55)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(8);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(54));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(54));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(54)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(54)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(9);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(53));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(53));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(53)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(53)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(10);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(52));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(52));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(52)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(52)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(48) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(48));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(11);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(51));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(51));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(51)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(51)),
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
// CPP:         ASCIIRef("foo", static_cast<size_t>(3)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _1_foo,
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
// CPP:     frame.getLocalVarRef(48) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(12);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(48));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(48));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(48)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 3, 3);
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
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u))));
// CPP:     frame.getLocalVarRef(48) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(13);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(48));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(48));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(48)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(42)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(48)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(14);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(48) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(48) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(48) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(42)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(48) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(15);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(42)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(42)), runtime, Handle<>(runtime, frame.getLocalVarRef(50)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(42)), Handle<>(runtime, frame.getLocalVarRef(50)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(49);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(42);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(48);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(42) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(42)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     JSObject::defineNewOwnProperty(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3))),
// CPP:         PropertyFlags::defaultNewNamedPropertyFlags(),
// CPP:         Handle<>(&frame.getLocalVarRef(47)));
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(42)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     JSObject::defineNewOwnProperty(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3))),
// CPP:         PropertyFlags::defaultNewNamedPropertyFlags(),
// CPP:         Handle<>(&frame.getLocalVarRef(46)));
// CPP:   }
// CPP:   marker.flush();
// CPP:   (void)JSObject::defineOwnComputedPrimitive(
// CPP:       Handle<JSObject>::vmcast(&frame.getLocalVarRef(42)),
// CPP:       runtime,
// CPP:       Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u))),
// CPP:       DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:       Handle<>(runtime, frame.getLocalVarRef(22)));
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(16);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(42));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(42));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(42)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(42)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(17);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(18);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(19);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(45);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(42);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(20);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(42) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(21);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(22)), runtime, Handle<>(runtime, frame.getLocalVarRef(44)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(22)), Handle<>(runtime, frame.getLocalVarRef(44)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(43);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(42);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(22);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(40));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(40));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(40)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(23);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(40) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(24);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(41);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(40);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(25);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(38));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(38));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(38)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(38)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(26);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(38) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(38) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(38) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(38) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(27);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("d", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(39);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(38);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(28);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(36));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(36));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(36)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(36)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(29);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(36) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(36) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(36) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(36) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(30);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("e", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(37);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(36);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(31);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(31));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(31));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(31)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 3, 3);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     {
// CPP:       auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:         return ExecutionStatus::EXCEPTION;
// CPP:       }
// CPP:       (void)JSObject::defineOwnComputedPrimitive(
// CPP:           Handle<JSObject>::vmcast(obj),
// CPP:           runtime,
// CPP:           Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(0u))),
// CPP:           DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:           Handle<>(runtime, *str));
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(32);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   (void)JSObject::defineOwnComputedPrimitive(
// CPP:       Handle<JSObject>::vmcast(&frame.getLocalVarRef(31)),
// CPP:       runtime,
// CPP:       Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u))),
// CPP:       DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:       Handle<>(runtime, frame.getLocalVarRef(22)));
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(33);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(31));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(31));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(31)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(34);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(31) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(35);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(22)), runtime, Handle<>(runtime, frame.getLocalVarRef(35)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(22)), Handle<>(runtime, frame.getLocalVarRef(35)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(34);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(31);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(36);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(31) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(37);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("f", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(22)), runtime, Handle<>(runtime, frame.getLocalVarRef(33)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(22)), Handle<>(runtime, frame.getLocalVarRef(33)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(31);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(38);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(29));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(29));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(29)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(29)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(39);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(29) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(29) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(29) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(29) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(40);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("g", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(30);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(29);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(41);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(27));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(27));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(27)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(27)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(42);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(27) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(27) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(27) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(27) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(43);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("h", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(28);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(27);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(44);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(24);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(22);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(71));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(24) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(22) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(45);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(26);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(22);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(71));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(22).isNumber() && frame.getLocalVarRef(25).isNumber())) {
// CPP:     frame.getLocalVarRef(22) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(22).getNumber() + frame.getLocalVarRef(25).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(22)), Handle<>(&frame.getLocalVarRef(25)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 7, 7);
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
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, frame.getLocalVarRef(24)));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, frame.getLocalVarRef(22)));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u))));
// CPP:     (void)JSObject::defineOwnComputedPrimitive(
// CPP:         Handle<JSObject>::vmcast(obj),
// CPP:         runtime,
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u))),
// CPP:         DefinePropertyFlags::getDefaultNewPropertyFlags(),
// CPP:         Handle<>(runtime, HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u))));
// CPP:     frame.getLocalVarRef(22) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(22));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(46);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(47);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(22));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(22));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(22)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(22)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(48);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(22) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(22) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(49);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(21)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(21) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(21)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(21) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(50);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(20)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(20) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(20)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(20) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(51);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(19)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(19)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(52);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(18)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(18) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(18)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(18) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(53);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(17)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(17)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(54);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(16)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(16)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(55);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(2)), runtime, Handle<>(runtime, frame.getLocalVarRef(15)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(15) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(2)), Handle<>(runtime, frame.getLocalVarRef(15)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(15) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(56);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("i", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(57);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(23);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(21);
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(20);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(19);
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(18);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(17);
// CPP:   frame.getLocalVarRef(65) = frame.getLocalVarRef(16);
// CPP:   frame.getLocalVarRef(64) = frame.getLocalVarRef(15);
// CPP:   frame.getLocalVarRef(63) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(22);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(8, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(69);
// CPP:     newFrame->getArgRef(2) = frame.getLocalVarRef(68);
// CPP:     newFrame->getArgRef(3) = frame.getLocalVarRef(67);
// CPP:     newFrame->getArgRef(4) = frame.getLocalVarRef(66);
// CPP:     newFrame->getArgRef(5) = frame.getLocalVarRef(65);
// CPP:     newFrame->getArgRef(6) = frame.getLocalVarRef(64);
// CPP:     newFrame->getArgRef(7) = frame.getLocalVarRef(63);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(8));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(2);
// CPP:   frame.getLocalVarRef(2) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(2).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(58);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(14).isNumber() && frame.getLocalVarRef(1).isNumber())) {
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(14).getNumber() + frame.getLocalVarRef(1).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(14)), Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(13);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto regRes = JSRegExp::create(runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
// CPP:     if (LLVM_UNLIKELY(regRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto re = runtime->makeHandle<JSRegExp>(*regRes);
// CPP:     auto patternRes = StringPrimitive::create(runtime, ASCIIRef("aa", static_cast<size_t>(2)));
// CPP:     if (LLVM_UNLIKELY(patternRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto pattern = runtime->makeHandle<StringPrimitive>(*patternRes);
// CPP:     auto flagsRes = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(flagsRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto flags = runtime->makeHandle<StringPrimitive>(*flagsRes);
// CPP:     auto bytecode = reinterpret_cast<const unsigned char*>("\x0\x0\x0\x0\x0\x4\x4\x61\x4\x61\x0");
// CPP:     auto res = JSRegExp::initialize(
// CPP:         re,
// CPP:         runtime,
// CPP:         pattern,
// CPP:         flags,
// CPP:         ArrayRef<uint8_t>(bytecode, 11));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = re.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(59);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(60);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(61);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("re", static_cast<size_t>(2)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(62);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("__proto__", static_cast<size_t>(9)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("__proto__", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(63);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(13) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("constructor", static_cast<size_t>(11)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(13) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(13) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("constructor", static_cast<size_t>(11)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(13) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(64);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("RegExp", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("RegExp", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(13), frame.getLocalVarRef(1)));
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(12);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(65);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto regRes = JSRegExp::create(runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
// CPP:     if (LLVM_UNLIKELY(regRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto re = runtime->makeHandle<JSRegExp>(*regRes);
// CPP:     auto patternRes = StringPrimitive::create(runtime, ASCIIRef("abc", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(patternRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto pattern = runtime->makeHandle<StringPrimitive>(*patternRes);
// CPP:     auto flagsRes = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(flagsRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto flags = runtime->makeHandle<StringPrimitive>(*flagsRes);
// CPP:     auto bytecode = reinterpret_cast<const unsigned char*>("\x0\x0\x0\x0\x0\x4\x4\x61\x4\x62\x4\x63\x0");
// CPP:     auto res = JSRegExp::initialize(
// CPP:         re,
// CPP:         runtime,
// CPP:         pattern,
// CPP:         flags,
// CPP:         ArrayRef<uint8_t>(bytecode, 13));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(12) = re.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(12));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(66);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("toString", static_cast<size_t>(8)));
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
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(12);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(71));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(11);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto regRes = JSRegExp::create(runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
// CPP:     if (LLVM_UNLIKELY(regRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto re = runtime->makeHandle<JSRegExp>(*regRes);
// CPP:     auto patternRes = StringPrimitive::create(runtime, ASCIIRef("\\w (\\d+)", static_cast<size_t>(8)));
// CPP:     if (LLVM_UNLIKELY(patternRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto pattern = runtime->makeHandle<StringPrimitive>(*patternRes);
// CPP:     auto flagsRes = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(flagsRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto flags = runtime->makeHandle<StringPrimitive>(*flagsRes);
// CPP:     auto bytecode = reinterpret_cast<const unsigned char*>("\x1\x0\x1\x0\x0\x4\xa\x0\x0\x0\x0\x8\x4\x20\xb\x1\x0\x10\x0\x0\x0\x0\x1\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x1\x0\x0\x0\x1\x4\x31\x0\x0\x0\xa\x0\x0\x0\x0\x2\x11\xb\x0\x0\x0\xc\x1\x0\x0");
// CPP:     auto res = JSRegExp::initialize(
// CPP:         re,
// CPP:         runtime,
// CPP:         pattern,
// CPP:         flags,
// CPP:         ArrayRef<uint8_t>(bytecode, 59));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = re.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(2));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(67);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("exec", static_cast<size_t>(4)));
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
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(2);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(10);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(68);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(69);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(70);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(71);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("input", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("input", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isNumber() && frame.getLocalVarRef(9).isNumber())) {
// CPP:     frame.getLocalVarRef(9) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(1).getNumber() + frame.getLocalVarRef(9).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(1)), Handle<>(&frame.getLocalVarRef(9)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(9) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(72);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(73);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("index", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("index", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(9).isNumber() && frame.getLocalVarRef(1).isNumber())) {
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(9).getNumber() + frame.getLocalVarRef(1).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(9)), Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isNumber() && frame.getLocalVarRef(8).isNumber())) {
// CPP:     frame.getLocalVarRef(8) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(1).getNumber() + frame.getLocalVarRef(8).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(1)), Handle<>(&frame.getLocalVarRef(8)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(74);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(75);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("length", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isNumber() && frame.getLocalVarRef(1).isNumber())) {
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(8).getNumber() + frame.getLocalVarRef(1).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(8)), Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isNumber() && frame.getLocalVarRef(6).isNumber())) {
// CPP:     frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(1).getNumber() + frame.getLocalVarRef(6).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(1)), Handle<>(&frame.getLocalVarRef(6)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(76);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("m", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(1)), runtime, Handle<>(runtime, frame.getLocalVarRef(7)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(1)), Handle<>(runtime, frame.getLocalVarRef(7)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isNumber() && frame.getLocalVarRef(1).isNumber())) {
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(6).getNumber() + frame.getLocalVarRef(1).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(6)), Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(4);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(77);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(2) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto regRes = JSRegExp::create(runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
// CPP:     if (LLVM_UNLIKELY(regRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto re = runtime->makeHandle<JSRegExp>(*regRes);
// CPP:     auto patternRes = StringPrimitive::create(runtime, ASCIIRef("\\w (\\d+)", static_cast<size_t>(8)));
// CPP:     if (LLVM_UNLIKELY(patternRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto pattern = runtime->makeHandle<StringPrimitive>(*patternRes);
// CPP:     auto flagsRes = StringPrimitive::create(runtime, ASCIIRef("", static_cast<size_t>(0)));
// CPP:     if (LLVM_UNLIKELY(flagsRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto flags = runtime->makeHandle<StringPrimitive>(*flagsRes);
// CPP:     auto bytecode = reinterpret_cast<const unsigned char*>("\x1\x0\x1\x0\x0\x4\xa\x0\x0\x0\x0\x8\x4\x20\xb\x1\x0\x10\x0\x0\x0\x0\x1\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x1\x0\x0\x0\x1\x4\x31\x0\x0\x0\xa\x0\x0\x0\x0\x2\x11\xb\x0\x0\x0\xc\x1\x0\x0");
// CPP:     auto res = JSRegExp::initialize(
// CPP:         re,
// CPP:         runtime,
// CPP:         pattern,
// CPP:         flags,
// CPP:         ArrayRef<uint8_t>(bytecode, 59));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(4) = re.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(4));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(78);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("exec", static_cast<size_t>(4)));
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
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(4);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(5);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(71) = frame.getLocalVarRef(3);
// CPP:   frame.getLocalVarRef(70) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(71));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(70);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(0);
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _1_foo(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(2 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeUndefinedValue();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }
