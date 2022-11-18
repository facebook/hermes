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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [protoDupComputed, protoDupMethod1, protoDupMethod2, protoDupAccessor1, protoDupAccessor2, protoDupAccessor3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %protoDupComputed()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "protoDupComputed" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %protoDupMethod1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "protoDupMethod1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %protoDupMethod2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "protoDupMethod2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %protoDupAccessor1()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "protoDupAccessor1" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %protoDupAccessor2()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "protoDupAccessor2" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %protoDupAccessor3()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "protoDupAccessor3" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = LoadFrameInst [func]
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, %2
// CHECK-NEXT:  %4 = StoreOwnPropertyInst null : null, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %5 = StoreOwnPropertyInst 42 : number, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = LoadFrameInst [func]
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, %2
// CHECK-NEXT:  %4 = CreateFunctionInst %__proto__()
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4 : closure, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 42 : number, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function __proto__(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = LoadFrameInst [y]
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2, %3
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %2 = CreateFunctionInst %"__proto__ 1#"()
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst %2 : closure, %1 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %4 = LoadFrameInst [func]
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %1 : object, %5
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst 42 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = LoadFrameInst [y]
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2, %3
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = LoadFrameInst [func]
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = AllocObjectInst 1 : number, %2
// CHECK-NEXT:  %4 = CreateFunctionInst %"get __proto__"()
// CHECK-NEXT:  %5 = StoreGetterSetterInst %4 : closure, undefined : undefined, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 33 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = LoadFrameInst [func]
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = AllocObjectInst 1 : number, %2
// CHECK-NEXT:  %4 = CreateFunctionInst %"set __proto__"()
// CHECK-NEXT:  %5 = StoreGetterSetterInst undefined : undefined, %4 : closure, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"(_)
// CHECK-NEXT:frame = [_]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %_, [_]
// CHECK-NEXT:  %1 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %2 = CreateFunctionInst %"get __proto__ 1#"()
// CHECK-NEXT:  %3 = CreateFunctionInst %"set __proto__ 1#"()
// CHECK-NEXT:  %4 = StoreGetterSetterInst %2 : closure, %3 : closure, %1 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %5 = LoadFrameInst [func]
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %1 : object, %6
// CHECK-NEXT:  %8 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__ 1#"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 33 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__ 1#"(_)
// CHECK-NEXT:frame = [_]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %_, [_]
// CHECK-NEXT:  %1 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
