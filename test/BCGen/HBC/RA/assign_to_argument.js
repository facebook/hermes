/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function foo(a) {
  a = a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r0}      %1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {r1}      %2 = CreateFunctionInst (:object) {r1} %0: environment, %foo(): functionCode
// CHECK-NEXT:  {r2}      %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}      %4 = StorePropertyLooseInst {r1} %2: object, {r2} %3: object, "foo": string
// CHECK-NEXT:  {r1}      %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  {r2}      %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}      %7 = StoreStackInst {r2} %6: undefined, {r1} %5: any
// CHECK-NEXT:  {r1}      %8 = LoadStackInst (:any) {r1} %5: any
// CHECK-NEXT:  {r0}      %9 = ReturnInst {r1} %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [a: any]

// CHECK:function foo(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  {r1}      %1 = CreateScopeInst (:environment) %VS1: any, {r1} %0: environment
// CHECK-NEXT:  {r2}      %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {r0}      %3 = StoreFrameInst {r1} %1: environment, {r2} %2: any, [%VS1.a]: any
// CHECK-NEXT:  {r2}      %4 = LoadFrameInst (:any) {r1} %1: environment, [%VS1.a]: any
// CHECK-NEXT:  {r0}      %5 = StoreFrameInst {r1} %1: environment, {r2} %4: any, [%VS1.a]: any
// CHECK-NEXT:  {r1}      %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}      %7 = ReturnInst {r1} %6: undefined
// CHECK-NEXT:function_end
