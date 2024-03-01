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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %sink(): functionCode
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg3 = StorePropertyLooseInst $Reg1, $Reg2, "sink": string
// CHECK-NEXT:  $Reg3 = CreateFunctionInst (:object) $Reg0, %foo(): functionCode
// CHECK-NEXT:  $Reg4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg5 = StorePropertyLooseInst $Reg3, $Reg4, "foo": string
// CHECK-NEXT:  $Reg5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  $Reg6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg7 = StoreStackInst $Reg6, $Reg5
// CHECK-NEXT:  $Reg7 = LoadStackInst (:any) $Reg5
// CHECK-NEXT:  $Reg8 = ReturnInst $Reg7
// CHECK-NEXT:function_end

// CHECK:function sink(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %sink(): any, $Reg0
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg3 = StoreFrameInst $Reg1, $Reg2, [x]: any
// CHECK-NEXT:  $Reg3 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  $Reg4 = StoreFrameInst $Reg1, $Reg3, [y]: any
// CHECK-NEXT:  $Reg4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  $Reg5 = StoreFrameInst $Reg1, $Reg4, [z]: any
// CHECK-NEXT:  $Reg5 = HBCLoadConstInst (:undefined) undefined: undefined
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
// CHECK-NEXT:  $Reg4 = LoadPropertyInst (:any) $Reg3, "sink": string
// CHECK-NEXT:  $Reg5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg6 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg7 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg8 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg9 = HBCCallNInst (:any) $Reg4, empty: any, empty: any, $Reg5, $Reg3, $Reg6, $Reg7, $Reg8
// CHECK-NEXT:  $Reg9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg10 = ReturnInst $Reg9
// CHECK-NEXT:function_end
