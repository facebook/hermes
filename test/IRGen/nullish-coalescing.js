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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %f1(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "f1": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %f2(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "f2": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function f1(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %6 = StoreStackInst %5: any, %4: any
// CHECK-NEXT:  %7 = BinaryEqualInst (:any) %5: any, null: null
// CHECK-NEXT:  %8 = CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %4: any
// CHECK-NEXT:  %11 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f2(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %5 = BinaryEqualInst (:any) %4: any, null: null
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst 1: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = ReturnInst 2: number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %11 = CondBranchInst %10: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = CondBranchInst %4: any, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:function_end
