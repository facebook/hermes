/*
RUN: %hermes -target=c++ -dump-bytecode -standalone-c++ %s | %S/Run.sh ~ | %FileCheck --match-full-lines %s
RUN: %hermes -target=c++ -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix CPP %s
REQUIRES: cpp
*/

print(5 == 5);
// CHECK: true

print(0 == "0");
// CHECK: true

print(3 == 6);
// CHECK: false

print(4 != 3);
// CHECK: true

print(0 === "0");
// CHECK: false

var x = {};
print(x !== x);
// CHECK: false

print(3 > 4);
// CHECK: false

print([] > {});
// CHECK: false

print(3 >= 3);
// CHECK: true

print(3 < 4);
// CHECK: true

print(4 <= 3);
// CHECK: false

print(2 << 3);
// CHECK: 16

print(15 >> 2);
// CHECK: 3

print(15 >> 33);
// CHECK: 7

print(15 >> '67');
// CHECK: 1

print(-1 >> 1);
// CHECK: -1

print(-1 >>> 1);
// CHECK: 2147483647

print(3 + 3);
// CHECK: 6

print('3' + '3');
// CHECK: 33

print(3 - 3);
// CHECK: 0

print('2' - '2');
// CHECK: 0

print(3 * 3);
// CHECK: 9

print('x' * 'x');
// CHECK: NaN

print(3 / 3);
// CHECK: 1

print(6 % 4);
// CHECK: 2

print(-5 % -2);
// CHECK: -1

print(7 | 12);
// CHECK: 15

print('6' | '7');
// CHECK: 7

print(7 ^ 12);
// CHECK: 11

print(7 & 12);
// CHECK: 4

var y = {};
y["foo"] = 0;
print("foo" in y);
// CHECK: true
print("bar" in y);
// CHECK: false

print(({}) instanceof Object);
// CHECK: true

print(typeof("x"));
// CHECK: string

print(typeof(5));
// CHECK: number

print(-5);
// CHECK: -5

print(-'8');
// CHECK: -8

print(~5);
// CHECK: -6

print(~'9');
// CHECK: -10

print(~9999999999999999999999999999)
// CHECK: -1

print(!5);
// CHECK: false

print(!0);
// CHECK: true

print(+'1' + +'1')
// CHECK: 2

var arr = [1, 2, 3]
print(delete arr[0])
// CHECK: true
print(arr[0])
// CHECK: undefined
Object.freeze(arr);
try {
  (function() {
    "use strict";
    delete arr[1];
  })();
} catch (e) {
  print('foo');
}
// CHECK: foo
print(arr[1]);
// CHECK: 2

a = 10;
print(a);
// CHECK: 10

print(delete a);
// CHECK: true

print(delete a);
// CHECK: true

print(this.a);
// CHECK: undefined

var b = 20;
print(b);
// CHECK: 20

print(delete(b));
// CHECK: false

print(b);
// CHECK: 20

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _1(void *, Runtime *runtime, NativeArgs args);

