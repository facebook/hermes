/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-lra %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-lra %s | %FileCheckOrRegen --match-full-lines  --check-prefix=CHKOPT %s

// Check that literals are uniqued when optimizations is disabled, but aren't
// when it is enabled.

var a, b;

function foo(x) {
  if (x)
    a = 10;
  else
    b = 10;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %foo(): functionCode
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg3 = StorePropertyLooseInst $Reg1, $Reg2, "foo": string
// CHECK-NEXT:  $Reg3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg3 = MovInst (:undefined) $Reg4
// CHECK-NEXT:  $Reg5 = LoadStackInst (:any) $Reg3
// CHECK-NEXT:  $Reg6 = ReturnInst $Reg5
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %foo(): any, $Reg0
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg3 = StoreFrameInst $Reg1, $Reg2, [x]: any
// CHECK-NEXT:  $Reg3 = LoadFrameInst (:any) $Reg1, [x]: any
// CHECK-NEXT:  $Reg4 = CondBranchInst $Reg3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg0, $Reg1, "a": string
// CHECK-NEXT:  $Reg2 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg0, $Reg1, "b": string
// CHECK-NEXT:  $Reg2 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  $Reg0 = DeclareGlobalVarInst "a": string
// CHKOPT-NEXT:  $Reg0 = DeclareGlobalVarInst "b": string
// CHKOPT-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHKOPT-NEXT:  $Reg0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKOPT-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %foo(): functionCode
// CHKOPT-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "foo": string
// CHKOPT-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(x: any): undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  $Reg1 = HBCLoadConstInst (:number) 10: number
// CHKOPT-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHKOPT-NEXT:  $Reg2 = CondBranchInst $Reg2, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "a": string
// CHKOPT-NEXT:  $Reg0 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  $Reg2 = StorePropertyLooseInst $Reg1, $Reg0, "b": string
// CHKOPT-NEXT:  $Reg2 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKOPT-NEXT:function_end
