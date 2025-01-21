/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-lra -O %s | %FileCheckOrRegen %s --match-full-lines

// Positive zero is 'cheap'.
function poszero(f) {
  return f(0.0, 0.0);
}

// Negative zero is NOT 'cheap'.
function negzero(f) {
  return f(-0.0, -0.0);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:                 DeclareGlobalVarInst "poszero": string
// CHECK-NEXT:                 DeclareGlobalVarInst "negzero": string
// CHECK-NEXT:  {r1}      %3 = CreateFunctionInst (:object) {r0} %0: environment, %poszero(): functionCode
// CHECK-NEXT:  {r2}      %4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %3: object, {r2} %4: object, "poszero": string
// CHECK-NEXT:  {r0}      %6 = CreateFunctionInst (:object) {r0} %0: environment, %negzero(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %6: object, {r2} %4: object, "negzero": string
// CHECK-NEXT:  {np0}     %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %8: undefined
// CHECK-NEXT:function_end

// CHECK:function poszero(f: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {r3}      %3 = ImplicitMovInst (:undefined) {np0} %1: undefined
// CHECK-NEXT:  {r2}      %4 = ImplicitMovInst (:number) {n0} %2: number
// CHECK-NEXT:  {r1}      %5 = ImplicitMovInst (:number) {n0} %2: number
// CHECK-NEXT:  {r0}      %6 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined, {n0} %2: number, {n0} %2: number
// CHECK-NEXT:                 ReturnInst {r0} %6: any
// CHECK-NEXT:function_end

// CHECK:function negzero(f: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) -0: number
// CHECK-NEXT:  {r3}      %3 = ImplicitMovInst (:undefined) {np0} %1: undefined
// CHECK-NEXT:  {r2}      %4 = ImplicitMovInst (:number) {n0} %2: number
// CHECK-NEXT:  {r1}      %5 = ImplicitMovInst (:number) {n0} %2: number
// CHECK-NEXT:  {r0}      %6 = HBCCallNInst (:any) {r0} %0: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %1: undefined, {n0} %2: number, {n0} %2: number
// CHECK-NEXT:                 ReturnInst {r0} %6: any
// CHECK-NEXT:function_end
