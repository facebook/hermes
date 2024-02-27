/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that __proto__ is not set as an "own" property.

// __proto__ is first property so it can be used as a parent.
function protoIsFirst(func) {
  return {__proto__: func(), a: 2, b: 3};
}

// __proto__ is a constant so it can be used as a parent.
function protoIsConst1() {
  return {a: 2, __proto__: null};
}

function protoIsConst2() {
  return {b: 3, __proto__: 10};
}

function protoIsConst3() {
  return {__proto__: null, c: 4};
}

// We must set it dynamically.
function protoIsDynamic(func, getParent) {
  return {a: func(), b: 10, __proto__: getParent()};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "protoIsFirst": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoIsConst1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoIsConst2": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoIsConst3": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoIsDynamic": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %protoIsFirst(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "protoIsFirst": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %protoIsConst1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "protoIsConst1": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %protoIsConst2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "protoIsConst2": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %protoIsConst3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "protoIsConst3": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %protoIsDynamic(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "protoIsDynamic": string
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:  %18 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:function_end

// CHECK:function protoIsFirst(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoIsFirst(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, %5: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %6: object, "b": string, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function protoIsConst1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoIsConst1(): any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function protoIsConst2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoIsConst2(): any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 1: number, 10: number
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %2: object, "b": string, true: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function protoIsConst3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoIsConst3(): any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:       StoreNewOwnPropertyInst 4: number, %2: object, "c": string, true: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function protoIsDynamic(func: any, getParent: any): any
// CHECK-NEXT:frame = [func: any, getParent: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoIsDynamic(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %getParent: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [getParent]: any
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreNewOwnPropertyInst %8: any, %6: object, "a": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst 10: number, %6: object, "b": string, true: boolean
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [getParent]: any
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: object, %12: any
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end
