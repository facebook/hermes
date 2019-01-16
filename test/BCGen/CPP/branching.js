/*
RUN: %hermes -target=c++ -dump-bytecode -standalone-c++ %s | %S/Run.sh ~ | %FileCheck --match-full-lines %s
RUN: %hermes -target=c++ -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix CPP %s
REQUIRES: cpp
*/

function basic() {
  var x = 1;
  if (x) return x;
  return 0;
}
print(basic());
// CHECK: 1

function nested() {
  var x = 3;
  var y = 0;
  if (x) {
    if (y) {
      return 1;
    } else {
      return 2;
    }
  } else {
    if (y) {
      return 3;
    } else {
      return 4;
    }
  }
}
print(nested());
// CHECK: 2

if (1) {
  print("foo");
} else {
  print("bar");
}
// CHECK: foo

function while_loop() {
  var x = 1;
  var y = 1;
  var z = 1;
  var r = 2;
  while (x) {
    x = Math.min(x, y);
    y = Math.min(y, z);
    z = 0;
    r = Math.pow(r, 2);
  }
  return r;
}
print(while_loop());
// CHECK: 256

while (0) {}
while (1) { break; }
for (;0;) {}
print("test");
// CHECK: test

var x = {};
x.a = '1';
x.b = '2';
for (var y in x) {
  print(y, x[y]);
}
// CHECK: a 1
// CHECK: b 2

x = undefined;
for (var y in x) {
  print(y, x[y]);
}

x = [];
x[0] = 1;
x['a'] = 2;
x[1] = 3;
for (var y in x) {
  print(y, x[y]);
}
// CHECK: 0 1
// CHECK: 1 3
// CHECK: a 2

Array.prototype[0] = 'a';
Array.prototype[1] = 'b';
Array.prototype[2] = 'c';
x = new Array();
x['a'] = 'd';
x['1'] = 'e';
for (var y in x) {
  print(y, x[y]);
}
// CHECK: 1 e
// CHECK: a d
// CHECK: 0 a
// CHECK: 2 c

print(3 > 2 ? 5 : 1);
// CHECK: 5

function multi_dense_switch(x) {
    var y = 0;
    switch (x) {
        case 1:
            y = 10
            break;
        case 2:
            y = 11
        case 3:
            y += 1
            break;
        case 5:
            y = 10
            break;
        case 6:
            y = 11
        case 8:
            y += 1
            break;
        case 9:
            y = 10
            break;
        case 10:
            y = 11
        case 11:
            y += 1
            break;
        case 12:
            y = 10
            break;
        case 13:
            y = 11
        case 14:
            y += 1
            break;
        default:
            y += 2
    }

    switch (y) {
        case 10:
            print(10);
            break;
        case 11:
            print(20);
            break;
        case 12:
            print(30);
            break;
        case 13:
            print(40);
        case 14:
            print(50);
            break;
        case 15:
            print(60);
            break;
        case 16:
            print(70);
            break;
        case 17:
            print(80);
        case 18:
            print(90);
            break;
        case 19:
            print(100);
            break;
        case 20:
            print(110);
            break;
        case 21:
            print(120);
    }
}

multi_dense_switch(1)
//CHECK: 10

