/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

var const_array = [1, 2, 3, "a"];

var t = 1;

var exp_array = [t, 1, t + 1, 2, t + 3];

var elision_array = [,,,"b"];

var const_then_elision_array = [1, 2, 3,,,"b"];

var exp_then_elision_array = [1, 2, t, t + 1,,,"b"];

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "const_array": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "t": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "exp_array": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "elision_array": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "const_then_elision_array": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "exp_then_elision_array": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 4: number, 1: number, 2: number, 3: number, "a": string
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "const_array": string
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg4, $Reg1, "t": string
// CHECK-NEXT:  $Reg2 = LoadPropertyInst (:any) $Reg1, "t": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 5: number
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg2, $Reg0, 0: number, true: boolean
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg4, $Reg0, 1: number, true: boolean
// CHECK-NEXT:  $Reg2 = LoadPropertyInst (:any) $Reg1, "t": string
// CHECK-NEXT:  $Reg2 = BinaryAddInst (:string|number) $Reg2, $Reg4
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg2, $Reg0, 2: number, true: boolean
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg2, $Reg0, 3: number, true: boolean
// CHECK-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg1, "t": string
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg2 = BinaryAddInst (:string|number) $Reg3, $Reg2
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg2, $Reg0, 4: number, true: boolean
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "exp_array": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 4: number
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:string) "b": string
// CHECK-NEXT:  $Reg3 = StoreOwnPropertyInst $Reg2, $Reg0, 3: number, true: boolean
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "elision_array": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 6: number, 1: number, 2: number, 3: number
// CHECK-NEXT:  $Reg3 = StoreOwnPropertyInst $Reg2, $Reg0, 5: number, true: boolean
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "const_then_elision_array": string
// CHECK-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg1, "t": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 7: number, 1: number, 2: number
// CHECK-NEXT:  $Reg3 = StoreOwnPropertyInst $Reg3, $Reg0, 2: number, true: boolean
// CHECK-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg1, "t": string
// CHECK-NEXT:  $Reg3 = BinaryAddInst (:string|number) $Reg3, $Reg4
// CHECK-NEXT:  $Reg3 = StoreOwnPropertyInst $Reg3, $Reg0, 3: number, true: boolean
// CHECK-NEXT:  $Reg2 = StoreOwnPropertyInst $Reg2, $Reg0, 6: number, true: boolean
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "exp_then_elision_array": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end
