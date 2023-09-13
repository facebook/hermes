/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -hermes-parser -dump-ir %s -O

var x = {
    1: 10,
    get a() { return "a" },
    get 1() { return 20; },
    b: 11,
    set 1(x) {},
    get 1() { return 21; },
    b: 12,
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 3: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %3: object, "1": string, true: boolean
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %"get a"(): any
// CHECK-NEXT:       StoreGetterSetterInst %5: object, undefined: undefined, %3: object, "a": string, true: boolean
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %"get 1"(): any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %"set 1"(): any
// CHECK-NEXT:       StoreGetterSetterInst %7: object, %8: object, %3: object, "1": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst null: null, %3: object, "b": string, true: boolean
// CHECK-NEXT:        StoreOwnPropertyInst 12: number, %3: object, "b": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst %3: object, globalObject: object, "x": string
// CHECK-NEXT:  %13 = LoadStackInst (:any) %1: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function "get a"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst "a": string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function "get 1"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 21: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function "set 1"(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
