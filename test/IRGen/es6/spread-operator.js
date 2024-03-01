/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(a, b, c) {
    return {a, ...b, c};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [c]: any
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:        StoreNewOwnPropertyInst %9: any, %8: object, "a": string, true: boolean
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: object, %11: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:        StoreOwnPropertyInst %13: any, %8: object, "c": string, true: boolean
// CHECK-NEXT:        ReturnInst %8: object
// CHECK-NEXT:function_end
