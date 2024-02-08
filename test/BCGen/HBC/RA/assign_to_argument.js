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
// CHECK-NEXT:  $Reg0 = HBCCreateFunctionEnvironmentInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = HBCCreateFunctionInst (:object) %foo(): functionCode, $Reg0
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
// CHECK-NEXT:  $Reg0 = HBCCreateFunctionEnvironmentInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $Reg2 = HBCStoreToEnvironmentInst $Reg0, $Reg1, [a]: any
// CHECK-NEXT:  $Reg2 = HBCLoadFromEnvironmentInst (:any) $Reg0, [a]: any
// CHECK-NEXT:  $Reg3 = HBCStoreToEnvironmentInst $Reg0, $Reg2, [a]: any
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg4 = ReturnInst $Reg3
// CHECK-NEXT:function_end
