/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function sink(x,y,z) {}

function foo(x) {
  x.sink(1,2,3)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r0}      %1 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  {r0}      %2 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {r2}      %3 = CreateFunctionInst (:object) {r1} %0: environment, %sink(): functionCode
// CHECK-NEXT:  {r3}      %4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}      %5 = StorePropertyLooseInst {r2} %3: object, {r3} %4: object, "sink": string
// CHECK-NEXT:  {r1}      %6 = CreateFunctionInst (:object) {r1} %0: environment, %foo(): functionCode
// CHECK-NEXT:  {r2}      %7 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}      %8 = StorePropertyLooseInst {r1} %6: object, {r2} %7: object, "foo": string
// CHECK-NEXT:  {r1}      %9 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  {r2}     %10 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %11 = StoreStackInst {r2} %10: undefined, {r1} %9: any
// CHECK-NEXT:  {r1}     %12 = LoadStackInst (:any) {r1} %9: any
// CHECK-NEXT:  {r0}     %13 = ReturnInst {r1} %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: any, y: any, z: any]

// CHECK:function sink(x: any, y: any, z: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  {r1}      %1 = CreateScopeInst (:environment) %VS1: any, {r1} %0: environment
// CHECK-NEXT:  {r2}      %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  {r0}      %3 = StoreFrameInst {r1} %1: environment, {r2} %2: any, [%VS1.x]: any
// CHECK-NEXT:  {r2}      %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  {r0}      %5 = StoreFrameInst {r1} %1: environment, {r2} %4: any, [%VS1.y]: any
// CHECK-NEXT:  {r2}      %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  {r0}      %7 = StoreFrameInst {r1} %1: environment, {r2} %6: any, [%VS1.z]: any
// CHECK-NEXT:  {r1}      %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}      %9 = ReturnInst {r1} %8: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r2}      %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  {r2}      %1 = CreateScopeInst (:environment) %VS1: any, {r2} %0: environment
// CHECK-NEXT:  {r1}      %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  {r0}      %3 = StoreFrameInst {r2} %1: environment, {r1} %2: any, [%VS1.x]: any
// CHECK-NEXT:  {r2}      %4 = LoadFrameInst (:any) {r2} %1: environment, [%VS1.x]: any
// CHECK-NEXT:  {r1}      %5 = LoadPropertyInst (:any) {r2} %4: any, "sink": string
// CHECK-NEXT:  {r3}      %6 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {r4}      %7 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  {r5}      %8 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  {r0}      %9 = HBCCallNInst (:any) {r1} %5: any, empty: any, false: boolean, empty: any, undefined: undefined, {r2} %4: any, {r3} %6: number, {r4} %7: number, {r5} %8: number
// CHECK-NEXT:  {r1}     %10 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %11 = ReturnInst {r1} %10: undefined
// CHECK-NEXT:function_end
