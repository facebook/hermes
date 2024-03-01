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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "condExpr": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %condExpr(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "condExpr": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function condExpr(a: any, b: any, c: any, d: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %condExpr(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [c]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %d: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [d]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [d]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:        StoreStackInst %15: any, %14: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB5, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = PhiInst (:any) %23: any, %BB5, %12: any, %BB1
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:        StoreStackInst %20: any, %14: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %23 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end
