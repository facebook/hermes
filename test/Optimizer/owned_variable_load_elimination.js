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

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [x: number]

// CHECK:function foo(sink: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.x]: number
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %0: environment, [%VS0.x]: number
// CHECK-NEXT:  %6 = FAddInst (:number) %5: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %6: number, [%VS0.x]: number
// CHECK-NEXT:  %8 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: number
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:        CondBranchInst %10: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:number) %0: environment, [%VS0.x]: number
// CHECK-NEXT:  %13 = FAddInst (:number) %12: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %13: number, [%VS0.x]: number
// CHECK-NEXT:  %15 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:        CondBranchInst %17: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = LoadFrameInst (:number) %0: environment, [%VS0.x]: number
// CHECK-NEXT:  %20 = FAddInst (:number) %19: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %20: number, [%VS0.x]: number
// CHECK-NEXT:  %22 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %19: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:        ReturnInst %24: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS0.x]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [%VS0.x]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
