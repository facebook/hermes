/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Test if we can eliminate loads from a captured variable across basic blocks
// when the loads occur before the capture.
function foo(sink){
    var x = 0;
    if (a) sink(x++);
    if (b) sink(x++);
    if (c) sink(x++);
    return () => x++;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(sink: any): object
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [x]: number
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       StoreFrameInst %1: environment, 1: number, [x]: number
// CHECK-NEXT:  %7 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:        CondBranchInst %9: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:number) %1: environment, [x]: number
// CHECK-NEXT:  %12 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: number, [x]: number
// CHECK-NEXT:  %14 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:        CondBranchInst %16: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = LoadFrameInst (:number) %1: environment, [x]: number
// CHECK-NEXT:  %19 = FAddInst (:number) %18: number, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: number, [x]: number
// CHECK-NEXT:  %21 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %18: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:        ReturnInst %23: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [x@foo]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [x@foo]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
