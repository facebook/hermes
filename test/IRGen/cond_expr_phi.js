/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function condExpr(a,b,c,d) {
    return a ? b || c : d;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "condExpr": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %condExpr(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "condExpr": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:function_end

// CHECK:function condExpr(a: any, b: any, c: any, d: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %d: any
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [d]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [d]: any
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %14 = StoreStackInst %13: any, %12: any
// CHECK-NEXT:  %15 = CondBranchInst %13: any, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = PhiInst (:any) %21: any, %BB4, %10: any, %BB2
// CHECK-NEXT:  %17 = ReturnInst (:any) %16: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:  %19 = StoreStackInst %18: any, %12: any
// CHECK-NEXT:  %20 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %22 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
