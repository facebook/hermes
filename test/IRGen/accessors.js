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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 3: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %4: object, "1": string, true: boolean
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %"get a"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %6: object, undefined: undefined, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %"get 1"(): functionCode
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %"set 1"(): functionCode
// CHECK-NEXT:        StoreGetterSetterInst %8: object, %9: object, %4: object, "1": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst null: null, %4: object, "b": string, true: boolean
// CHECK-NEXT:        StoreOwnPropertyInst 12: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst %4: object, globalObject: object, "x": string
// CHECK-NEXT:  %14 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function "get a"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"get a"(): any, %0: environment
// CHECK-NEXT:       ReturnInst "a": string
// CHECK-NEXT:function_end

// CHECK:function "get 1"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"get 1"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 21: number
// CHECK-NEXT:function_end

// CHECK:function "set 1"(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"set 1"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
