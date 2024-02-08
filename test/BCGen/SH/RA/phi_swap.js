/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {loc0}    %1 = HBCCreateFunctionEnvironmentInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  {loc1}    %2 = HBCCreateFunctionInst (:object) %foo(): functionCode, {loc0} %1: environment
// CHECK-NEXT:  {loc0}    %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {loc1} %2: object, {loc0} %3: object, "foo": string
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %5: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc1}    %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {loc0}    %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  {loc1}    %2 = MovInst (:any) {loc1} %0: any
// CHECK-NEXT:  {loc0}    %3 = MovInst (:any) {loc0} %1: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {loc1}    %5 = PhiInst (:any) {loc1} %2: any, %BB0, {loc1} %9: any, %BB1
// CHECK-NEXT:  {loc0}    %6 = PhiInst (:any) {loc0} %3: any, %BB0, {loc0} %10: any, %BB1
// CHECK-NEXT:  {loc2}    %7 = MovInst (:any) {loc1} %5: any
// CHECK-NEXT:  {loc0}    %8 = MovInst (:any) {loc0} %6: any
// CHECK-NEXT:  {loc1}    %9 = MovInst (:any) {loc0} %8: any
// CHECK-NEXT:  {loc0}   %10 = MovInst (:any) {loc2} %7: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:function_end
