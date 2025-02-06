/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-lra %s | %FileCheckOrRegen --match-full-lines %s

// Check that call and new arguments, including the calleee and new.target, are
// correctly lowered into stack registers.

function test_call(bar) {
    return bar(10,11,12,13);
}
function test_new(bar) {
    return new bar(10,11,12,13);
}
function test_builtin(a) {
    return a ** 3;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc0}    %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:                 DeclareGlobalVarInst "test_call": string
// CHECK-NEXT:                 DeclareGlobalVarInst "test_new": string
// CHECK-NEXT:                 DeclareGlobalVarInst "test_builtin": string
// CHECK-NEXT:  {loc1}    %4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc2}    %5 = CreateFunctionInst (:object) {loc0} %0: environment, %VS0: any, %test_call(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {loc2} %5: object, {loc1} %4: object, "test_call": string
// CHECK-NEXT:  {loc2}    %7 = CreateFunctionInst (:object) {loc0} %0: environment, %VS0: any, %test_new(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {loc2} %7: object, {loc1} %4: object, "test_new": string
// CHECK-NEXT:  {loc0}    %9 = CreateFunctionInst (:object) {loc0} %0: environment, %VS0: any, %test_builtin(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {loc0} %9: object, {loc1} %4: object, "test_builtin": string
// CHECK-NEXT:  {np0}    %11 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %11: undefined
// CHECK-NEXT:function_end

// CHECK:function test_call(bar: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {stack[0]}  %0 = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  {stack[1]}  %1 = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  {stack[2]}  %2 = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  {stack[3]}  %3 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  {np0}     %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[5]}  %5 = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  {stack[6]}  %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[4]}  %7 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {loc0}    %8 = CallInst (:any) {stack[5]} %5: any, empty: any, false: boolean, empty: any, {np0} %4: undefined, {stack[4]} %7: undefined, {stack[3]} %3: number, {stack[2]} %2: number, {stack[1]} %1: number, {stack[0]} %0: number
// CHECK-NEXT:                 ReturnInst {loc0} %8: any
// CHECK-NEXT:function_end

// CHECK:function test_new(bar: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc0}    %0 = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  {loc1}    %1 = CreateThisInst (:undefined|object) {loc0} %0: any, empty: any
// CHECK-NEXT:  {stack[0]}  %2 = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  {stack[1]}  %3 = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  {stack[2]}  %4 = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  {stack[3]}  %5 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  {stack[6]}  %6 = MovInst (:any) {loc0} %0: any
// CHECK-NEXT:  {stack[5]}  %7 = MovInst (:any) {loc0} %0: any
// CHECK-NEXT:  {stack[4]}  %8 = MovInst (:undefined|object) {loc1} %1: undefined|object
// CHECK-NEXT:  {loc0}    %9 = CallInst (:any) {stack[5]} %7: any, empty: any, false: boolean, empty: any, {loc0} %0: any, {stack[4]} %8: undefined|object, {stack[3]} %5: number, {stack[2]} %4: number, {stack[1]} %3: number, {stack[0]} %2: number
// CHECK-NEXT:  {loc0}   %10 = GetConstructedObjectInst (:object) {loc1} %1: undefined|object, {loc0} %9: any
// CHECK-NEXT:                 ReturnInst {loc0} %10: object
// CHECK-NEXT:function_end

// CHECK:function test_builtin(a: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {stack[0]}  %0 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {stack[1]}  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {stack[4]}  %2 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {stack[3]}  %3 = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  {stack[2]}  %4 = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  {np0}     %5 = CallBuiltinInst (:number) [HermesBuiltin.exponentiationOperator]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, {stack[1]} %1: any, {stack[0]} %0: number
// CHECK-NEXT:                 ReturnInst {np0} %5: number
// CHECK-NEXT:function_end
