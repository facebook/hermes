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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "const_array": string
// CHECK-NEXT:                 DeclareGlobalVarInst "t": string
// CHECK-NEXT:                 DeclareGlobalVarInst "exp_array": string
// CHECK-NEXT:                 DeclareGlobalVarInst "elision_array": string
// CHECK-NEXT:                 DeclareGlobalVarInst "const_then_elision_array": string
// CHECK-NEXT:                 DeclareGlobalVarInst "exp_then_elision_array": string
// CHECK-NEXT:  {r0}      %6 = AllocArrayInst (:object) 4: number, 1: number, 2: number, 3: number, "a": string
// CHECK-NEXT:  {r1}      %7 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %6: object, {r1} %7: object, "const_array": string
// CHECK-NEXT:  {n0}      %9 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:                 StorePropertyLooseInst {n0} %9: number, {r1} %7: object, "t": string
// CHECK-NEXT:  {r2}     %11 = LoadPropertyInst (:any) {r1} %7: object, "t": string
// CHECK-NEXT:  {r0}     %12 = AllocArrayInst (:object) 5: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %11: any, {r0} %12: object, 0: number, true: boolean
// CHECK-NEXT:                 StoreOwnPropertyInst {n0} %9: number, {r0} %12: object, 1: number, true: boolean
// CHECK-NEXT:  {r2}     %15 = LoadPropertyInst (:any) {r1} %7: object, "t": string
// CHECK-NEXT:  {r2}     %16 = BinaryAddInst (:string|number) {r2} %15: any, {n0} %9: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %16: string|number, {r0} %12: object, 2: number, true: boolean
// CHECK-NEXT:  {n1}     %18 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:                 StoreOwnPropertyInst {n1} %18: number, {r0} %12: object, 3: number, true: boolean
// CHECK-NEXT:  {r2}     %20 = LoadPropertyInst (:any) {r1} %7: object, "t": string
// CHECK-NEXT:  {n1}     %21 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {r2}     %22 = BinaryAddInst (:string|number) {r2} %20: any, {n1} %21: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %22: string|number, {r0} %12: object, 4: number, true: boolean
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %12: object, {r1} %7: object, "exp_array": string
// CHECK-NEXT:  {r0}     %25 = AllocArrayInst (:object) 4: number
// CHECK-NEXT:  {r2}     %26 = HBCLoadConstInst (:string) "b": string
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %26: string, {r0} %25: object, 3: number, true: boolean
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %25: object, {r1} %7: object, "elision_array": string
// CHECK-NEXT:  {r0}     %29 = AllocArrayInst (:object) 6: number, 1: number, 2: number, 3: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %26: string, {r0} %29: object, 5: number, true: boolean
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %29: object, {r1} %7: object, "const_then_elision_array": string
// CHECK-NEXT:  {r3}     %32 = LoadPropertyInst (:any) {r1} %7: object, "t": string
// CHECK-NEXT:  {r0}     %33 = AllocArrayInst (:object) 7: number, 1: number, 2: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r3} %32: any, {r0} %33: object, 2: number, true: boolean
// CHECK-NEXT:  {r3}     %35 = LoadPropertyInst (:any) {r1} %7: object, "t": string
// CHECK-NEXT:  {r3}     %36 = BinaryAddInst (:string|number) {r3} %35: any, {n0} %9: number
// CHECK-NEXT:                 StoreOwnPropertyInst {r3} %36: string|number, {r0} %33: object, 3: number, true: boolean
// CHECK-NEXT:                 StoreOwnPropertyInst {r2} %26: string, {r0} %33: object, 6: number, true: boolean
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %33: object, {r1} %7: object, "exp_then_elision_array": string
// CHECK-NEXT:  {np0}    %40 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %40: undefined
// CHECK-NEXT:function_end
