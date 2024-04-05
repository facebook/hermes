/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O -target=HBC %s | %FileCheckOrRegen %s --match-full-lines
var arr = [{a: 1}, {b: 2}, {c: 3}, {d: 4}];

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "arr": string
// CHECK-NEXT:  $Reg1 = AllocArrayInst (:object) 4: number
// CHECK-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) 1: number, "a": string, 1: number
// CHECK-NEXT:  $Reg0 = StoreOwnPropertyInst $Reg0, $Reg1, 0: number, true: boolean
// CHECK-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) 1: number, "b": string, 2: number
// CHECK-NEXT:  $Reg0 = StoreOwnPropertyInst $Reg0, $Reg1, 1: number, true: boolean
// CHECK-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) 1: number, "c": string, 3: number
// CHECK-NEXT:  $Reg0 = StoreOwnPropertyInst $Reg0, $Reg1, 2: number, true: boolean
// CHECK-NEXT:  $Reg0 = HBCAllocObjectFromBufferInst (:object) 1: number, "d": string, 4: number
// CHECK-NEXT:  $Reg0 = StoreOwnPropertyInst $Reg0, $Reg1, 3: number, true: boolean
// CHECK-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "arr": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end
