/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function bar() { return 1 }

function continue_test(cond) {
  while (cond) { continue; }
}

function break_test(cond) {
  while (cond) { break; }
}

function break_label(cond) {
  fail:
  while (cond) { break fail; }
}

function continue_label(cond) {
  fail:
  while (cond) { continue fail; }
}

function nested_label(cond) {
fail1:
  while (cond) {

fail2:
    while (cond) { continue fail2; }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_label": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS0: any, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "bar": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %continue_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "continue_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %break_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "break_test": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %break_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "break_label": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %VS0: any, %continue_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "continue_label": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %VS0: any, %nested_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "nested_label": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function bar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [cond: any]

// CHECK:function continue_test(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS2.cond]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [cond: any]

// CHECK:function break_test(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS3.cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [cond: any]

// CHECK:function break_label(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS4.cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [cond: any]

// CHECK:function continue_label(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS5.cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS5.cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS5.cond]: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [cond: any]

// CHECK:function nested_label(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS6.cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS6.cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS6.cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB6, %BB7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS6.cond]: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS6.cond]: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB6, %BB7
// CHECK-NEXT:function_end
