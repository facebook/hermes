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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "protoDupComputed" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "protoDupMethod1" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "protoDupMethod2" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "protoDupAccessor1" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "protoDupAccessor2" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "protoDupAccessor3" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %protoDupComputed()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "protoDupComputed" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %protoDupMethod1()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "protoDupMethod1" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %protoDupMethod2()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "protoDupMethod2" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %protoDupAccessor1()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "protoDupAccessor1" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %protoDupAccessor2()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "protoDupAccessor2" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %protoDupAccessor3()
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "protoDupAccessor3" : string
// CHECK-NEXT:  %18 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %18
// CHECK-NEXT:  %20 = LoadStackInst %18
// CHECK-NEXT:  %21 = ReturnInst %20
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadFrameInst [func]
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = StoreOwnPropertyInst null : null, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 42 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadFrameInst [func]
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %__proto__()
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5 : closure, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst 42 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %3 = CreateFunctionInst %"__proto__ 1#"()
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst %3 : closure, %2 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %5 = LoadFrameInst [func]
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %6
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst 42 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %9 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadFrameInst [func]
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %"get __proto__"()
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5 : closure, undefined : undefined, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadFrameInst [func]
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %"set __proto__"()
// CHECK-NEXT:  %6 = StoreGetterSetterInst undefined : undefined, %5 : closure, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %3 = CreateFunctionInst %"get __proto__ 1#"()
// CHECK-NEXT:  %4 = CreateFunctionInst %"set __proto__ 1#"()
// CHECK-NEXT:  %5 = StoreGetterSetterInst %3 : closure, %4 : closure, %2 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = LoadFrameInst [func]
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %7
// CHECK-NEXT:  %9 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function __proto__(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 33 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"(_)
// CHECK-NEXT:frame = [_]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %_
// CHECK-NEXT:  %1 = StoreFrameInst %0, [_]
// CHECK-NEXT:  %2 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
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
// CHECK-NEXT:  %0 = LoadParamInst %_
// CHECK-NEXT:  %1 = StoreFrameInst %0, [_]
// CHECK-NEXT:  %2 = ReturnInst 44 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
