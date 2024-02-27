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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupComputed": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupMethod1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupMethod2": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor2": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoDupAccessor3": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %protoDupComputed(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "protoDupComputed": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %protoDupMethod1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "protoDupMethod1": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %protoDupMethod2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "protoDupMethod2": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %protoDupAccessor1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "protoDupAccessor1": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %protoDupAccessor2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "protoDupAccessor2": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %protoDupAccessor3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "protoDupAccessor3": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function protoDupComputed(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupComputed(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, %5: any
// CHECK-NEXT:       StoreOwnPropertyInst null: null, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 42: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupMethod1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, %5: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %__proto__(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %7: object, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 42: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function protoDupMethod2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupMethod2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %"__proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %5: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %9 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object, %8: any
// CHECK-NEXT:        StoreNewOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupAccessor1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 1: number, %5: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"get __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %7: object, undefined: undefined, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupAccessor2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 1: number, %5: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"set __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst undefined: undefined, %7: object, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function protoDupAccessor3(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoDupAccessor3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %"get __proto__ 1#"(): functionCode
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %"set __proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %5: object, %6: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object, %9: any
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function __proto__(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupMethod1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %__proto__(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function "__proto__ 1#"(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupMethod2(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"__proto__ 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function "get __proto__"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupAccessor1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"get __proto__"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:function "set __proto__"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupAccessor2(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"set __proto__"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [_]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end

// CHECK:function "get __proto__ 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupAccessor3(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"get __proto__ 1#"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:function "set __proto__ 1#"(_: any): any
// CHECK-NEXT:frame = [_: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %protoDupAccessor3(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"set __proto__ 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [_]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end
