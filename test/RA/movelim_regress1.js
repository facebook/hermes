/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
// Mov elimination incorrectly eliminated a mov in this case.

function fib(n) {
  var f0 = 0, f1 = 1;
  for (; n > 0; n = n -1) {
	var f2 = f0 + f1;
	f0 = f1; f1 = f2;
  }
  return f0;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "fib": string
// CHECK-NEXT:  {r0}      %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r1}      %2 = CreateFunctionInst (:object) {r0} %1: environment, %fib(): functionCode
// CHECK-NEXT:  {r0}      %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %2: object, {r0} %3: object, "fib": string
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %5: undefined
// CHECK-NEXT:function_end

// CHECK:function fib(n: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %n: any
// CHECK-NEXT:  {n4}      %1 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {np0}     %2 = BinaryGreaterThanInst (:boolean) {r0} %0: any, {n4} %1: number
// CHECK-NEXT:  {n3}      %3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {n2}      %4 = MovInst (:number) {n3} %3: number
// CHECK-NEXT:  {n1}      %5 = MovInst (:number) {n4} %1: number
// CHECK-NEXT:  {r0}      %6 = MovInst (:any) {r0} %0: any
// CHECK-NEXT:  {n0}      %7 = MovInst (:number) {n1} %5: number
// CHECK-NEXT:                 CondBranchInst {np0} %2: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {n2}      %9 = PhiInst (:number) {n2} %4: number, %BB0, {n2} %15: number, %BB1
// CHECK-NEXT:  {n1}     %10 = PhiInst (:number) {n1} %5: number, %BB0, {n1} %16: number, %BB1
// CHECK-NEXT:  {r0}     %11 = PhiInst (:any) {r0} %6: any, %BB0, {r0} %18: number, %BB1
// CHECK-NEXT:  {n7}     %12 = FAddInst (:number) {n1} %10: number, {n2} %9: number
// CHECK-NEXT:  {n5}     %13 = BinarySubtractInst (:number) {r0} %11: any, {n3} %3: number
// CHECK-NEXT:  {n6}     %14 = MovInst (:number) {n2} %9: number
// CHECK-NEXT:  {n2}     %15 = MovInst (:number) {n7} %12: number
// CHECK-NEXT:  {n1}     %16 = MovInst (:number) {n6} %14: number
// CHECK-NEXT:  {n0}     %17 = MovInst (:number) {n1} %16: number
// CHECK-NEXT:  {r0}     %18 = MovInst (:number) {n5} %13: number
// CHECK-NEXT:                 HBCFCmpBrGreaterThanInst {r0} %18: number, {n4} %1: number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {n0}     %20 = PhiInst (:number) {n0} %7: number, %BB0, {n0} %17: number, %BB1
// CHECK-NEXT:  {n0}     %21 = MovInst (:number) {n0} %20: number
// CHECK-NEXT:                 ReturnInst {n0} %21: number
// CHECK-NEXT:function_end