// CPP: static CallResult<HermesValue> _0(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 0;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(148 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3))),
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
// CPP:             runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("?anon_2_e", static_cast<size_t>(9))),
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
// CPP:     frame.getLocalVarRef(17) = *env;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getThisArgRef().isObject())) {
// CPP:     frame.getLocalVarRef(1) = frame.getThisArgRef();
// CPP:   } else if (
// CPP:       frame.getThisArgRef().isNull() ||
// CPP:       frame.getThisArgRef().isUndefined()) {
// CPP:     frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>::vmcast(&frame.getThisArgRef()));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(140) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(139) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(138) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(137) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(134) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(136) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(131) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(135) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(133) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("0", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(128) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(132) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(130) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(125) = HermesValue::encodeDoubleValue(makeDouble(4618441417868443648u));
// CPP:   frame.getLocalVarRef(129) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(127) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(121) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(126) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(124) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("0", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(123) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(122) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(120) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(114) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(119) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(118) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(116) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(111) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(115) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(113) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(108) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(112) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(110) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(105) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(109) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(107) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(102) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(106) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(104) = HermesValue::encodeDoubleValue(makeDouble(4624633867356078080u));
// CPP:   frame.getLocalVarRef(99) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(103) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(101) = HermesValue::encodeDoubleValue(makeDouble(4624633867356078080u));
// CPP:   frame.getLocalVarRef(96) = HermesValue::encodeDoubleValue(makeDouble(4629841154425225216u));
// CPP:   frame.getLocalVarRef(100) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(98) = HermesValue::encodeDoubleValue(makeDouble(4624633867356078080u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("67", static_cast<size_t>(2)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(93) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(97) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(90) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(95) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(94) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(87) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(92) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(91) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(89) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(84) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(88) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("3", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(86) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("3", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(81) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(85) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(83) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(78) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(82) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("2", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(80) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("2", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(75) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(79) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(77) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(72) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(76) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(74) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(69) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(73) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(71) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(66) = HermesValue::encodeDoubleValue(makeDouble(4613937818241073152u));
// CPP:   frame.getLocalVarRef(70) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(68) = HermesValue::encodeDoubleValue(makeDouble(4618441417868443648u));
// CPP:   frame.getLocalVarRef(63) = HermesValue::encodeDoubleValue(makeDouble(4616189618054758400u));
// CPP:   frame.getLocalVarRef(67) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(65) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(60) = HermesValue::encodeDoubleValue(makeDouble(4611686018427387904u));
// CPP:   frame.getLocalVarRef(64) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(62) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   frame.getLocalVarRef(57) = HermesValue::encodeDoubleValue(makeDouble(4622945017495814144u));
// CPP:   frame.getLocalVarRef(61) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("6", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(59) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("7", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(54) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(58) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(56) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   frame.getLocalVarRef(51) = HermesValue::encodeDoubleValue(makeDouble(4622945017495814144u));
// CPP:   frame.getLocalVarRef(55) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(53) = HermesValue::encodeDoubleValue(makeDouble(4619567317775286272u));
// CPP:   frame.getLocalVarRef(44) = HermesValue::encodeDoubleValue(makeDouble(4622945017495814144u));
// CPP:   frame.getLocalVarRef(52) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(50) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(42) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(49) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(48) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("bar", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(47) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(46) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(45) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(43) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(38) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(41) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(36) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(39) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("8", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(34) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(37) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(32) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(35) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("9", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(30) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(33) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(28) = HermesValue::encodeDoubleValue(makeDouble(5026061058026967681u));
// CPP:   frame.getLocalVarRef(31) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(26) = HermesValue::encodeDoubleValue(makeDouble(4617315517961601024u));
// CPP:   frame.getLocalVarRef(29) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(19) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(27) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("1", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(25) = *str;
// CPP:   }
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("1", static_cast<size_t>(1)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(24) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(23) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(22) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(21) = HermesValue::encodeDoubleValue(makeDouble(0u));
// CPP:   frame.getLocalVarRef(20) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(16) = HermesValue::encodeUndefinedValue();
// CPP:   {
// CPP:     auto str = StringPrimitive::create(runtime, ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:     if (LLVM_UNLIKELY(str == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(13) = *str;
// CPP:   }
// CPP:   frame.getLocalVarRef(15) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(14) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(12) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(6) = HermesValue::encodeDoubleValue(makeDouble(4621819117588971520u));
// CPP:   frame.getLocalVarRef(11) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(10) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(9) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(7) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4626322717216342016u));
// CPP:   frame.getLocalVarRef(5) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(4) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(3) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(18) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(0);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(140));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(140));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(140)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(140)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(1);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(139));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(139));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(139)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(139)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(2);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(138));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(138));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(138)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(0)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(138)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(0).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(0));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(3);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(137));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(137));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(137)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(137)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(134);
// CPP:   frame.getLocalVarRef(134) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(134).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(134));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(4);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(134) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(134) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(134) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(134)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(134) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = abstractEqualityTest(runtime, Handle<>(&frame.getLocalVarRef(136)), Handle<>(&frame.getLocalVarRef(131)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(131) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(135);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(131);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(134);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(131) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(131);
// CPP:   frame.getLocalVarRef(131) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(131).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(131));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(5);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(131) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(131) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(131) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(131)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(131) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = abstractEqualityTest(runtime, Handle<>(&frame.getLocalVarRef(133)), Handle<>(&frame.getLocalVarRef(128)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(128) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(132);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(128);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(131);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(128) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(128);
// CPP:   frame.getLocalVarRef(128) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(128).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(128));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(6);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(128) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(128) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(128) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(128)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(128) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = abstractEqualityTest(runtime, Handle<>(&frame.getLocalVarRef(130)), Handle<>(&frame.getLocalVarRef(125)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(125) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(129);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(125);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(128);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(125) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(125);
// CPP:   frame.getLocalVarRef(125) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(125).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(125));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(7);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(125) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(125) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(125) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(125)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(125) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = abstractEqualityTest(runtime, Handle<>(&frame.getLocalVarRef(127)), Handle<>(&frame.getLocalVarRef(121)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(121) = HermesValue::encodeBoolValue(!res->getBool());
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(126);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(121);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(125);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(121) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(121);
// CPP:   frame.getLocalVarRef(121) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(121).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(121));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(8);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(121) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(121) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(121) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(121)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(121) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = HermesValue::encodeBoolValue(strictEqualityTest(frame.getLocalVarRef(124), frame.getLocalVarRef(117)));
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(123);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(117);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(121);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(117);
// CPP:   frame.getLocalVarRef(121) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(117).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(117));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(9);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(121));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(121));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(121)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(117)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(121)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(117).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(117));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(10);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(121) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(121) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(121) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(117)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(121) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(117).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(117));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(11);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(123) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(123) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(123) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(117)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(123) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(117).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(117));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(12);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(117) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("x", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(117)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(117) = HermesValue::encodeBoolValue(!strictEqualityTest(frame.getLocalVarRef(123), frame.getLocalVarRef(117)));
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(122);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(117);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(121);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(117);
// CPP:   frame.getLocalVarRef(117) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(117).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(117));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(13);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(117) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(117)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(114) = HermesValue::encodeBoolValue(frame.getLocalVarRef(120).getNumber() > frame.getLocalVarRef(114).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(119);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(114);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(117);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(114) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(114);
// CPP:   frame.getLocalVarRef(114) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(114).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(114));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(14);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(117) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(117) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(114)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(117) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto res = JSArray::create(runtime, 0, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto obj = toHandle(runtime, std::move(*res));
// CPP:     frame.getLocalVarRef(119) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(114) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(119).isNumber() && frame.getLocalVarRef(114).isNumber())) {
// CPP:     frame.getLocalVarRef(114) = HermesValue::encodeBoolValue(frame.getLocalVarRef(119).getNumber() > frame.getLocalVarRef(114).getNumber());
// CPP:   } else {
// CPP:     auto res = greaterOp(runtime, Handle<>(&frame.getLocalVarRef(119)), Handle<>(&frame.getLocalVarRef(114)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(114) = res.getValue();
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(118);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(114);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(117);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(114) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(114);
// CPP:   frame.getLocalVarRef(114) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(114).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(114));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(15);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(114) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(114) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(114) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(114)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(114) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(111) = HermesValue::encodeBoolValue(frame.getLocalVarRef(116).getNumber() >= frame.getLocalVarRef(111).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(115);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(111);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(114);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(111) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(111);
// CPP:   frame.getLocalVarRef(111) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(111).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(111));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(16);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(111) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(111) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(111) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(111)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(111) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(108) = HermesValue::encodeBoolValue(frame.getLocalVarRef(113).getNumber() < frame.getLocalVarRef(108).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(112);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(108);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(111);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(108) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(108);
// CPP:   frame.getLocalVarRef(108) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(108).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(108));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(17);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(108) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(108) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(108) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(108)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(108) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(105) = HermesValue::encodeBoolValue(frame.getLocalVarRef(110).getNumber() <= frame.getLocalVarRef(105).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(109);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(105);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(108);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(105) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(105);
// CPP:   frame.getLocalVarRef(105) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(105).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(105));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(18);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(105) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(105) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(105) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(105)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(105) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto lnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(107).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(102).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(102) = HermesValue::encodeDoubleValue(static_cast<int32_t>(lnum << rnum));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(106);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(102);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(105);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(102) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(102);
// CPP:   frame.getLocalVarRef(102) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(102).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(102));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(19);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(102) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(102) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(102) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(102)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(102) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto lnum = static_cast<int32_t>(hermes::truncateToInt32(frame.getLocalVarRef(104).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(99).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(99) = HermesValue::encodeDoubleValue(static_cast<int32_t>(lnum >> rnum));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(103);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(99);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(102);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(99) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(99);
// CPP:   frame.getLocalVarRef(99) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(99).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(99));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(20);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(99) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(99) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(99) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(99)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(99) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     auto lnum = static_cast<int32_t>(hermes::truncateToInt32(frame.getLocalVarRef(101).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(96).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(96) = HermesValue::encodeDoubleValue(static_cast<int32_t>(lnum >> rnum));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(100);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(96);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(99);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(96) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(96);
// CPP:   frame.getLocalVarRef(96) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(96).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(96));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(21);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(96) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(96) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(96) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(96)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(96) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(98).isNumber() && frame.getLocalVarRef(93).isNumber())) {
// CPP:     auto lnum = static_cast<int32_t>(hermes::truncateToInt32(frame.getLocalVarRef(98).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(93).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(93) = HermesValue::encodeDoubleValue(static_cast<int32_t>(lnum >> rnum));
// CPP:   } else {
// CPP:     auto leftRes = toInt32(runtime, Handle<>(&frame.getLocalVarRef(98)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto left = static_cast<int32_t>(leftRes->getNumber());
// CPP:     auto rightRes = toUInt32(runtime, Handle<>(&frame.getLocalVarRef(93)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto right = static_cast<uint32_t>(rightRes->getNumber()) & 0x1f;
// CPP:     frame.getLocalVarRef(93) = HermesValue::encodeDoubleValue(static_cast<int32_t>(left >> right));
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(97);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(93);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(96);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(93) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(93);
// CPP:   frame.getLocalVarRef(93) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(93).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(93));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(22);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(93) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(93) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(93) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(93)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(93) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(90) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(90).getNumber());
// CPP:   }
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(90).isNumber() && frame.getLocalVarRef(95).isNumber())) {
// CPP:     auto lnum = static_cast<int32_t>(hermes::truncateToInt32(frame.getLocalVarRef(90).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(95).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(90) = HermesValue::encodeDoubleValue(static_cast<int32_t>(lnum >> rnum));
// CPP:   } else {
// CPP:     auto leftRes = toInt32(runtime, Handle<>(&frame.getLocalVarRef(90)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto left = static_cast<int32_t>(leftRes->getNumber());
// CPP:     auto rightRes = toUInt32(runtime, Handle<>(&frame.getLocalVarRef(95)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto right = static_cast<uint32_t>(rightRes->getNumber()) & 0x1f;
// CPP:     frame.getLocalVarRef(90) = HermesValue::encodeDoubleValue(static_cast<int32_t>(left >> right));
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(94);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(90);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(93);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(90) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(90);
// CPP:   frame.getLocalVarRef(90) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(90).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(90));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(23);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(90) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(90) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(90) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(90)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(90) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(87) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(87).getNumber());
// CPP:   }
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(87).isNumber() && frame.getLocalVarRef(92).isNumber())) {
// CPP:     auto lnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(87).getNumber()));
// CPP:     auto rnum = static_cast<uint32_t>(hermes::truncateToInt32(frame.getLocalVarRef(92).getNumber())) & 0x1f;
// CPP:     frame.getLocalVarRef(87) = HermesValue::encodeDoubleValue(static_cast<uint32_t>(lnum >> rnum));
// CPP:   } else {
// CPP:     auto leftRes = toUInt32(runtime, Handle<>(&frame.getLocalVarRef(87)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto left = static_cast<uint32_t>(leftRes->getNumber());
// CPP:     auto rightRes = toUInt32(runtime, Handle<>(&frame.getLocalVarRef(92)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto right = static_cast<uint32_t>(rightRes->getNumber()) & 0x1f;
// CPP:     frame.getLocalVarRef(87) = HermesValue::encodeDoubleValue(static_cast<uint32_t>(left >> right));
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(91);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(87);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(90);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(87) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(87);
// CPP:   frame.getLocalVarRef(87) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(87).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(87));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(24);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(87) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(87) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(87) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(87)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(87) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(84) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(89).getNumber() + frame.getLocalVarRef(84).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(88);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(84);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(87);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(84) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(84);
// CPP:   frame.getLocalVarRef(84) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(84).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(84));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(25);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(84) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(84) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(84) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(84)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(84) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(86).isNumber() && frame.getLocalVarRef(81).isNumber())) {
// CPP:     frame.getLocalVarRef(81) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(86).getNumber() + frame.getLocalVarRef(81).getNumber());
// CPP:   } else {
// CPP:     auto res = addOp(runtime, Handle<>(&frame.getLocalVarRef(86)), Handle<>(&frame.getLocalVarRef(81)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(81) = *res;
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(85);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(81);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(84);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(81) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(81);
// CPP:   frame.getLocalVarRef(81) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(81).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(81));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(26);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(81) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(81) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(81) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(81)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(81) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(78) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(83).getNumber() - frame.getLocalVarRef(78).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(82);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(78);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(81);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(78) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(78);
// CPP:   frame.getLocalVarRef(78) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(78).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(78));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(27);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(78) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(78) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(78) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(78)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(78) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(80).isNumber() && frame.getLocalVarRef(75).isNumber())) {
// CPP:     frame.getLocalVarRef(75) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(80).getNumber() - frame.getLocalVarRef(75).getNumber());
// CPP:   } else {
// CPP:     auto leftRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(80)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto rightRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(75)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(75) = HermesValue::encodeDoubleValue(leftRes->getDouble() - rightRes->getDouble());
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(79);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(75);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(78);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(75) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(75);
// CPP:   frame.getLocalVarRef(75) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(75).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(75));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(28);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(75) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(75) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(75) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(75)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(75) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(72) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(77).getNumber() * frame.getLocalVarRef(72).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(76);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(72);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(75);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(72) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(72);
// CPP:   frame.getLocalVarRef(72) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(72).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(72));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(29);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(72) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(72) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(72) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(72)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(72) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(74).isNumber() && frame.getLocalVarRef(69).isNumber())) {
// CPP:     frame.getLocalVarRef(69) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(74).getNumber() * frame.getLocalVarRef(69).getNumber());
// CPP:   } else {
// CPP:     auto leftRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(74)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto rightRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(69)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(69) = HermesValue::encodeDoubleValue(leftRes->getDouble() * rightRes->getDouble());
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(73);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(69);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(72);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(69) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(69);
// CPP:   frame.getLocalVarRef(69) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(69).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(69));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(30);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(69) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(69) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(69) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(69)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(69) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(66) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(71).getNumber() / frame.getLocalVarRef(66).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(70);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(66);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(69);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(66) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(66);
// CPP:   frame.getLocalVarRef(66) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(66).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(66));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(31);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(66) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(66) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(66) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(66)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(66) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(63) = HermesValue::encodeDoubleValue(std::fmod(frame.getLocalVarRef(68).getNumber(), frame.getLocalVarRef(63).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(67);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(63);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(66);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(63) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(63);
// CPP:   frame.getLocalVarRef(63) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(63).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(63));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(32);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(63) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(63) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(63) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(63)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(63) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(65) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(65).getNumber());
// CPP:   }
// CPP:   {
// CPP:     frame.getLocalVarRef(60) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(60).getNumber());
// CPP:   }
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(65).isNumber() && frame.getLocalVarRef(60).isNumber())) {
// CPP:     frame.getLocalVarRef(60) = HermesValue::encodeDoubleValue(std::fmod(frame.getLocalVarRef(65).getNumber(), frame.getLocalVarRef(60).getNumber()));
// CPP:   } else {
// CPP:     auto leftRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(65)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto rightRes = toNumber(runtime, Handle<>(&frame.getLocalVarRef(60)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(60) = HermesValue::encodeDoubleValue(std::fmod(leftRes->getDouble(), rightRes->getDouble()));
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(64);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(60);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(63);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(60) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(60);
// CPP:   frame.getLocalVarRef(60) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(60).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(60));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(33);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(60) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(60) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(60) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(60)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(60) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(57) = HermesValue::encodeDoubleValue(hermes::truncateToInt32(frame.getLocalVarRef(62).getNumber()) | hermes::truncateToInt32(frame.getLocalVarRef(57).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(61);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(57);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(60);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(57) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(57);
// CPP:   frame.getLocalVarRef(57) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(57).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(57));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(34);
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
// CPP:         Handle<>(&frame.getLocalVarRef(57)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(57) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(59).isNumber() && frame.getLocalVarRef(54).isNumber())) {
// CPP:     frame.getLocalVarRef(54) = HermesValue::encodeDoubleValue(hermes::truncateToInt32(frame.getLocalVarRef(59).getNumber()) | hermes::truncateToInt32(frame.getLocalVarRef(54).getNumber()));
// CPP:   } else {
// CPP:     auto leftRes = toInt32(runtime, Handle<>(&frame.getLocalVarRef(59)));
// CPP:     if (LLVM_UNLIKELY(leftRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto rightRes = toInt32(runtime, Handle<>(&frame.getLocalVarRef(54)));
// CPP:     if (LLVM_UNLIKELY(rightRes == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(54) = HermesValue::encodeDoubleValue(leftRes->getNumberAs<int32_t>() | rightRes->getNumberAs<int32_t>());
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(58);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(54);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(57);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(54) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(54);
// CPP:   frame.getLocalVarRef(54) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(54).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(54));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(35);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(54) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(54) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(54) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(54)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(54) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(51) = HermesValue::encodeDoubleValue(hermes::truncateToInt32(frame.getLocalVarRef(56).getNumber()) ^ hermes::truncateToInt32(frame.getLocalVarRef(51).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(55);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(51);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(54);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(51) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(51);
// CPP:   frame.getLocalVarRef(51) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(51).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(51));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(36);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(51) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(51) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(51) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(51)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(51) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(44) = HermesValue::encodeDoubleValue(hermes::truncateToInt32(frame.getLocalVarRef(53).getNumber()) & hermes::truncateToInt32(frame.getLocalVarRef(44).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(52);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(44);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(51);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(44);
// CPP:   frame.getLocalVarRef(51) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(44) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(44).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(44));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(37);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(51));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(44)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(51)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(44) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(44).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(44));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(38);
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
// CPP:         Handle<>(&frame.getLocalVarRef(44)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(44).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(44));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(39);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(50));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("foo", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(50));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(50)),
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
// CPP:         Handle<>(&frame.getLocalVarRef(44)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(50)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(40);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(44) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = *res;
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
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(41);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
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
// CPP:   {
// CPP:     auto res = getIsIn(runtime, Handle<>(&frame.getLocalVarRef(49)), Handle<>(&frame.getLocalVarRef(42)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(48);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(42);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(44);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(42);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(44) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = *res;
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
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(43);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("y", static_cast<size_t>(1)));
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
// CPP:   {
// CPP:     auto res = getIsIn(runtime, Handle<>(&frame.getLocalVarRef(47)), Handle<>(&frame.getLocalVarRef(42)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(46);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(42);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(44);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(44);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(44) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(44) = *res;
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
// CPP:     frame.getLocalVarRef(44) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(46) = JSObject::create(runtime).getHermesValue();
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(45);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(42) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Object", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Object", static_cast<size_t>(6)));
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
// CPP:   {
// CPP:     auto res = instanceOf(runtime, Handle<>(&frame.getLocalVarRef(46)), Handle<>(&frame.getLocalVarRef(42)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = HermesValue::encodeBoolValue(*res);
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(45);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(42);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(44);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(42);
// CPP:   frame.getLocalVarRef(42) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(42).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(42));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(46);
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
// CPP:         Handle<>(&frame.getLocalVarRef(42)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(42) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(40) = typeOf(runtime, Handle<>(&frame.getLocalVarRef(40)));
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(43);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(40);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(42);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(40) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(40);
// CPP:   frame.getLocalVarRef(40) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(40).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(40));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(47);
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
// CPP:   frame.getLocalVarRef(38) = typeOf(runtime, Handle<>(&frame.getLocalVarRef(38)));
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(41);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(38);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(40);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(38) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(38);
// CPP:   frame.getLocalVarRef(38) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(38).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(38));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(48);
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
// CPP:         Handle<>(&frame.getLocalVarRef(38)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(38) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(36) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(36).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(39);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(36);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(38);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(36) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(36);
// CPP:   frame.getLocalVarRef(36) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(36).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(36));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(49);
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
// CPP:         Handle<>(&frame.getLocalVarRef(36)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(36) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(34).isNumber())) {
// CPP:     frame.getLocalVarRef(34) = HermesValue::encodeDoubleValue(-frame.getLocalVarRef(34).getNumber());
// CPP:   } else {
// CPP:     auto res = toNumber(runtime, Handle<>(&frame.getLocalVarRef(34)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(34) = HermesValue::encodeDoubleValue(-res->getNumber());
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(37);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(34);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(36);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(34) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(34);
// CPP:   frame.getLocalVarRef(34) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(34).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(34));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(50);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(34) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(34) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(34) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(34)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(34) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(32) = HermesValue::encodeDoubleValue(~hermes::truncateToInt32(frame.getLocalVarRef(32).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(35);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(32);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(34);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(32) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(32);
// CPP:   frame.getLocalVarRef(32) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(32).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(32));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(51);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(32) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(32) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(30).isNumber())) {
// CPP:     frame.getLocalVarRef(30) = HermesValue::encodeDoubleValue(~hermes::truncateToInt32(frame.getLocalVarRef(30).getNumber()));
// CPP:   } else {
// CPP:     auto res = toInt32(runtime, Handle<>(&frame.getLocalVarRef(30)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(30) = HermesValue::encodeDoubleValue(~res->getNumberAs<int32_t>());
// CPP:     marker.flush();
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(33);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(30);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(32);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(30) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(30);
// CPP:   frame.getLocalVarRef(30) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(30).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(30));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(52);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(30) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(30) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(30) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(30)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(30) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(28) = HermesValue::encodeDoubleValue(~hermes::truncateToInt32(frame.getLocalVarRef(28).getNumber()));
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(31);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(28);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(30);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(28) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(28);
// CPP:   frame.getLocalVarRef(28) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(28).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(28));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(53);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(28) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(28) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(28) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(28)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(28) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(26) = HermesValue::encodeBoolValue(!toBoolean(frame.getLocalVarRef(26)));
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(29);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(26);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(28);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(26) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(26);
// CPP:   frame.getLocalVarRef(26) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(26).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(26));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(54);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(26) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(26) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(26) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:   frame.getLocalVarRef(19) = HermesValue::encodeBoolValue(!toBoolean(frame.getLocalVarRef(19)));
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(27);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(19);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(26);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(19);
// CPP:   frame.getLocalVarRef(19) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(19).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(19));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(55);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(19) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(19)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(25).isNumber())) {
// CPP:     frame.getLocalVarRef(25) = frame.getLocalVarRef(25);
// CPP:   } else {
// CPP:     auto res = toNumber(runtime, Handle<>(&frame.getLocalVarRef(25)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(25) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isNumber())) {
// CPP:     frame.getLocalVarRef(8) = frame.getLocalVarRef(8);
// CPP:   } else {
// CPP:     auto res = toNumber(runtime, Handle<>(&frame.getLocalVarRef(8)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = res.getValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   {
// CPP:     frame.getLocalVarRef(8) = HermesValue::encodeDoubleValue(frame.getLocalVarRef(25).getNumber() + frame.getLocalVarRef(8).getNumber());
// CPP:   }
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(24);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(19);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
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
// CPP:     frame.getLocalVarRef(19) = obj.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(56);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(19));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(19));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(19)),
// CPP:             0 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(19)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(57);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(19) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(58);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto status = JSObject::deleteComputed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(8)),
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(23)),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(8)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteComputed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(23)),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(22);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(19);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(59);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(19) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(19) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(60);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(8)), runtime, Handle<>(runtime, frame.getLocalVarRef(21)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(8)), Handle<>(runtime, frame.getLocalVarRef(21)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(20);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(19);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(61);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(20) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Object", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("Object", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(62);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(19) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("freeze", static_cast<size_t>(6)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("freeze", static_cast<size_t>(6)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(20)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(19) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(63);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(20);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(19);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   goto L1;
// CPP: L1:
// CPP:   (void)&&L1;
// CPP:   {
// CPP:     auto objProto = Handle<JSObject>::vmcast(&runtime->objectPrototype);
// CPP:     auto prototypeObjectHandle = toHandle(runtime, JSObject::create(runtime, objProto));
// CPP:     auto symbol = runtime->getIdentifierTable().getSymbolHandle(
// CPP:         runtime,
// CPP:         ASCIIRef("", static_cast<size_t>(0)))->get();
// CPP:     auto func = toHandle(runtime, NativeConstructor::create(
// CPP:         runtime,
// CPP:         Handle<JSObject>::vmcast(&runtime->functionPrototype),
// CPP:         Handle<Environment>::vmcast(&frame.getLocalVarRef(17)),
// CPP:         nullptr,
// CPP:         _1,
// CPP:         JSObject::createWithException,
// CPP:         CellKind::ObjectKind));
// CPP:     (void)Callable::defineNameLengthAndPrototype(
// CPP:         func,
// CPP:         runtime,
// CPP:         symbol,
// CPP:         0,
// CPP:         prototypeObjectHandle,
// CPP:         false,
// CPP:         1);
// CPP:     frame.getLocalVarRef(8) = func.getHermesValue();
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(18);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(8);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       goto L3;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(0, *func, frame.getLocalVarRef(142));
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       goto L3;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(0));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   goto L2;
// CPP: L2:
// CPP:   (void)&&L2;
// CPP:   goto L4;
// CPP: L3:
// CPP:   (void)&&L3;
// CPP:   frame.getLocalVarRef(8) = runtime->getThrownValue();
// CPP:   runtime->clearThrownValue();
// CPP:   vmcast<Environment>(frame.getLocalVarRef(17))->slot(49).set(frame.getLocalVarRef(8), &runtime->getHeap());
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(64);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(16);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(13);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(8);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   goto L4;
// CPP: L4:
// CPP:   (void)&&L4;
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(65);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(13) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(13) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(13) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(13) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(66);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto res = JSObject::getComputed(Handle<JSObject>::vmcast(&frame.getLocalVarRef(8)), runtime, Handle<>(runtime, frame.getLocalVarRef(15)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   } else {
// CPP:     auto res = Interpreter::getByValTransient(runtime, Handle<>(&frame.getLocalVarRef(8)), Handle<>(runtime, frame.getLocalVarRef(15)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(14);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(8);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(13);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(8);
// CPP:   frame.getLocalVarRef(8) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(8).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(8));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(67);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(12));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor && desc.flags.writable && !desc.flags.internalSetter)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         JSObject::setNamedSlotValue(*obj, runtime, desc.slot, frame.getLocalVarRef(12));
// CPP:       } else {
// CPP:         auto res = JSObject::putNamed(
// CPP:             obj,
// CPP:             runtime,
// CPP:             id,
// CPP:             Handle<>(runtime, frame.getLocalVarRef(12)),
// CPP:             1 && strictMode ? defaultPropOpFlags.plusMustExist() : defaultPropOpFlags);
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::putByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(8)),
// CPP:         id,
// CPP:         Handle<>(&frame.getLocalVarRef(12)),
// CPP:         strictMode);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(68);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(69);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(6) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(11);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(6);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(8);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(70);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(6)),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(6)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       (void)amendPropAccessErrorMsgWithPropName(runtime, Handle<>(&frame.getLocalVarRef(6)), "delete", runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))));
// CPP:     return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(10);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(6);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(8);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(71);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(8) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:       NamedPropertyDescriptor desc;
// CPP:       if (LLVM_LIKELY(JSObject::tryGetOwnNamedDescriptorFast(*obj, id, desc) && !desc.flags.accessor)) {
// CPP:         if (LLVM_LIKELY(!clazz->isDictionary())) {
// CPP:           cacheEntry->clazz = clazz;
// CPP:           cacheEntry->slot = desc.slot;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = JSObject::getNamedSlotValue(*obj, desc);
// CPP:       } else {
// CPP:         auto res = JSObject::getNamedOrIndexed(obj, runtime, id, defaultPropOpFlags.plusMustExist());
// CPP:         if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:           return ExecutionStatus::EXCEPTION;
// CPP:         }
// CPP:         frame.getLocalVarRef(8) = *res;
// CPP:       }
// CPP:     }
// CPP:   } else {
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(8) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(6)),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(6)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       (void)amendPropAccessErrorMsgWithPropName(runtime, Handle<>(&frame.getLocalVarRef(6)), "delete", runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))));
// CPP:     return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(9);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(6);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(8);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
// CPP:     auto res = Callable::call(func, runtime, 0);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:     runtime->popStack(StackFrameLayout::callerOutgoingRegisters(1));
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(0) = frame.getLocalVarRef(6);
// CPP:   frame.getLocalVarRef(6) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(6).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(6));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(72);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(6) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("print", static_cast<size_t>(5)));
// CPP:     auto res = Interpreter::getByIdTransient(
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(6)),
// CPP:         id);
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(6) = *res;
// CPP:   }
// CPP:   marker.flush();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(73);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("a", static_cast<size_t>(1)));
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
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(7);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(6);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(74);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       JSObject::setNamedSlotValue<PropStorage::Inline::Yes>(*obj, runtime, cacheEntry->slot, frame.getLocalVarRef(2));
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(75);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(76);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(5);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
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
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       (void)amendPropAccessErrorMsgWithPropName(runtime, Handle<>(&frame.getLocalVarRef(1)), "delete", runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))));
// CPP:     return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteNamed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1))),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(4);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(78);
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
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(79);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("b", static_cast<size_t>(1)));
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
// CPP:   frame.getLocalVarRef(142) = frame.getLocalVarRef(3);
// CPP:   frame.getLocalVarRef(141) = frame.getLocalVarRef(1);
// CPP:   {
// CPP:     auto callee = frame.getLocalVarRef(2);
// CPP:     if (LLVM_UNLIKELY(!vmisa<Callable>(callee))) {
// CPP:       runtime->raiseTypeErrorForValue(Handle<>(runtime, callee), " is not a function");
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto func = Handle<Callable>::vmcast(&callee);
// CPP:     auto newFrame = runtime->pushNativeCallFrame<Runtime::InitializeArgs::No>(1, *func, frame.getLocalVarRef(142));
// CPP:     newFrame->getArgRef(0) = frame.getLocalVarRef(141);
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

