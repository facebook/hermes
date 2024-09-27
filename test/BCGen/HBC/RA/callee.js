/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function sink(x,y,z) {}

function foo(x) {
  x.sink(1,2,3)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg1, %sink(): functionCode
// CHECK-NEXT:  $Reg3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg2, $Reg3, "sink": string
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

// CHECK:scope %VS1 [x: any, y: any, z: any]

// CHECK:function sink(x: any, y: any, z: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS1: any, $Reg1
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.x]: any
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.y]: any
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.z]: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  $Reg2 = CreateScopeInst (:environment) %VS1: any, $Reg2
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg2, $Reg1, [%VS1.x]: any
// CHECK-NEXT:  $Reg2 = LoadFrameInst (:any) $Reg2, [%VS1.x]: any
// CHECK-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg2, "sink": string
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg5 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg0 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg3, $Reg4, $Reg5
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end
