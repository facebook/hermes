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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_label": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "bar": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %continue_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "continue_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %break_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "break_test": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %break_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "break_label": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %continue_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "continue_label": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %nested_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "nested_label": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %bar(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %continue_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %break_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function break_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %break_label(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function continue_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %continue_label(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB3
// CHECK-NEXT:function_end

// CHECK:function nested_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %nested_label(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB6, %BB7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB6, %BB7
// CHECK-NEXT:function_end
