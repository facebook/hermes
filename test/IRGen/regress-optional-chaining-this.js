/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a) {
  return (a?.b.c)();
}

function f2(a) {
  return (a?.b?.c)();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "f1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f2": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [a: any]

// CHECK:function f1(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.a]: any
// CHECK-NEXT:  %5 = BinaryEqualInst (:any) %4: any, null: null
// CHECK-NEXT:       CondBranchInst %5: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst (:any) undefined: undefined, %BB2, %13: any, %BB3
// CHECK-NEXT:  %8 = PhiInst (:any) undefined: undefined, %BB2, %12: any, %BB3
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %4: any, "b": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %12: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [a: any]

// CHECK:function f2(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.a]: any
// CHECK-NEXT:  %5 = BinaryEqualInst (:any) %4: any, null: null
// CHECK-NEXT:       CondBranchInst %5: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst (:any) undefined: undefined, %BB2, %15: any, %BB4
// CHECK-NEXT:  %8 = PhiInst (:any) undefined: undefined, %BB2, %12: any, %BB4
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %4: any, "b": string
// CHECK-NEXT:  %13 = BinaryEqualInst (:any) %12: any, null: null
// CHECK-NEXT:        CondBranchInst %13: any, %BB2, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %12: any, "c": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
