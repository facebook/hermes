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
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "break_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "continue_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_label": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %bar(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %continue_test(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "continue_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %break_test(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "break_test": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %break_label(): any
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "break_label": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %continue_label(): any
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "continue_label": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %nested_label(): any
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "nested_label": string
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %18: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function break_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function continue_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function nested_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:        CondBranchInst %13: any, %BB4, %BB5
// CHECK-NEXT:function_end
