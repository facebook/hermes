/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function simple_do_while_test(cond) {
  do {
  } while (cond);
}

function do_while_break_test(cond) {
  do {
   break;
  } while (cond);
}

function do_while_continue_test(cond) {
  do {
    continue;
  } while (cond);
}

function for_while_do_mixed_test(cond) {
  for (var i = 0; i < 10; i++) {
    while (cond) {
      do {
        continue;
      } while (cond);
      break;
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple_do_while_test": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "do_while_break_test": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "do_while_continue_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "for_while_do_mixed_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %simple_do_while_test(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "simple_do_while_test": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %do_while_break_test(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: object, globalObject: object, "do_while_break_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %do_while_continue_test(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "do_while_continue_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %for_while_do_mixed_test(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "for_while_do_mixed_test": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function simple_do_while_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB3
// CHECK-NEXT:function_end

// CHECK:function do_while_break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function do_while_continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function for_while_do_mixed_test(cond: any): any
// CHECK-NEXT:frame = [cond: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %3 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:any) %4: any, 10: number
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:any) %10: any, 10: number
// CHECK-NEXT:  %12 = CondBranchInst %11: any, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = AsNumericInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = UnaryIncInst (:any) %14: number|bigint
// CHECK-NEXT:  %16 = StoreFrameInst %15: any, [i]: any
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %21 = CondBranchInst %20: any, %BB3, %BB4
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %22 = BranchInst %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %23 = BranchInst %BB4
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:  %25 = CondBranchInst %24: any, %BB7, %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %26 = BranchInst %BB9
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %27 = BranchInst %BB8
// CHECK-NEXT:function_end
