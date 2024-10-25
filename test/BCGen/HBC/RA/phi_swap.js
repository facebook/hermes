/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
// Ensure that the register allocator correctly handles cycles between Phi-nodes.

function foo (a, b) {
    var t;
    for(;;) {
        t = a;
        a = b;
        b = t;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {r0}      %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r1}      %2 = CreateFunctionInst (:object) {r0} %1: environment, %foo(): functionCode
// CHECK-NEXT:  {r0}      %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r1} %2: object, {r0} %3: object, "foo": string
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %5: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {r0}      %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  {r0}      %2 = MovInst (:any) {r0} %1: any
// CHECK-NEXT:  {r1}      %3 = MovInst (:any) {r1} %0: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r0}      %5 = PhiInst (:any) {r0} %2: any, %BB0, {r0} %9: any, %BB1
// CHECK-NEXT:  {r1}      %6 = PhiInst (:any) {r1} %3: any, %BB0, {r1} %10: any, %BB1
// CHECK-NEXT:  {r2}      %7 = MovInst (:any) {r0} %5: any
// CHECK-NEXT:  {r3}      %8 = MovInst (:any) {r1} %6: any
// CHECK-NEXT:  {r0}      %9 = MovInst (:any) {r3} %8: any
// CHECK-NEXT:  {r1}     %10 = MovInst (:any) {r2} %7: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:function_end
