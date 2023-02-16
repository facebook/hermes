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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "protoIsFirst": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "protoIsConst1": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "protoIsConst2": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "protoIsConst3": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "protoIsDynamic": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %protoIsFirst(): any
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: closure, globalObject: object, "protoIsFirst": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %protoIsConst1(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "protoIsConst1": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %protoIsConst2(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "protoIsConst2": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %protoIsConst3(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "protoIsConst3": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:closure) %protoIsDynamic(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: closure, globalObject: object, "protoIsDynamic": string
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %16 = StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %18 = ReturnInst (:any) %17: any
// CHECK-NEXT:function_end

// CHECK:function protoIsFirst(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, %3: any
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst (:any) %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 2: number, %0: object, "a": string, true: boolean
// CHECK-NEXT:  %2 = ReturnInst (:any) %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst (:object) 1: number, 10: number
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 3: number, %0: object, "b": string, true: boolean
// CHECK-NEXT:  %2 = ReturnInst (:any) %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 4: number, %0: object, "c": string, true: boolean
// CHECK-NEXT:  %2 = ReturnInst (:any) %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsDynamic(func: any, getParent: any): any
// CHECK-NEXT:frame = [func: any, getParent: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %getParent: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [getParent]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %6: any, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst 10: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [getParent]: any
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %11 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, %4: object, %10: any
// CHECK-NEXT:  %12 = ReturnInst (:any) %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
