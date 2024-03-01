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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %outer(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %outer(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %inner(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [x]: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function inner(a: any, b: any, c: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %4 = LoadFrameInst (:number) %0: environment, [x@outer]: number
// CHECK-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: number, [x@outer]: number
// CHECK-NEXT:       CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = FAddInst (:number) %5: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %8: number, [x@outer]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = PhiInst (:number) %8: number, %BB1, %5: number, %BB0
// CHECK-NEXT:        CondBranchInst %2: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %13: number, [x@outer]: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = PhiInst (:number) %13: number, %BB3, %11: number, %BB2
// CHECK-NEXT:        CondBranchInst %3: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = FAddInst (:number) %16: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %18: number, [x@outer]: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = PhiInst (:number) %18: number, %BB5, %16: number, %BB4
// CHECK-NEXT:  %22 = FAddInst (:number) %21: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %22: number, [x@outer]: number
// CHECK-NEXT:        ReturnInst %21: number
// CHECK-NEXT:function_end
