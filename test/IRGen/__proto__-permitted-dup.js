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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "protoDupComputed": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "protoDupMethod1": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "protoDupMethod2": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "protoDupAccessor1": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "protoDupAccessor2": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "protoDupAccessor3": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %protoDupComputed(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "protoDupComputed": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %protoDupMethod1(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "protoDupMethod1": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %protoDupMethod2(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "protoDupMethod2": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:closure) %protoDupAccessor1(): any
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: closure, globalObject: object, "protoDupAccessor1": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:closure) %protoDupAccessor2(): any
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: closure, globalObject: object, "protoDupAccessor2": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:closure) %protoDupAccessor3(): any
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16: closure, globalObject: object, "protoDupAccessor3": string
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %19 = StoreStackInst undefined: undefined, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %18: any
// CHECK-NEXT:  %21 = ReturnInst %20: any
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, %3: any
// CHECK-NEXT:  %5 = StoreOwnPropertyInst null: null, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %__proto__(): any
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5: closure, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %8 = ReturnInst %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %"__proto__ 1#"(): any
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst %3: closure, %2: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %7 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, %2: object, %6: any
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst 42: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %9 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %"get __proto__"(): any
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5: closure, undefined: undefined, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %"set __proto__"(): any
// CHECK-NEXT:  %6 = StoreGetterSetterInst undefined: undefined, %5: closure, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %"get __proto__ 1#"(): any
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %"set __proto__ 1#"(): any
// CHECK-NEXT:  %5 = StoreGetterSetterInst %3: closure, %4: closure, %2: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %8 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, %2: object, %7: any
// CHECK-NEXT:  %9 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function __proto__(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 33: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %_: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [_]: any
// CHECK-NEXT:  %2 = ReturnInst 44: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "get __proto__ 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 33: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "set __proto__ 1#"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %_: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [_]: any
// CHECK-NEXT:  %2 = ReturnInst 44: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
