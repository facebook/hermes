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
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): object
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(sink: any): object
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst 0: number, [x]: number
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       StoreFrameInst 1: number, [x]: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:       CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst (:number) [x]: number
// CHECK-NEXT:  %10 = FAddInst (:number) %9: number, 1: number
// CHECK-NEXT:        StoreFrameInst %10: number, [x]: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "c": string
// CHECK-NEXT:        CondBranchInst %14: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %16 = LoadFrameInst (:number) [x]: number
// CHECK-NEXT:  %17 = FAddInst (:number) %16: number, 1: number
// CHECK-NEXT:        StoreFrameInst %17: number, [x]: number
// CHECK-NEXT:  %19 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %16: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %""(): number
// CHECK-NEXT:        ReturnInst %21: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number) [x@foo]: number
// CHECK-NEXT:  %1 = FAddInst (:number) %0: number, 1: number
// CHECK-NEXT:       StoreFrameInst %1: number, [x@foo]: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end
