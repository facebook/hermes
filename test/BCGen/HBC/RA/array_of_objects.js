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
// CHECK-NEXT:                 DeclareGlobalVarInst "arr": string
// CHECK-NEXT:  {r1}      %1 = AllocArrayInst (:object) 4: number
// CHECK-NEXT:  {r0}      %2 = HBCAllocObjectFromBufferInst (:object) "a": string, 1: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r0} %2: object, {r1} %1: object, 0: number, true: boolean
// CHECK-NEXT:  {r0}      %4 = HBCAllocObjectFromBufferInst (:object) "b": string, 2: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r0} %4: object, {r1} %1: object, 1: number, true: boolean
// CHECK-NEXT:  {r0}      %6 = HBCAllocObjectFromBufferInst (:object) "c": string, 3: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r0} %6: object, {r1} %1: object, 2: number, true: boolean
// CHECK-NEXT:  {r0}      %8 = HBCAllocObjectFromBufferInst (:object) "d": string, 4: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r0} %8: object, {r1} %1: object, 3: number, true: boolean
// CHECK-NEXT:  {r0}     %10 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %1: object, {r0} %10: object, "arr": string
// CHECK-NEXT:  {np0}    %12 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %12: undefined
// CHECK-NEXT:function_end
