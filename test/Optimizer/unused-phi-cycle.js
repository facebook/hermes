/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test that we do not emit cycles of unused Phis in SimpleMem2Reg.

function foo(sink) {
  for (var i = 0; i < sink; i++) {
    sink();
    if (sink) {
      // This assignment results in a definition that would have produced a Phi
      // in the old Mem2Reg. However, this Phi would be unused, since the only
      // read of x is dominated by this assignment.
      var x = sink();
      if (sink() || x) {
        sink();
      }
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(sink: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %1 = BinaryLessThanInst (:boolean) 0: number, %0: any
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:any) undefined: undefined, %BB0, %8: any, %BB3
// CHECK-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %9: number, %BB3
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %0: any, %BB4, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = PhiInst (:any) %12: any, %BB6, %3: any, %BB1, %12: any, %BB5
// CHECK-NEXT:  %9 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %10 = BinaryLessThanInst (:boolean) %9: number, %0: any
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        CondBranchInst %13: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        CondBranchInst %12: any, %BB5, %BB3
// CHECK-NEXT:function_end
