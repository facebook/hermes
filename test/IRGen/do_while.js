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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple_do_while_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "do_while_break_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "do_while_continue_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_while_do_mixed_test": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %simple_do_while_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "simple_do_while_test": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %do_while_break_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "do_while_break_test": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %do_while_continue_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "do_while_continue_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %for_while_do_mixed_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "for_while_do_mixed_test": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function simple_do_while_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_do_while_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function do_while_break_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %do_while_break_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function do_while_continue_test(cond: any): any
// CHECK-NEXT:frame = [cond: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %do_while_continue_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function for_while_do_mixed_test(cond: any): any
// CHECK-NEXT:frame = [cond: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %for_while_do_mixed_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [cond]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: any, 10: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %12: any, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %16 = AsNumericInst (:number|bigint) %15: any
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [cond]: any
// CHECK-NEXT:        CondBranchInst %24: any, %BB7, %BB8
// CHECK-NEXT:function_end
