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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHECK:scope %VS1 [func: any]

// CHECK:function protoDupComputed(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) %5: any
// CHECK-NEXT:       StoreOwnPropertyInst null: null, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 42: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [func: any, ?obj: object]

// CHECK:function protoDupMethod1(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) %5: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS2.?obj]: object
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %__proto__(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %8: object, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst 42: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [func: any, ?obj: object]

// CHECK:function protoDupMethod2(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.func]: any
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS3.?obj]: object
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %"__proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreNewOwnPropertyInst %6: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS3.func]: any
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object, %9: any
// CHECK-NEXT:        StoreNewOwnPropertyInst 42: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [func: any]

// CHECK:function protoDupAccessor1(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS4.func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) %5: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"get __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %7: object, undefined: undefined, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [func: any]

// CHECK:function protoDupAccessor2(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS5.func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS5.func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) %5: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"set __proto__"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst undefined: undefined, %7: object, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [func: any]

// CHECK:function protoDupAccessor3(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS6.func]: any
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %"get __proto__ 1#"(): functionCode
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %"set __proto__ 1#"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %5: object, %6: object, %4: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS6.func]: any
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object, %9: any
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [x: any, y: any]

// CHECK:method __proto__(x: any, y: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS7.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS7.y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS7.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS7.y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS8 [x: any, y: any]

// CHECK:method "__proto__ 1#"(x: any, y: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS8.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS8.y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS8.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS8.y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS9 []

// CHECK:function "get __proto__"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:scope %VS10 [_: any]

// CHECK:function "set __proto__"(_: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS5: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS10: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS10._]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end

// CHECK:scope %VS11 []

// CHECK:function "get __proto__ 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS11: any, %0: environment
// CHECK-NEXT:       ReturnInst 33: number
// CHECK-NEXT:function_end

// CHECK:scope %VS12 [_: any]

// CHECK:function "set __proto__ 1#"(_: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS12: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %_: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS12._]: any
// CHECK-NEXT:       ReturnInst 44: number
// CHECK-NEXT:function_end
