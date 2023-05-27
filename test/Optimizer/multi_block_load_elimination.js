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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %outer(): object
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [x: number|bigint]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %inner(): number|bigint
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [x]: number|bigint
// CHECK-NEXT:  %2 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function inner(a: any, b: any, c: any): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %3 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:  %6 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %8 = UnaryIncInst (:number|bigint) %7: number|bigint
// CHECK-NEXT:  %9 = StoreFrameInst %8: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:  %10 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = CondBranchInst %1: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %13 = UnaryIncInst (:number|bigint) %12: number|bigint
// CHECK-NEXT:  %14 = StoreFrameInst %13: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:  %15 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = CondBranchInst %2: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %18 = UnaryIncInst (:number|bigint) %17: number|bigint
// CHECK-NEXT:  %19 = StoreFrameInst %18: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:  %20 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = LoadFrameInst (:number|bigint) [x@outer]: number|bigint
// CHECK-NEXT:  %22 = AsNumericInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:  %23 = UnaryIncInst (:number|bigint) %22: number|bigint
// CHECK-NEXT:  %24 = StoreFrameInst %23: number|bigint, [x@outer]: number|bigint
// CHECK-NEXT:  %25 = ReturnInst %22: number|bigint
// CHECK-NEXT:function_end
