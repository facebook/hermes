/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

var a = [];
var x = {};
var y = {}

for (var i=0 ; i < 3; ++i) {
  y = {}

  for (a[2] in x) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined|object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "a": string
// CHECK-NEXT:                 DeclareGlobalVarInst "x": string
// CHECK-NEXT:                 DeclareGlobalVarInst "y": string
// CHECK-NEXT:                 DeclareGlobalVarInst "i": string
// CHECK-NEXT:  {r0}      %4 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  {r2}      %5 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %4: object, {r2} %5: object, "a": string
// CHECK-NEXT:  {r0}      %7 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %7: object, {r2} %5: object, "x": string
// CHECK-NEXT:  {r0}      %9 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %9: object, {r2} %5: object, "y": string
// CHECK-NEXT:  {n0}     %11 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:                 StorePropertyLooseInst {n0} %11: number, {r2} %5: object, "i": string
// CHECK-NEXT:  {r0}     %13 = LoadPropertyInst (:any) {r2} %5: object, "i": string
// CHECK-NEXT:  {n1}     %14 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {np0}    %15 = BinaryLessThanInst (:boolean) {r0} %13: any, {n1} %14: number
// CHECK-NEXT:  {r0}     %16 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {n0}     %17 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  {r0}     %18 = MovInst (:undefined) {r0} %16: undefined
// CHECK-NEXT:                 CondBranchInst {np0} %15: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r3}     %20 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:                 StorePropertyLooseInst {r3} %20: object, {r2} %5: object, "y": string
// CHECK-NEXT:  {r5}     %22 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  {r4}     %23 = AllocStackInst (:any) $?anon_2_base: any
// CHECK-NEXT:  {n3}     %24 = AllocStackInst (:number) $?anon_3_idx: any
// CHECK-NEXT:  {n2}     %25 = AllocStackInst (:number) $?anon_4_size: any
// CHECK-NEXT:  {r1}     %26 = LoadPropertyInst (:any) {r2} %5: object, "x": string
// CHECK-NEXT:                 StoreStackInst {r1} %26: any, {r4} %23: any
// CHECK-NEXT:  {r1}     %28 = AllocStackInst (:any) $?anon_5_prop: any
// CHECK-NEXT:                 GetPNamesInst {r5} %22: any, {r4} %23: any, {n3} %24: number, {n2} %25: number, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r0}     %30 = PhiInst (:undefined|object) {r0} %18: undefined, %BB0, {r0} %37: object, %BB3
// CHECK-NEXT:  {r0}     %31 = MovInst (:undefined|object) {r0} %30: undefined|object
// CHECK-NEXT:                 ReturnInst {r0} %31: undefined|object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {r1}     %33 = LoadPropertyInst (:any) {r2} %5: object, "i": string
// CHECK-NEXT:  {r1}     %34 = UnaryIncInst (:number|bigint) {r1} %33: any
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %34: number|bigint, {r2} %5: object, "i": string
// CHECK-NEXT:  {r1}     %36 = LoadPropertyInst (:any) {r2} %5: object, "i": string
// CHECK-NEXT:  {r0}     %37 = MovInst (:object) {r3} %20: object
// CHECK-NEXT:                 CmpBrLessThanInst {r1} %36: any, {n1} %14: number, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:                 GetNextPNameInst {r1} %28: any, {r4} %23: any, {n3} %24: number, {n2} %25: number, {r5} %22: any, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  {r7}     %40 = LoadStackInst (:any) {r1} %28: any
// CHECK-NEXT:  {r6}     %41 = LoadPropertyInst (:any) {r2} %5: object, "a": string
// CHECK-NEXT:                 StorePropertyLooseInst {r7} %40: any, {r6} %41: any, {n0} %17: number
// CHECK-NEXT:                 BranchInst %BB4
// CHECK-NEXT:function_end
