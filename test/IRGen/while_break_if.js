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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "continue_test": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "break_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "break_label": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "continue_label": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "nested_label": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %bar(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %continue_test(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "continue_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %break_test(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "break_test": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:closure) %break_label(): any
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: closure, globalObject: object, "break_label": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:closure) %continue_label(): any
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: closure, globalObject: object, "continue_label": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:closure) %nested_label(): any
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16: closure, globalObject: object, "nested_label": string
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %19 = StoreStackInst undefined: undefined, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %18: any
// CHECK-NEXT:  %21 = ReturnInst (:any) %20: any
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:any) 1: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function break_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function continue_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function nested_label(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %3 = CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %14 = CondBranchInst %13: any, %BB4, %BB5
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %15 = BranchInst %BB8
// CHECK-NEXT:function_end
