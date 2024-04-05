/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Test if we can eliminate loads from a captured variable across basic blocks
// when the loads occur before the capture.
// TODO: We currently do not perform this optimisation.
function foo(sink){
    var x = 0;
    if (a) sink(x++);
    if (b) sink(x++);
    if (c) sink(x++);
    return () => x++;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: number]

// CHECK:function foo(sink: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.x]: number
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst (:number) %1: environment, [%VS1.x]: number
// CHECK-NEXT:  %7 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: number, [%VS1.x]: number
// CHECK-NEXT:  %9 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:        CondBranchInst %11: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:number) %1: environment, [%VS1.x]: number
// CHECK-NEXT:  %14 = FAddInst (:number) %13: number, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: number, [%VS1.x]: number
// CHECK-NEXT:  %16 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:        CondBranchInst %18: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:number) %1: environment, [%VS1.x]: number
// CHECK-NEXT:  %21 = FAddInst (:number) %20: number, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: number, [%VS1.x]: number
// CHECK-NEXT:  %23 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %20: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:        ReturnInst %25: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.x]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [%VS1.x]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
