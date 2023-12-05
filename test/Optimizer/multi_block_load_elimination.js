/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function outer(){
    var x = 0;
    function inner(a, b, c) {
        x++;
        if (a) x++;
        if (b) x++;
        if (c) x++;
        return x++;
    }
    return inner;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %outer(): object
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %inner(): number
// CHECK-NEXT:       StoreFrameInst 0: number, [x]: number
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function inner(a: any, b: any, c: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %3 = LoadFrameInst (:number) [x@outer]: number
// CHECK-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHECK-NEXT:       StoreFrameInst %4: number, [x@outer]: number
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:       StoreFrameInst %7: number, [x@outer]: number
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst (:number) %7: number, %BB1, %4: number, %BB0
// CHECK-NEXT:        CondBranchInst %1: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = FAddInst (:number) %10: number, 1: number
// CHECK-NEXT:        StoreFrameInst %12: number, [x@outer]: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = PhiInst (:number) %12: number, %BB3, %10: number, %BB2
// CHECK-NEXT:        CondBranchInst %2: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = FAddInst (:number) %15: number, 1: number
// CHECK-NEXT:        StoreFrameInst %17: number, [x@outer]: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = PhiInst (:number) %17: number, %BB5, %15: number, %BB4
// CHECK-NEXT:  %21 = FAddInst (:number) %20: number, 1: number
// CHECK-NEXT:        StoreFrameInst %21: number, [x@outer]: number
// CHECK-NEXT:        ReturnInst %20: number
// CHECK-NEXT:function_end
