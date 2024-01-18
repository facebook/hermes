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
// CHECK-NEXT:       DeclareGlobalVarInst "simple_do_while_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "do_while_break_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "do_while_continue_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_while_do_mixed_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %simple_do_while_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "simple_do_while_test": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %do_while_break_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "do_while_break_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %do_while_continue_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "do_while_continue_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %for_while_do_mixed_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "for_while_do_mixed_test": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function simple_do_while_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB3
// CHECK-NEXT:function_end

// CHECK:function do_while_break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function do_while_continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB3
// CHECK-NEXT:function_end

// CHECK:function for_while_do_mixed_test(cond: any): any
// CHECK-NEXT:frame = [cond: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %0: any, [cond]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = AsNumericInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = UnaryIncInst (:any) %14: number|bigint
// CHECK-NEXT:        StoreFrameInst %15: any, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [cond]: any
// CHECK-NEXT:        CondBranchInst %22: any, %BB7, %BB9
// CHECK-NEXT:function_end
