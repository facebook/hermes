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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg1, %foo(): functionCode
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg2, "foo": string
// CHECK-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = StoreStackInst $Reg2, $Reg1
// CHECK-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [a: any]

// CHECK:function foo(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS1: any, $Reg1
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.a]: any
// CHECK-NEXT:  $Reg2 = LoadFrameInst (:any) $Reg1, [%VS1.a]: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.a]: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end