multi_dense_switch(2)
//CHECK: 30

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _1_basic(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _2_nested(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _3_while_loop(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _4_multi_dense_switch(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(75 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("basic", static_cast<size_t>(5))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested", static_cast<size_t>(6))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("while_loop", static_cast<size_t>(10))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18))),
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
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 51);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(66) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(65) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(64) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(63) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(62) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(56) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(61) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(60) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(59) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(58) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(49) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(53) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(50) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(55) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(54) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(52) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(51) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(48) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(47) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(46) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(44) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("test", static_cast<size_t>(4)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("1", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(41) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("1", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(37) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("2", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(38) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("2", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(35) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(45) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(40) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(36) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(32) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(42) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(34) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(33) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(29) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(31) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(26) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(28) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(27) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(23) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(39) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(25) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(24) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(20) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(22) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(21) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(18) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("c", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(14) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("d", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("d", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(11) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("e", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(13) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(12) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("e", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(30) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(10) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(9) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(15) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(8) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(0);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(66));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(66));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(66)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(66)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(1);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(65));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(65));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(65)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(65)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(2);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(64));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(64));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(64)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(64)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(3);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(63));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(63));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(63)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(63)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(4);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(62));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(62)),
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
// CPP:         ASCIIRef("basic", static_cast<size_t>(5)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _1_basic,
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
// CPP:     frame.getLocalVarRef(62) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(5);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(62));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("basic", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("basic", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(62)),
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
// CPP:         ASCIIRef("nested", static_cast<size_t>(6)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _2_nested,
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
// CPP:     frame.getLocalVarRef(62) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(6);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(62));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(62)),
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
// CPP:         ASCIIRef("while_loop", static_cast<size_t>(10)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _3_while_loop,
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
// CPP:     frame.getLocalVarRef(62) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(7);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(62));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("while_loop", static_cast<size_t>(10)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("while_loop", static_cast<size_t>(10)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(62)),
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
// CPP:         ASCIIRef("multi_dense_switch", static_cast<size_t>(18)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(0)),
// CPP:         nullptr,
// CPP:         _4_multi_dense_switch,
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
// CPP:     frame.getLocalVarRef(57) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(8);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(57));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(56);
// CPP:   frame.getLocalVarRef(56) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(56).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(56));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(9);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(57) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(57) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(57) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(56)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(57) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(56) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(56).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(56));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(10);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(56) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("basic", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(56) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(56) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("basic", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(56)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(61);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(56);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(69));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(60);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(56);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(57);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(56);
// CPP:   frame.getLocalVarRef(56) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(56).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(56));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(11);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(57) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(57) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(57) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(56)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(57) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(56) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(56).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(56));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(12);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(56) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested", static_cast<size_t>(6)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(56) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(56) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("nested", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(56)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(59);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(56);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(69));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(58);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(56);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(57);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(56) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(56);
// CPP:   if (toBoolean(frame.getLocalVarRef(49))) { goto L2; } else { goto L1; }
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   frame.getLocalVarRef(49) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(49).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(49));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(13);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(49) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(49)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(55);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(54);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(49);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(49);
// CPP:   goto L3;
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   frame.getLocalVarRef(49) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(49).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(49));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(14);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(49) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(49)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(53);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(50);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(49);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(49);
// CPP:   goto L3;
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   frame.getLocalVarRef(49) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(49).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(49));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(15);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(50) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(50) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(50) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(49)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(50) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(49) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(49).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(49));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(16);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(49) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("while_loop", static_cast<size_t>(10)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(49) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("while_loop", static_cast<size_t>(10)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(49)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(52);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(49);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(69));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(51);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(49);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(50);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(49);
// CPP:   goto L4;
// CPP: L4:
// CPP:   (void)&&L4;
// CPP:   if (toBoolean(frame.getLocalVarRef(48))) { goto L5; } else { goto L8; }
// CPP: L5:
// CPP:   (void)&&L5;
// CPP:   goto L6;
// CPP: L6:
// CPP:   (void)&&L6;
// CPP:   goto L7;
// CPP: L7:
// CPP:   (void)&&L7;
// CPP:   if (toBoolean(frame.getLocalVarRef(47))) { goto L5; } else { goto L8; }
// CPP: L8:
// CPP:   (void)&&L8;
// CPP:   goto L9;
// CPP: L9:
// CPP:   (void)&&L9;
// CPP:   if (toBoolean(frame.getLocalVarRef(46))) { goto L10; } else { goto L11; }
// CPP: L10:
// CPP:   (void)&&L10;
// CPP:   goto L11;
// CPP: L11:
// CPP:   (void)&&L11;
// CPP:   goto L12;
// CPP: L12:
// CPP:   (void)&&L12;
// CPP:   if (toBoolean(frame.getLocalVarRef(45))) { goto L13; } else { goto L16; }
// CPP: L13:
// CPP:   (void)&&L13;
// CPP:   goto L14;
// CPP: L14:
// CPP:   (void)&&L14;
// CPP:   goto L15;
// CPP: L15:
// CPP:   (void)&&L15;
// CPP:   if (toBoolean(frame.getLocalVarRef(40))) { goto L13; } else { goto L16; }
// CPP: L16:
// CPP:   (void)&&L16;
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(17);
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
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(44);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(43);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(40);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(40);
// CPP:   frame.getLocalVarRef(43) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(18);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(43));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(43));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(43)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(19);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(40) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(20);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(41));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(41));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(41)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(41)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(37);
// CPP:   frame.getLocalVarRef(37) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(37).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(37));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(21);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(37) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(37) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(37) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(37)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(37) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(37).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(37));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(22);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(38));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(37)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(38)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(35);
// CPP:   frame.getLocalVarRef(35) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(35).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(35));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(23);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(35) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(35) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(35) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(35)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(35) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = frame.getLocalVarRef(35);
// CPP:   if (frame.getLocalVarRef(40).isUndefined() || frame.getLocalVarRef(40).isNull()) {
// CPP:     frame.getLocalVarRef(41) = HermesValue::encodeUndefinedValue();
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(40)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = res.getValue();
// CPP:     auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));
// CPP:     auto cr = getAllPropertyNames(obj, runtime);
// CPP:     if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto arr = *cr;
// CPP:     frame.getLocalVarRef(41) = arr.getHermesValue();
// CPP:     frame.getLocalVarRef(38) = HermesValue::encodeNumberValue(0);
// CPP:     frame.getLocalVarRef(37) = HermesValue::encodeNumberValue(JSArray::getLength(*arr));
// CPP:   }
// CPP:   if (frame.getLocalVarRef(41).isUndefined()) goto L19;
// CPP:   goto L17;
// CPP:   marker.flush();
// CPP: L17:
// CPP:   (void)&&L17;
// CPP:   {
// CPP:     MutableHandle<> tmpHandle{runtime};
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto arr = Handle<JSArray>::vmcast(&frame.getLocalVarRef(41));
// CPP:     uint32_t idx = frame.getLocalVarRef(38).getNumber();
// CPP:     uint32_t size = frame.getLocalVarRef(37).getNumber();
// CPP:     MutableHandle<JSObject> propObj{runtime};
// CPP:     while (idx < size) {
// CPP:       tmpHandle = arr->at(idx);
// CPP:       ComputedPropertyDescriptor desc;
// CPP:       JSObject::getComputedPrimitiveDescriptor(obj, runtime, tmpHandle, propObj, desc);
// CPP:       if (LLVM_LIKELY(propObj)) break;
// CPP:       ++idx;
// CPP:     }
// CPP:     if (idx < size) {
// CPP:       if (tmpHandle->isNumber()) {
// CPP:         auto status = toString(runtime, tmpHandle);
// CPP:         tmpHandle = status.getValue();
// CPP:       }
// CPP:       frame.getLocalVarRef(35) = tmpHandle.get();
// CPP:       frame.getLocalVarRef(38) = HermesValue::encodeNumberValue(idx + 1);
// CPP:     } else {
// CPP:       frame.getLocalVarRef(35) = HermesValue::encodeUndefinedValue();
// CPP:     }
// CPP:     if (frame.getLocalVarRef(35).isUndefined()) goto L19;
// CPP:     goto L18;
// CPP:   }
// CPP:   marker.flush();
// CPP: L18:
// CPP:   (void)&&L18;
// CPP:   frame.getLocalVarRef(44) = frame.getLocalVarRef(35);
// CPP:   frame.getLocalVarRef(43) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(43));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(24);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(44));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(44));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(44)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(44)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(43) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(43));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(25);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(45) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(45) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(45) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(45) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(43) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(43));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(26);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(44) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(43) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(43));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(27);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(46) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(46) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(46) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(46) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(43) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(43));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(28);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(43) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(43) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(43) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(43)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(46).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(46)), runtime, Handle<>(runtime, frame.getLocalVarRef(43)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(46)), Handle<>(runtime, frame.getLocalVarRef(43)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(44);
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(43);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(45);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(67);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(43);
// CPP:   goto L17;
// CPP: L19:
// CPP:   (void)&&L19;
// CPP:   frame.getLocalVarRef(35) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(35).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(35));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(29);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(36));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(35)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(36)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(32) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(32).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(32));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(30);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(32) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(32)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(32) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(37) = frame.getLocalVarRef(32);
// CPP:   if (frame.getLocalVarRef(37).isUndefined() || frame.getLocalVarRef(37).isNull()) {
// CPP:     frame.getLocalVarRef(38) = HermesValue::encodeUndefinedValue();
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(37)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(37) = res.getValue();
// CPP:     auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));
// CPP:     auto cr = getAllPropertyNames(obj, runtime);
// CPP:     if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto arr = *cr;
// CPP:     frame.getLocalVarRef(38) = arr.getHermesValue();
// CPP:     frame.getLocalVarRef(36) = HermesValue::encodeNumberValue(0);
// CPP:     frame.getLocalVarRef(35) = HermesValue::encodeNumberValue(JSArray::getLength(*arr));
// CPP:   }
// CPP:   if (frame.getLocalVarRef(38).isUndefined()) goto L22;
// CPP:   goto L20;
// CPP:   marker.flush();
// CPP: L20:
// CPP:   (void)&&L20;
// CPP:   {
// CPP:     MutableHandle<> tmpHandle{runtime};
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(37));
// CPP:     auto arr = Handle<JSArray>::vmcast(&frame.getLocalVarRef(38));
// CPP:     uint32_t idx = frame.getLocalVarRef(36).getNumber();
// CPP:     uint32_t size = frame.getLocalVarRef(35).getNumber();
// CPP:     MutableHandle<JSObject> propObj{runtime};
// CPP:     while (idx < size) {
// CPP:       tmpHandle = arr->at(idx);
// CPP:       ComputedPropertyDescriptor desc;
// CPP:       JSObject::getComputedPrimitiveDescriptor(obj, runtime, tmpHandle, propObj, desc);
// CPP:       if (LLVM_LIKELY(propObj)) break;
// CPP:       ++idx;
// CPP:     }
// CPP:     if (idx < size) {
// CPP:       if (tmpHandle->isNumber()) {
// CPP:         auto status = toString(runtime, tmpHandle);
// CPP:         tmpHandle = status.getValue();
// CPP:       }
// CPP:       frame.getLocalVarRef(32) = tmpHandle.get();
// CPP:       frame.getLocalVarRef(36) = HermesValue::encodeNumberValue(idx + 1);
// CPP:     } else {
// CPP:       frame.getLocalVarRef(32) = HermesValue::encodeUndefinedValue();
// CPP:     }
// CPP:     if (frame.getLocalVarRef(32).isUndefined()) goto L22;
// CPP:     goto L21;
// CPP:   }
// CPP:   marker.flush();
// CPP: L21:
// CPP:   (void)&&L21;
// CPP:   frame.getLocalVarRef(41) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(31);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(41));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(41));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(41)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(41)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(32);
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
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(33);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(41) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(41) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(41) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(41) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(34);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(43) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(43) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(43) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(43) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(35);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(40) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(40) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(40)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(43).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(43)), runtime, Handle<>(runtime, frame.getLocalVarRef(40)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(43)), Handle<>(runtime, frame.getLocalVarRef(40)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(39);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(41);
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(40);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(42);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(67);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(40);
// CPP:   goto L20;
// CPP: L22:
// CPP:   (void)&&L22;
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 0, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     frame.getLocalVarRef(32) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(35) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(35).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(35));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(36);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(32));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(32));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(32)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(35)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(32)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(32) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(32).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(32));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(37);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(32) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(32)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(32) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(32).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(32)), runtime, Handle<>(runtime, frame.getLocalVarRef(33)), Handle<>(runtime, frame.getLocalVarRef(34)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(32)), Handle<>(runtime, frame.getLocalVarRef(33)), Handle<>(runtime, frame.getLocalVarRef(34)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(29);
// CPP:   frame.getLocalVarRef(29) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(29).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(29));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(38);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(29) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(29) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(29) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(29)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(29) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(29).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(29));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(39);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(31));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(29)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(26);
// CPP:   frame.getLocalVarRef(26) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(26).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(26));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(40);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(26) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(26) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(26) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(26)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(26) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(26).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(26)), runtime, Handle<>(runtime, frame.getLocalVarRef(27)), Handle<>(runtime, frame.getLocalVarRef(28)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(26)), Handle<>(runtime, frame.getLocalVarRef(27)), Handle<>(runtime, frame.getLocalVarRef(28)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(23);
// CPP:   frame.getLocalVarRef(23) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(23).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(23));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(41);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(23) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(23)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(23) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(28) = frame.getLocalVarRef(23);
// CPP:   if (frame.getLocalVarRef(28).isUndefined() || frame.getLocalVarRef(28).isNull()) {
// CPP:     frame.getLocalVarRef(29) = HermesValue::encodeUndefinedValue();
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(28)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(28) = res.getValue();
// CPP:     auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));
// CPP:     auto cr = getAllPropertyNames(obj, runtime);
// CPP:     if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto arr = *cr;
// CPP:     frame.getLocalVarRef(29) = arr.getHermesValue();
// CPP:     frame.getLocalVarRef(27) = HermesValue::encodeNumberValue(0);
// CPP:     frame.getLocalVarRef(26) = HermesValue::encodeNumberValue(JSArray::getLength(*arr));
// CPP:   }
// CPP:   if (frame.getLocalVarRef(29).isUndefined()) goto L25;
// CPP:   goto L23;
// CPP:   marker.flush();
// CPP: L23:
// CPP:   (void)&&L23;
// CPP:   {
// CPP:     MutableHandle<> tmpHandle{runtime};
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(28));
// CPP:     auto arr = Handle<JSArray>::vmcast(&frame.getLocalVarRef(29));
// CPP:     uint32_t idx = frame.getLocalVarRef(27).getNumber();
// CPP:     uint32_t size = frame.getLocalVarRef(26).getNumber();
// CPP:     MutableHandle<JSObject> propObj{runtime};
// CPP:     while (idx < size) {
// CPP:       tmpHandle = arr->at(idx);
// CPP:       ComputedPropertyDescriptor desc;
// CPP:       JSObject::getComputedPrimitiveDescriptor(obj, runtime, tmpHandle, propObj, desc);
// CPP:       if (LLVM_LIKELY(propObj)) break;
// CPP:       ++idx;
// CPP:     }
// CPP:     if (idx < size) {
// CPP:       if (tmpHandle->isNumber()) {
// CPP:         auto status = toString(runtime, tmpHandle);
// CPP:         tmpHandle = status.getValue();
// CPP:       }
// CPP:       frame.getLocalVarRef(23) = tmpHandle.get();
// CPP:       frame.getLocalVarRef(27) = HermesValue::encodeNumberValue(idx + 1);
// CPP:     } else {
// CPP:       frame.getLocalVarRef(23) = HermesValue::encodeUndefinedValue();
// CPP:     }
// CPP:     if (frame.getLocalVarRef(23).isUndefined()) goto L25;
// CPP:     goto L24;
// CPP:   }
// CPP:   marker.flush();
// CPP: L24:
// CPP:   (void)&&L24;
// CPP:   frame.getLocalVarRef(32) = frame.getLocalVarRef(23);
// CPP:   frame.getLocalVarRef(31) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(31).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(31));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(42);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(32));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(32));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(32)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(32)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(31) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(31).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(31));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(43);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(33) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(33) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(33) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(33) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(31) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(31).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(31));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(44);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(32) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(32) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(31) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(31).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(31));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(45);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(34) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(34) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(34) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(34) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(31) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(31).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(31));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(46);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(31) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(31) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(31)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(34).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(34)), runtime, Handle<>(runtime, frame.getLocalVarRef(31)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(34)), Handle<>(runtime, frame.getLocalVarRef(31)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(30);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(31);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(33);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(67);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(31) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(31);
// CPP:   goto L23;
// CPP: L25:
// CPP:   (void)&&L25;
// CPP:   frame.getLocalVarRef(23) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(23).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(23));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(47);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(23) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(23)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(23) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(23).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(23));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(48);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(23) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(23) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(23)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(23) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(23).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(23)), runtime, Handle<>(runtime, frame.getLocalVarRef(24)), Handle<>(runtime, frame.getLocalVarRef(25)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(23)), Handle<>(runtime, frame.getLocalVarRef(24)), Handle<>(runtime, frame.getLocalVarRef(25)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(20);
// CPP:   frame.getLocalVarRef(20) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(20).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(20));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(49);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(20) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(20) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(20) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(20)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(20) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(20).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(20));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(50);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(20) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(20) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(20) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(20)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(20) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(20).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(20)), runtime, Handle<>(runtime, frame.getLocalVarRef(21)), Handle<>(runtime, frame.getLocalVarRef(22)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(20)), Handle<>(runtime, frame.getLocalVarRef(21)), Handle<>(runtime, frame.getLocalVarRef(22)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(17);
// CPP:   frame.getLocalVarRef(17) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(17).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(17));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(51);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(17) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(17)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(17).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(17));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(52);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(17) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(17)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(17).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(17)), runtime, Handle<>(runtime, frame.getLocalVarRef(18)), Handle<>(runtime, frame.getLocalVarRef(19)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(17)), Handle<>(runtime, frame.getLocalVarRef(18)), Handle<>(runtime, frame.getLocalVarRef(19)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(14);
// CPP:   frame.getLocalVarRef(14) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(14).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(14));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(53);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(14) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(14) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(14) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Array", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(14)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(14) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(14).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(14));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(54);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(17) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("prototype", static_cast<size_t>(9)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(14)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(frame.getLocalVarRef(14)))) {
// CPP:       runtime->raiseTypeError("constructor is not callable");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto crtRes = Callable::newObject(
// CPP:         Handle<Callable>::vmcast(&frame.getLocalVarRef(14)),
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(frame.getLocalVarRef(17).isObject() ? &frame.getLocalVarRef(17) : &runtime->objectPrototype));
// CPP:     if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *crtRes;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(17);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(14);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(69));
// CPP:     auto res = Callable::call(func, runtime, 1);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(14) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(14) = frame.getLocalVarRef(14).isObject() ? frame.getLocalVarRef(14) : frame.getLocalVarRef(17);
// CPP:   frame.getLocalVarRef(17) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(17).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(17));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(55);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(14));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(14));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(14)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(17)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(14)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(14);
// CPP:   frame.getLocalVarRef(14) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(14).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(14));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(56);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(14) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(14) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(14) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(14)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(14) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(14).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(14));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(57);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(16));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(16));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(16)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(14)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(11);
// CPP:   frame.getLocalVarRef(11) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(11).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(11));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(58);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(11) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(11) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(11) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(11)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(11) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(11).isObject())) {
// CPP:     auto res = JSObject::putComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(11)), runtime, Handle<>(runtime, frame.getLocalVarRef(12)), Handle<>(runtime, frame.getLocalVarRef(13)), defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   } else {
// CPP:     auto res = Interpreter::putByValTransient(runtime, Handle<>(&frame.getLocalVarRef(11)), Handle<>(runtime, frame.getLocalVarRef(12)), Handle<>(runtime, frame.getLocalVarRef(13)), strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(59);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
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
// CPP:   frame.getLocalVarRef(13) = frame.getLocalVarRef(1);
// CPP:   if (frame.getLocalVarRef(13).isUndefined() || frame.getLocalVarRef(13).isNull()) {
// CPP:     frame.getLocalVarRef(14) = HermesValue::encodeUndefinedValue();
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(13)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(13) = res.getValue();
// CPP:     auto obj = runtime->makeMutableHandle(vmcast<JSObject>(res.getValue()));
// CPP:     auto cr = getAllPropertyNames(obj, runtime);
// CPP:     if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto arr = *cr;
// CPP:     frame.getLocalVarRef(14) = arr.getHermesValue();
// CPP:     frame.getLocalVarRef(12) = HermesValue::encodeNumberValue(0);
// CPP:     frame.getLocalVarRef(11) = HermesValue::encodeNumberValue(JSArray::getLength(*arr));
// CPP:   }
// CPP:   if (frame.getLocalVarRef(14).isUndefined()) goto L28;
// CPP:   goto L26;
// CPP:   marker.flush();
// CPP: L26:
// CPP:   (void)&&L26;
// CPP:   {
// CPP:     MutableHandle<> tmpHandle{runtime};
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(13));
// CPP:     auto arr = Handle<JSArray>::vmcast(&frame.getLocalVarRef(14));
// CPP:     uint32_t idx = frame.getLocalVarRef(12).getNumber();
// CPP:     uint32_t size = frame.getLocalVarRef(11).getNumber();
// CPP:     MutableHandle<JSObject> propObj{runtime};
// CPP:     while (idx < size) {
// CPP:       tmpHandle = arr->at(idx);
// CPP:       ComputedPropertyDescriptor desc;
// CPP:       JSObject::getComputedPrimitiveDescriptor(obj, runtime, tmpHandle, propObj, desc);
// CPP:       if (LLVM_LIKELY(propObj)) break;
// CPP:       ++idx;
// CPP:     }
// CPP:     if (idx < size) {
// CPP:       if (tmpHandle->isNumber()) {
// CPP:         auto status = toString(runtime, tmpHandle);
// CPP:         tmpHandle = status.getValue();
// CPP:       }
// CPP:       frame.getLocalVarRef(1) = tmpHandle.get();
// CPP:       frame.getLocalVarRef(12) = HermesValue::encodeNumberValue(idx + 1);
// CPP:     } else {
// CPP:       frame.getLocalVarRef(1) = HermesValue::encodeUndefinedValue();
// CPP:     }
// CPP:     if (frame.getLocalVarRef(1).isUndefined()) goto L28;
// CPP:     goto L27;
// CPP:   }
// CPP:   marker.flush();
// CPP: L27:
// CPP:   (void)&&L27;
// CPP:   frame.getLocalVarRef(17) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(16) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(16).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(16));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(60);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(17));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(17));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(17)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(17)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(16) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(16).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(16));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(61);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(18) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(18) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(18) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(18) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(16) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(16).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(16));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(62);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(17) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(17) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(17) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(16) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(16).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(16));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(63);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(19) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(16) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(16).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(16));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(64);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(16) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(16) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(16) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(16)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(19).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(19)), runtime, Handle<>(runtime, frame.getLocalVarRef(16)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(19)), Handle<>(runtime, frame.getLocalVarRef(16)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(15);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(17);
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(16);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(18);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(67);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(16) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(16);
// CPP:   goto L26;
// CPP: L28:
// CPP:   (void)&&L28;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(65);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(9) = HermesValue::encodeBoolValue(frame.getLocalVarRef(10).getNumber() > frame.getLocalVarRef(9).getNumber());
// CPP:   }
// CPP:   if (toBoolean(frame.getLocalVarRef(9))) { goto L30; } else { goto L29; }
// CPP: L29:
// CPP:   (void)&&L29;
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(7);
// CPP:   goto L31;
// CPP: L30:
// CPP:   (void)&&L30;
// CPP:   frame.getLocalVarRef(7) = frame.getLocalVarRef(8);
// CPP:   goto L31;
// CPP: L31:
// CPP:   (void)&&L31;
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(7);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(66);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
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
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(5);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(4);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(67);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("multi_dense_switch", static_cast<size_t>(18)));
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
// CPP:   frame.getLocalVarRef(69) = frame.getLocalVarRef(3);
// CPP:   frame.getLocalVarRef(68) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(69));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(68);
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

