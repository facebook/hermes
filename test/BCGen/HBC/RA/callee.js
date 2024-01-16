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
// CHECK-NEXT:  $Reg0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = HBCCreateFunctionInst (:object) %sink(): functionCode, $Reg0
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg3 = StorePropertyLooseInst $Reg1, $Reg2, "sink": string
// CHECK-NEXT:  $Reg3 = HBCCreateFunctionInst (:object) %foo(): functionCode, $Reg0
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
// CHECK-NEXT:  $Reg0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg2 = HBCStoreToEnvironmentInst $Reg0, $Reg1, [x]: any
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  $Reg3 = HBCStoreToEnvironmentInst $Reg0, $Reg2, [y]: any
// CHECK-NEXT:  $Reg3 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  $Reg4 = HBCStoreToEnvironmentInst $Reg0, $Reg3, [z]: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg5 = ReturnInst $Reg4
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg2 = HBCStoreToEnvironmentInst $Reg0, $Reg1, [x]: any
// CHECK-NEXT:  $Reg2 = HBCLoadFromEnvironmentInst (:any) $Reg0, [x]: any
// CHECK-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg2, "sink": string
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg5 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg6 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg7 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg8 = HBCCallNInst (:any) $Reg3, empty: any, empty: any, $Reg4, $Reg2, $Reg5, $Reg6, $Reg7
// CHECK-NEXT:  $Reg8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg9 = ReturnInst $Reg8
// CHECK-NEXT:function_end
