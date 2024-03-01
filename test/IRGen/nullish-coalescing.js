/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a, b) {
  return a ?? b;
}

function f2(a, b) {
  if (a ?? b) {
    return 1;
  } else {
    return 2;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "f1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f2": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function f1(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:       StoreStackInst %7: any, %6: any
// CHECK-NEXT:  %9 = BinaryEqualInst (:any) %7: any, null: null
// CHECK-NEXT:        CondBranchInst %9: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:        StoreStackInst %11: any, %6: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function f2(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %7 = BinaryEqualInst (:any) %6: any, null: null
// CHECK-NEXT:       CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst 2: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:        CondBranchInst %11: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:function_end
