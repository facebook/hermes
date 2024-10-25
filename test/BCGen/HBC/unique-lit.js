/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-lra %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-lra %s | %FileCheckOrRegen --match-full-lines  --check-prefix=CHKOPT %s

// Check that literals are uniqued when optimizations is disabled, but aren't
// when it is enabled.

var a, b;

function foo(x) {
  if (x)
    a = 10;
  else
    b = 10;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r0}      %1 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  {r0}      %2 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  {r0}      %3 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {r1}      %4 = CreateFunctionInst (:object) {r1} %0: environment, %foo(): functionCode
// CHECK-NEXT:  {r2}      %5 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}      %6 = StorePropertyLooseInst {r1} %4: object, {r2} %5: object, "foo": string
// CHECK-NEXT:  {r1}      %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  {r2}      %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r1}      %9 = MovInst (:undefined) {r2} %8: undefined
// CHECK-NEXT:  {r1}     %10 = LoadStackInst (:any) {r1} %7: any
// CHECK-NEXT:  {r0}     %11 = ReturnInst {r1} %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  {r1}      %1 = CreateScopeInst (:environment) %VS1: any, {r1} %0: environment
// CHECK-NEXT:  {r2}      %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  {r0}      %3 = StoreFrameInst {r1} %1: environment, {r2} %2: any, [%VS1.x]: any
// CHECK-NEXT:  {r1}      %4 = LoadFrameInst (:any) {r1} %1: environment, [%VS1.x]: any
// CHECK-NEXT:  {r0}      %5 = CondBranchInst {r1} %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r1}      %6 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  {r2}      %7 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}      %8 = StorePropertyLooseInst {r1} %6: number, {r2} %7: object, "a": string
// CHECK-NEXT:  {r0}      %9 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r1}     %10 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  {r2}     %11 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}     %12 = StorePropertyLooseInst {r1} %10: number, {r2} %11: object, "b": string
// CHECK-NEXT:  {r0}     %13 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {r1}     %14 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %15 = ReturnInst {r1} %14: undefined
// CHECK-NEXT:function_end

// CHKOPT:scope %VS0 []

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:                 DeclareGlobalVarInst "a": string
// CHKOPT-NEXT:                 DeclareGlobalVarInst "b": string
// CHKOPT-NEXT:                 DeclareGlobalVarInst "foo": string
// CHKOPT-NEXT:  {r0}      %3 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKOPT-NEXT:  {r1}      %4 = CreateFunctionInst (:object) {r0} %3: environment, %foo(): functionCode
// CHKOPT-NEXT:  {r0}      %5 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:                 StorePropertyLooseInst {r1} %4: object, {r0} %5: object, "foo": string
// CHKOPT-NEXT:  {np0}     %7 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:                 ReturnInst {np0} %7: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(x: any): undefined
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  {n0}      %0 = HBCLoadConstInst (:number) 10: number
// CHKOPT-NEXT:  {r1}      %1 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  {r0}      %2 = LoadParamInst (:any) %x: any
// CHKOPT-NEXT:                 CondBranchInst {r0} %2: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:                 StorePropertyLooseInst {n0} %0: number, {r1} %1: object, "a": string
// CHKOPT-NEXT:                 BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:                 StorePropertyLooseInst {n0} %0: number, {r1} %1: object, "b": string
// CHKOPT-NEXT:                 BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  {np0}     %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:                 ReturnInst {np0} %8: undefined
// CHKOPT-NEXT:function_end
