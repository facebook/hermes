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
// CHECK-NEXT:frame = [x: number|bigint]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %inner(): number|bigint
// CHECK-NEXT:       StoreFrameInst 0: number, [x]: number|bigint
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function inner(a: any, b: any, c: any): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %3 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: number|bigint
// CHECK-NEXT:       StoreFrameInst %4: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = UnaryIncInst (:number|bigint) %4: number|bigint
// CHECK-NEXT:       StoreFrameInst %7: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst (:number|bigint) %7: number|bigint, %BB1, %4: number|bigint, %BB0
// CHECK-NEXT:        CondBranchInst %1: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = UnaryIncInst (:number|bigint) %10: number|bigint
// CHECK-NEXT:        StoreFrameInst %12: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = PhiInst (:number|bigint) %12: number|bigint, %BB3, %10: number|bigint, %BB2
// CHECK-NEXT:        CondBranchInst %2: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %15: number|bigint
// CHECK-NEXT:        StoreFrameInst %17: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = PhiInst (:number|bigint) %17: number|bigint, %BB5, %15: number|bigint, %BB4
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: number|bigint
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:        StoreFrameInst %22: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:        ReturnInst %21: number|bigint
// CHECK-NEXT:function_end
