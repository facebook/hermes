/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function foo(a) {
  a = a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %foo(): functionCode
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg3 = StorePropertyLooseInst $Reg1, $Reg2, "foo": string
// CHECK-NEXT:  $Reg3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg5 = StoreStackInst $Reg4, $Reg3
// CHECK-NEXT:  $Reg5 = LoadStackInst (:any) $Reg3
// CHECK-NEXT:  $Reg6 = ReturnInst $Reg5
// CHECK-NEXT:function_end

// CHECK:function foo(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %foo(): any, $Reg0
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $Reg3 = StoreFrameInst $Reg1, $Reg2, [a]: any
// CHECK-NEXT:  $Reg3 = LoadFrameInst (:any) $Reg1, [a]: any
// CHECK-NEXT:  $Reg4 = StoreFrameInst $Reg1, $Reg3, [a]: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg5 = ReturnInst $Reg4
// CHECK-NEXT:function_end
