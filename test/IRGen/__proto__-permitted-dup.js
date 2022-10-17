/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that __proto__  is only special in the form
// "__proto__ : AssignmentExpression"

// __proto__ with computed syntax is not special
function protoDupComputed(func) {
  return {__proto__: func(), ['__proto__']: null, a: 42};
}

// __proto__ which is a method is not special regardless of order
function protoDupMethod1(func) {
  return {__proto__: func(), __proto__(x,y) { return x + y; }, a: 42};
}

function protoDupMethod2(func) {
  return {__proto__(x,y) { return x + y; }, __proto__: func(), a: 42};
}

// __proto__ which is an accessor is not special.
function protoDupAccessor1(func) {
  return {__proto__: func(), get __proto__() { return 33;} };
}

function protoDupAccessor2(func) {
  return {__proto__: func(), set __proto__(_) { return 44;} };
}

function protoDupAccessor3(func) {
  return {set __proto__(_) { return 44;},
          __proto__: func(),
          get __proto__() { return 33;}};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [protoDupComputed, protoDupMethod1, protoDupMethod2, protoDupAccessor1, protoDupAccessor2, protoDupAccessor3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %protoDupComputed#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "protoDupComputed" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %protoDupMethod1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "protoDupMethod1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %protoDupMethod2#0#1()#5, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "protoDupMethod2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %protoDupAccessor1#0#1()#7, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "protoDupAccessor1" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %protoDupAccessor2#0#1()#9, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "protoDupAccessor2" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %protoDupAccessor3#0#1()#11, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "protoDupAccessor3" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed#0#1(func)#2
// CHECK-NEXT:frame = [func#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupComputed#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [func#2], %0
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = StoreOwnPropertyInst null : null, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 42 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1#0#1(func)#3
// CHECK-NEXT:frame = [func#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupMethod1#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [func#3], %0
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %__proto__#1#3()#4, %0
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5 : closure, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst 42 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function __proto__#1#3(x, y)#4
// CHECK-NEXT:frame = [x#4, y#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{__proto__#1#3()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %4 = LoadFrameInst [y#4], %0
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3, %4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2#0#1(func)#5
// CHECK-NEXT:frame = [func#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupMethod2#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#5], %0
// CHECK-NEXT:  %2 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %3 = CreateFunctionInst %"__proto__ 1#"#1#5()#6, %0
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst %3 : closure, %2 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %5 = LoadFrameInst [func#5], %0
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %6
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst 42 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %9 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"#1#5(x, y)#6
// CHECK-NEXT:frame = [x#6, y#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"__proto__ 1#"#1#5()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#6], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %4 = LoadFrameInst [y#6], %0
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3, %4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1#0#1(func)#7
// CHECK-NEXT:frame = [func#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupAccessor1#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#7], %0
// CHECK-NEXT:  %2 = LoadFrameInst [func#7], %0
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %"get __proto__"#1#7()#8, %0
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5 : closure, undefined : undefined, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"#1#7()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"get __proto__"#1#7()#8}
// CHECK-NEXT:  %1 = ReturnInst 33 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2#0#1(func)#9
// CHECK-NEXT:frame = [func#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupAccessor2#0#1()#9}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#9], %0
// CHECK-NEXT:  %2 = LoadFrameInst [func#9], %0
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %"set __proto__"#1#9()#10, %0
// CHECK-NEXT:  %6 = StoreGetterSetterInst undefined : undefined, %5 : closure, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"#1#9(_)#10
// CHECK-NEXT:frame = [_#10]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"set __proto__"#1#9()#10}
// CHECK-NEXT:  %1 = StoreFrameInst %_, [_#10], %0
// CHECK-NEXT:  %2 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3#0#1(func)#11
// CHECK-NEXT:frame = [func#11]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoDupAccessor3#0#1()#11}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#11], %0
// CHECK-NEXT:  %2 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %3 = CreateFunctionInst %"get __proto__ 1#"#1#11()#12, %0
// CHECK-NEXT:  %4 = CreateFunctionInst %"set __proto__ 1#"#1#11()#13, %0
// CHECK-NEXT:  %5 = StoreGetterSetterInst %3 : closure, %4 : closure, %2 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = LoadFrameInst [func#11], %0
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %7
// CHECK-NEXT:  %9 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__ 1#"#1#11()#12
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"get __proto__ 1#"#1#11()#12}
// CHECK-NEXT:  %1 = ReturnInst 33 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__ 1#"#1#11(_)#13
// CHECK-NEXT:frame = [_#13]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"set __proto__ 1#"#1#11()#13}
// CHECK-NEXT:  %1 = StoreFrameInst %_, [_#13], %0
// CHECK-NEXT:  %2 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
