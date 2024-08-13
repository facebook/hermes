/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
// Mov elimination incorrectly eliminated a mov in this case.

function fib(n) {
  var f0 = 0, f1 = 1;
  for (; n > 0; n = n -1) {
	var f2 = f0 + f1;
	f0 = f1; f1 = f2;
  }
  return f0;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "fib": string
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %fib(): functionCode
// CHECK-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "fib": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function fib(n: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg5 = LoadParamInst (:any) %n: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg6 = BinaryGreaterThanInst (:boolean) $Reg5, $Reg4
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg2 = MovInst (:number) $Reg3
// CHECK-NEXT:  $Reg1 = MovInst (:number) $Reg4
// CHECK-NEXT:  $Reg5 = MovInst (:any) $Reg5
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg1
// CHECK-NEXT:  $Reg6 = CondBranchInst $Reg6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg2 = PhiInst (:number) $Reg2, %BB0, $Reg2, %BB1
// CHECK-NEXT:  $Reg1 = PhiInst (:number) $Reg1, %BB0, $Reg1, %BB1
// CHECK-NEXT:  $Reg5 = PhiInst (:any) $Reg5, %BB0, $Reg5, %BB1
// CHECK-NEXT:  $Reg8 = FAddInst (:number) $Reg1, $Reg2
// CHECK-NEXT:  $Reg6 = BinarySubtractInst (:number) $Reg5, $Reg3
// CHECK-NEXT:  $Reg7 = MovInst (:number) $Reg2
// CHECK-NEXT:  $Reg2 = MovInst (:number) $Reg8
// CHECK-NEXT:  $Reg1 = MovInst (:number) $Reg7
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg1
// CHECK-NEXT:  $Reg5 = MovInst (:number) $Reg6
// CHECK-NEXT:  $Reg1 = HBCFCmpBrGreaterThanInst $Reg5, $Reg4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = PhiInst (:number) $Reg0, %BB0, $Reg0, %BB1
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end