// CPP: static CallResult<HermesValue> _1(void *, Runtime *runtime, NativeArgs args) {
// CPP:   StackFramePtr frame = runtime->getCurrentFrame();
// CPP:   constexpr bool strictMode = 1;
// CPP:   const PropOpFlags defaultPropOpFlags = strictMode ? PropOpFlags().plusThrowOnError() : PropOpFlags();
// CPP:   (void)defaultPropOpFlags;
// CPP:   runtime->checkAndAllocStack(3 + StackFrameLayout::CalleeExtraRegistersAtStart, HermesValue::encodeUndefinedValue());
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
// CPP:   frame.getLocalVarRef(2) = HermesValue::encodeDoubleValue(makeDouble(4607182418800017408u));
// CPP:   frame.getLocalVarRef(0) = HermesValue::encodeUndefinedValue();
// CPP:   frame.getLocalVarRef(1) = runtime->getGlobal().getHermesValue();
// CPP:   if (LLVM_LIKELY(frame.getLocalVarRef(1).isObject())) {
// CPP:     auto obj = Handle<JSObject>::vmcast(&frame.getLocalVarRef(1));
// CPP:     auto clazz = obj->getClass();
// CPP:     auto *cacheEntry = runtime->getCppPropertyCacheEntry(80);
// CPP:     if (LLVM_LIKELY(cacheEntry->clazz == clazz)) {
// CPP:       frame.getLocalVarRef(1) = JSObject::getNamedSlotValue<PropStorage::Inline::Yes>(*obj, cacheEntry->slot);
// CPP:     } else {
// CPP:       auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
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
// CPP:     auto id = runtime->getIdentifierTable().registerLazyIdentifier(ASCIIRef("arr", static_cast<size_t>(3)));
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
// CPP:     auto status = JSObject::deleteComputed(
// CPP:         Handle<JSObject>::vmcast(&frame.getLocalVarRef(1)),
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   } else {
// CPP:     auto res = toObject(runtime, Handle<>(&frame.getLocalVarRef(1)));
// CPP:     if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     auto status = JSObject::deleteComputed(
// CPP:         runtime->makeHandle<JSObject>(res.getValue()),
// CPP:         runtime,
// CPP:         Handle<>(&frame.getLocalVarRef(2)),
// CPP:         defaultPropOpFlags);
// CPP:     if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
// CPP:       return ExecutionStatus::EXCEPTION;
// CPP:     }
// CPP:     frame.getLocalVarRef(1) = HermesValue::encodeBoolValue(status.getValue());
// CPP:   }
// CPP:   marker.flush();
// CPP:   return frame.getLocalVarRef(0);
// CPP: }
