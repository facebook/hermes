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
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupComputed": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupMethod1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupMethod2": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor2": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor3": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %protoDupComputed(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "protoDupComputed": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %protoDupMethod1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "protoDupMethod1": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %protoDupMethod2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "protoDupMethod2": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %protoDupAccessor1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "protoDupAccessor1": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %protoDupAccessor2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "protoDupAccessor2": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %protoDupAccessor3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "protoDupAccessor3": string
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %18: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, %3: any
// CHECK-NEXT:       StoreOwnPropertyInst null: null, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %__proto__(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %5: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %"__proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %3: object, %2: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %7 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object, %6: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 42: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %"get __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %5: object, undefined: undefined, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, %3: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %"set __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst undefined: undefined, %5: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %"get __proto__ 1#"(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %"set __proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %3: object, %4: object, %2: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %8 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object, %7: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function __proto__(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %0: any, [_]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end

// CHECK:function "get __proto__ 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:function "set __proto__ 1#"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %0: any, [_]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end