// CPP: static CallResult<HermesValue> _1_basic(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(5 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 1);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeUndefinedValue();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(0).set(frame.getLocalVarRef(3), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(0).set(frame.getLocalVarRef(2), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(2) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(2))) { goto L3; } else { goto L1; }
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   goto L2;
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   return frame.getLocalVarRef(1);
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   frame.getLocalVarRef(0) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _2_nested(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(9 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 2);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(2) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   vmcast<Environment>(frame.getLocalVarRef(2))->slot(1).set(frame.getLocalVarRef(8), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(2))->slot(0).set(frame.getLocalVarRef(7), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(2))->slot(0).set(frame.getLocalVarRef(6), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(2))->slot(1).set(frame.getLocalVarRef(5), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(5) = vmcast<Environment>(frame.getLocalVarRef(2))->slot(0);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(5))) { goto L4; } else { goto L1; }
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   frame.getLocalVarRef(5) = vmcast<Environment>(frame.getLocalVarRef(2))->slot(1);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(5))) { goto L3; } else { goto L2; }
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   return frame.getLocalVarRef(4);
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   return frame.getLocalVarRef(3);
// CPP: L4:
// CPP:   (void)&&L4;
// CPP:   frame.getLocalVarRef(2) = vmcast<Environment>(frame.getLocalVarRef(2))->slot(1);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(2))) { goto L6; } else { goto L5; }
// CPP: L5:
// CPP:   (void)&&L5;
// CPP:   return frame.getLocalVarRef(1);
// CPP: L6:
// CPP:   (void)&&L6;
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _3_while_loop(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(20 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 4);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(0) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(10) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(9) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(8) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(11) = HermesValue::encodeUndefinedValue();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(3).set(frame.getLocalVarRef(10), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(2).set(frame.getLocalVarRef(9), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(1).set(frame.getLocalVarRef(8), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(0).set(frame.getLocalVarRef(7), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(0).set(frame.getLocalVarRef(6), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(1).set(frame.getLocalVarRef(5), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(2).set(frame.getLocalVarRef(4), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(3).set(frame.getLocalVarRef(1), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L1;
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(1))) { goto L2; } else { goto L5; }
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(68);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(6) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(69);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("min", static_cast<size_t>(3)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("min", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(4) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(1);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(14) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(13) = frame.getLocalVarRef(4);
// CPP:   frame.getLocalVarRef(12) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(5);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(14));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(13);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(12);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(0).set(frame.getLocalVarRef(1), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(70);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(6) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(6) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Math", static_cast<size_t>(4)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(71);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(5) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("min", static_cast<size_t>(3)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("min", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(5) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(4) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(1);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(2);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(14) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(13) = frame.getLocalVarRef(4);
// CPP:   frame.getLocalVarRef(12) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(5);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(14));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(13);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(12);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(1).set(frame.getLocalVarRef(1), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(2).set(frame.getLocalVarRef(3), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(72);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(73);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(4) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("pow", static_cast<size_t>(3)));
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
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(3);
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(14) = frame.getLocalVarRef(5);
// CPP:   frame.getLocalVarRef(13) = frame.getLocalVarRef(1);
// CPP:   frame.getLocalVarRef(12) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(4);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(2, *func, frame.getLocalVarRef(14));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(13);
// CPP:     newFrame->getArgRef(1) = frame.getLocalVarRef(12);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(2));
// CPP:   }
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(0))->slot(3).set(frame.getLocalVarRef(1), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L3;
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   goto L4;
// CPP: L4:
// CPP:   (void)&&L4;
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(0);
// CPP:   marker.flush();
// CPP:   if (toBoolean(frame.getLocalVarRef(1))) { goto L2; } else { goto L5; }
// CPP: L5:
// CPP:   (void)&&L5;
// CPP:   frame.getLocalVarRef(0) = vmcast<Environment>(frame.getLocalVarRef(0))->slot(3);
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }

// CPP: static CallResult<HermesValue> _4_multi_dense_switch(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(73 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
// CPP:   GCScopeMarkerRAII marker{runtime};
// CPP: L0:
// CPP:   (void)&&L0;
// CPP:   {
// CPP:     auto env = Environment::create(runtime, runtime->makeHandle(frame.getCalleeClosure()->getEnvironment()), 2);
// CPP:     if (LLVM_UNLIKELY(env == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(64) = args.getArg(0);
// CPP:   frame.getLocalVarRef(65) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(50) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(38) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(39) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(40) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(41) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(42) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(43) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(44) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(45) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(46) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(47) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(48) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(49) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(51) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeDoubleValue(makeDouble(4626322717216342016u));
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(makeDouble(4629137466983448576u));
// CPP:   frame.getLocalVarRef(11) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(10) = HermesValue::encodeDoubleValue(makeDouble(4630826316843712512u));
// CPP:   frame.getLocalVarRef(9) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(8) = HermesValue::encodeDoubleValue(makeDouble(4632233691727265792u));
// CPP:   frame.getLocalVarRef(13) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(12) = HermesValue::encodeDoubleValue(makeDouble(4633641066610819072u));
// CPP:   frame.getLocalVarRef(15) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(14) = HermesValue::encodeDoubleValue(makeDouble(4634626229029306368u));
// CPP:   frame.getLocalVarRef(19) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(18) = HermesValue::encodeDoubleValue(makeDouble(4635329916471083008u));
// CPP:   frame.getLocalVarRef(17) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(16) = HermesValue::encodeDoubleValue(makeDouble(4636033603912859648u));
// CPP:   frame.getLocalVarRef(21) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(20) = HermesValue::encodeDoubleValue(makeDouble(4636737291354636288u));
// CPP:   frame.getLocalVarRef(23) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(22) = HermesValue::encodeDoubleValue(makeDouble(4637440978796412928u));
// CPP:   frame.getLocalVarRef(25) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(24) = HermesValue::encodeDoubleValue(makeDouble(4638144666238189568u));
// CPP:   frame.getLocalVarRef(52) = HermesValue::encodeDoubleValue(makeDouble(4624070917402656768u));
// CPP:   frame.getLocalVarRef(53) = HermesValue::encodeDoubleValue(makeDouble(4623507967449235456u));
// CPP:   frame.getLocalVarRef(54) = HermesValue::encodeDoubleValue(makeDouble(4622945017495814144u));
// CPP:   frame.getLocalVarRef(55) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(56) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(57) = HermesValue::encodeDoubleValue(makeDouble(4621256167635550208u));
// CPP:   frame.getLocalVarRef(58) = HermesValue::encodeDoubleValue(makeDouble(4620693217682128896u));
// CPP:   frame.getLocalVarRef(59) = HermesValue::encodeDoubleValue(makeDouble(4618441417868443648u));
// CPP:   frame.getLocalVarRef(60) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(61) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(62) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(63) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(26) = HermesValue::encodeDoubleValue(makeDouble(4626604192193052672u));
// CPP:   frame.getLocalVarRef(27) = HermesValue::encodeDoubleValue(makeDouble(4626322717216342016u));
// CPP:   frame.getLocalVarRef(28) = HermesValue::encodeDoubleValue(makeDouble(4626041242239631360u));
// CPP:   frame.getLocalVarRef(29) = HermesValue::encodeDoubleValue(makeDouble(4625759767262920704u));
// CPP:   frame.getLocalVarRef(30) = HermesValue::encodeDoubleValue(makeDouble(4625478292286210048u));
// CPP:   frame.getLocalVarRef(31) = HermesValue::encodeDoubleValue(makeDouble(4625196817309499392u));
// CPP:   frame.getLocalVarRef(32) = HermesValue::encodeDoubleValue(makeDouble(4624633867356078080u));
// CPP:   frame.getLocalVarRef(33) = HermesValue::encodeDoubleValue(makeDouble(4624070917402656768u));
// CPP:   frame.getLocalVarRef(34) = HermesValue::encodeDoubleValue(makeDouble(4623507967449235456u));
// CPP:   frame.getLocalVarRef(35) = HermesValue::encodeDoubleValue(makeDouble(4622945017495814144u));
// CPP:   frame.getLocalVarRef(36) = HermesValue::encodeDoubleValue(makeDouble(4622382067542392832u));
// CPP:   frame.getLocalVarRef(37) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(65), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(1).set(frame.getLocalVarRef(64), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(50), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(50) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(1);
// CPP:   marker.flush();
// CPP:   goto L1;
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   frame.getLocalVarRef(63) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(63), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(63))) { goto L25; } else { goto L2; }
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   frame.getLocalVarRef(62) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(62), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(62))) { goto L23; } else { goto L3; }
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   frame.getLocalVarRef(61) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(61), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(61))) { goto L24; } else { goto L4; }
// CPP: L4:
// CPP:   (void)&&L4;
// CPP:   frame.getLocalVarRef(60) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(60), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(60))) { goto L22; } else { goto L5; }
// CPP: L5:
// CPP:   (void)&&L5;
// CPP:   frame.getLocalVarRef(59) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(59), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(59))) { goto L20; } else { goto L6; }
// CPP: L6:
// CPP:   (void)&&L6;
// CPP:   frame.getLocalVarRef(58) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(58), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(58))) { goto L21; } else { goto L7; }
// CPP: L7:
// CPP:   (void)&&L7;
// CPP:   frame.getLocalVarRef(57) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(57), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(57))) { goto L19; } else { goto L8; }
// CPP: L8:
// CPP:   (void)&&L8;
// CPP:   frame.getLocalVarRef(56) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(56), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(56))) { goto L17; } else { goto L9; }
// CPP: L9:
// CPP:   (void)&&L9;
// CPP:   frame.getLocalVarRef(55) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(55), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(55))) { goto L18; } else { goto L10; }
// CPP: L10:
// CPP:   (void)&&L10;
// CPP:   frame.getLocalVarRef(54) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(54), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(54))) { goto L16; } else { goto L11; }
// CPP: L11:
// CPP:   (void)&&L11;
// CPP:   frame.getLocalVarRef(53) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(53), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(53))) { goto L14; } else { goto L12; }
// CPP: L12:
// CPP:   (void)&&L12;
// CPP:   frame.getLocalVarRef(50) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(52), frame.getLocalVarRef(50)));
// CPP:   if (toBoolean(frame.getLocalVarRef(50))) { goto L15; } else { goto L13; }
// CPP: L13:
// CPP:   (void)&&L13;
// CPP:   frame.getLocalVarRef(50) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(50).isNumber() && frame.getLocalVarRef(51).isNumber())) {
// CPP:     frame.getLocalVarRef(50) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(50).getNumber() + frame.getLocalVarRef(51).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(50)), Handle<>(&frame.getLocalVarRef(51)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(50) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(50), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L14:
// CPP:   (void)&&L14;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(48), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L15;
// CPP: L15:
// CPP:   (void)&&L15;
// CPP:   frame.getLocalVarRef(48) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(48).isNumber() && frame.getLocalVarRef(49).isNumber())) {
// CPP:     frame.getLocalVarRef(48) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(48).getNumber() + frame.getLocalVarRef(49).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(48)), Handle<>(&frame.getLocalVarRef(49)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(48) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(48), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L16:
// CPP:   (void)&&L16;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(47), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L17:
// CPP:   (void)&&L17;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(45), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L18;
// CPP: L18:
// CPP:   (void)&&L18;
// CPP:   frame.getLocalVarRef(45) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(45).isNumber() && frame.getLocalVarRef(46).isNumber())) {
// CPP:     frame.getLocalVarRef(45) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(45).getNumber() + frame.getLocalVarRef(46).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(45)), Handle<>(&frame.getLocalVarRef(46)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(45) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(45), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L19:
// CPP:   (void)&&L19;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(44), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L20:
// CPP:   (void)&&L20;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(42), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L21;
// CPP: L21:
// CPP:   (void)&&L21;
// CPP:   frame.getLocalVarRef(42) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isNumber() && frame.getLocalVarRef(43).isNumber())) {
// CPP:     frame.getLocalVarRef(42) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(42).getNumber() + frame.getLocalVarRef(43).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(42)), Handle<>(&frame.getLocalVarRef(43)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(42), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L22:
// CPP:   (void)&&L22;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(41), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L23:
// CPP:   (void)&&L23;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(39), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L24;
// CPP: L24:
// CPP:   (void)&&L24;
// CPP:   frame.getLocalVarRef(39) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(39).isNumber() && frame.getLocalVarRef(40).isNumber())) {
// CPP:     frame.getLocalVarRef(39) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(39).getNumber() + frame.getLocalVarRef(40).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(39)), Handle<>(&frame.getLocalVarRef(40)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(39) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(39), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L25:
// CPP:   (void)&&L25;
// CPP:   vmcast<Environment>(frame.getLocalVarRef(1))->slot(0).set(frame.getLocalVarRef(38), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   goto L26;
// CPP: L26:
// CPP:   (void)&&L26;
// CPP:   frame.getLocalVarRef(1) = vmcast<Environment>(frame.getLocalVarRef(1))->slot(0);
// CPP:   marker.flush();
// CPP:   goto L27;
// CPP: L27:
// CPP:   (void)&&L27;
// CPP:   frame.getLocalVarRef(37) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(37), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(37))) { goto L50; } else { goto L28; }
// CPP: L28:
// CPP:   (void)&&L28;
// CPP:   frame.getLocalVarRef(36) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(36), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(36))) { goto L49; } else { goto L29; }
// CPP: L29:
// CPP:   (void)&&L29;
// CPP:   frame.getLocalVarRef(35) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(35), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(35))) { goto L48; } else { goto L30; }
// CPP: L30:
// CPP:   (void)&&L30;
// CPP:   frame.getLocalVarRef(34) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(34), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(34))) { goto L46; } else { goto L31; }
// CPP: L31:
// CPP:   (void)&&L31;
// CPP:   frame.getLocalVarRef(33) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(33), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(33))) { goto L47; } else { goto L32; }
// CPP: L32:
// CPP:   (void)&&L32;
// CPP:   frame.getLocalVarRef(32) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(32), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(32))) { goto L45; } else { goto L33; }
// CPP: L33:
// CPP:   (void)&&L33;
// CPP:   frame.getLocalVarRef(31) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(31), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(31))) { goto L44; } else { goto L34; }
// CPP: L34:
// CPP:   (void)&&L34;
// CPP:   frame.getLocalVarRef(30) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(30), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(30))) { goto L42; } else { goto L35; }
// CPP: L35:
// CPP:   (void)&&L35;
// CPP:   frame.getLocalVarRef(29) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(29), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(29))) { goto L43; } else { goto L36; }
// CPP: L36:
// CPP:   (void)&&L36;
// CPP:   frame.getLocalVarRef(28) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(28), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(28))) { goto L41; } else { goto L37; }
// CPP: L37:
// CPP:   (void)&&L37;
// CPP:   frame.getLocalVarRef(27) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(27), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(27))) { goto L40; } else { goto L38; }
// CPP: L38:
// CPP:   (void)&&L38;
// CPP:   frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(26), frame.getLocalVarRef(1)));
// CPP:   if (toBoolean(frame.getLocalVarRef(1))) { goto L39; } else { goto L51; }
// CPP: L39:
// CPP:   (void)&&L39;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(74);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(25);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(24);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L40:
// CPP:   (void)&&L40;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(75);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(23);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(22);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L41:
// CPP:   (void)&&L41;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(76);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(21);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(20);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L42:
// CPP:   (void)&&L42;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(77);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(19);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(18);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L43;
// CPP: L43:
// CPP:   (void)&&L43;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(78);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(17);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(16);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L44:
// CPP:   (void)&&L44;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(79);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(15);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(14);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L45:
// CPP:   (void)&&L45;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(80);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(13);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(12);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L46:
// CPP:   (void)&&L46;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(81);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(11);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(10);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L47;
// CPP: L47:
// CPP:   (void)&&L47;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(82);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(9);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L48:
// CPP:   (void)&&L48;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(83);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(7);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(6);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L49:
// CPP:   (void)&&L49;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(84);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(5);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(4);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L50:
// CPP:   (void)&&L50;
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(85);
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
// CPP:         Handle<>(&frame.getLocalVarRef(1)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(67) = frame.getLocalVarRef(3);
// CPP:   frame.getLocalVarRef(66) = frame.getLocalVarRef(2);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(1);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(67));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(66);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   goto L51;
// CPP: L51:
// CPP:   (void)&&L51;
// CPP:   return frame.getLocalVarRef(0);
// CPP: }
