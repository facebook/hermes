/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen --check-prefix=CHKIR --match-full-lines %s
// RUN: %shermes -dump-lir %s | %FileCheckOrRegen --check-prefix=CHKLIR --match-full-lines %s

function foo(i) {
  switch (i) {
    case 0:
      i = 2;
    case 1:
      break;
  }
  return i;
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function foo(i: any): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %i: any
// CHKIR-NEXT:       SwitchInst %0: any, %BB1, 0: number, %BB2, 1: number, %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:any) 2: number, %BB2, %0: any, %BB0
// CHKIR-NEXT:       ReturnInst %2: any
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:function_end

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLIR-NEXT:  %1 = HBCCreateEnvironmentInst (:environment)
// CHKLIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %foo(): any, %1: environment
// CHKLIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       ReturnInst %5: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function foo(i: any): any
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCLoadConstInst (:number) 2: number
// CHKLIR-NEXT:  %1 = LoadParamInst (:any) %i: any
// CHKLIR-NEXT:       BranchInst %BB1
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %3 = PhiInst (:any) %0: number, %BB3, %1: any, %BB4
// CHKLIR-NEXT:       ReturnInst %3: any
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:       BranchInst %BB2
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:       CmpBrStrictlyEqualInst %6: number, %1: any, %BB2, %BB2
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %8 = HBCLoadConstInst (:number) 0: number
// CHKLIR-NEXT:       CmpBrStrictlyEqualInst %8: number, %1: any, %BB3, %BB4
// CHKLIR-NEXT:function_end
