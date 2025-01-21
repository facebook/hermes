/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

function main(x, y, z) {

  var sum = 3;
  for (var i = 0; i < 10; i++) {
    sum += (x + i) * (y + i);
  }

  return sum;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "main": string
// CHECK-NEXT:  {r0}      %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r1}      %2 = CreateFunctionInst (:object) {r0} %1: environment, %main(): functionCode
// CHECK-NEXT:  {r0}      %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %2: object, {r0} %3: object, "main": string
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %5: undefined
// CHECK-NEXT:function_end

// CHECK:function main(x: any, y: any, z: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  {r0}      %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  {n4}      %2 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {n3}      %3 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {n2}      %4 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {n1}      %5 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  {n4}      %6 = MovInst (:number) {n4} %2: number
// CHECK-NEXT:  {n3}      %7 = MovInst (:number) {n3} %3: number
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {n4}      %9 = PhiInst (:number) {n4} %6: number, %BB0, {n4} %17: number, %BB1
// CHECK-NEXT:  {n3}     %10 = PhiInst (:number) {n3} %7: number, %BB0, {n3} %16: number, %BB1
// CHECK-NEXT:  {r3}     %11 = BinaryAddInst (:string|number) {r1} %0: any, {n4} %9: number
// CHECK-NEXT:  {r2}     %12 = BinaryAddInst (:string|number) {r0} %1: any, {n4} %9: number
// CHECK-NEXT:  {n0}     %13 = BinaryMultiplyInst (:number) {r3} %11: string|number, {r2} %12: string|number
// CHECK-NEXT:  {n0}     %14 = FAddInst (:number) {n3} %10: number, {n0} %13: number
// CHECK-NEXT:  {n5}     %15 = FAddInst (:number) {n4} %9: number, {n2} %4: number
// CHECK-NEXT:  {n3}     %16 = MovInst (:number) {n0} %14: number
// CHECK-NEXT:  {n4}     %17 = MovInst (:number) {n5} %15: number
// CHECK-NEXT:                 HBCFCmpBrLessThanInst {n4} %17: number, {n1} %5: number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:                 ReturnInst {n0} %14: number
// CHECK-NEXT:function_end
