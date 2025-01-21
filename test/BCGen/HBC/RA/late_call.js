/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

function foo() {
  return 0;
}

function bar(a,b,c,d,e,f,g,h) {
  b += a;
  c += b;
  d += c;
  e += d;
  f += e;
  g += f;
  h += a;
  foo(h, g, f, e, d, c, b, a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:                 DeclareGlobalVarInst "foo": string
// CHECK-NEXT:                 DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  {r2}      %3 = CreateFunctionInst (:object) {r0} %0: environment, %foo(): functionCode
// CHECK-NEXT:  {r1}      %4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %3: object, {r1} %4: object, "foo": string
// CHECK-NEXT:  {r0}      %6 = CreateFunctionInst (:object) {r0} %0: environment, %bar(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %6: object, {r1} %4: object, "bar": string
// CHECK-NEXT:  {np0}     %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %8: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {n0}      %0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:                 ReturnInst {n0} %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(a: any, b: any, c: any, d: any, e: any, f: any, g: any, h: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r8}      %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {r0}      %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  {r7}      %2 = BinaryAddInst (:string|number|bigint) {r0} %1: any, {r8} %0: any
// CHECK-NEXT:  {r0}      %3 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  {r6}      %4 = BinaryAddInst (:string|number|bigint) {r0} %3: any, {r7} %2: string|number|bigint
// CHECK-NEXT:  {r0}      %5 = LoadParamInst (:any) %d: any
// CHECK-NEXT:  {r5}      %6 = BinaryAddInst (:string|number|bigint) {r0} %5: any, {r6} %4: string|number|bigint
// CHECK-NEXT:  {r0}      %7 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  {r4}      %8 = BinaryAddInst (:string|number|bigint) {r0} %7: any, {r5} %6: string|number|bigint
// CHECK-NEXT:  {r0}      %9 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  {r3}     %10 = BinaryAddInst (:string|number|bigint) {r0} %9: any, {r4} %8: string|number|bigint
// CHECK-NEXT:  {r0}     %11 = LoadParamInst (:any) %g: any
// CHECK-NEXT:  {r2}     %12 = BinaryAddInst (:string|number|bigint) {r0} %11: any, {r3} %10: string|number|bigint
// CHECK-NEXT:  {r0}     %13 = LoadParamInst (:any) %h: any
// CHECK-NEXT:  {r1}     %14 = BinaryAddInst (:string|number|bigint) {r0} %13: any, {r8} %0: any
// CHECK-NEXT:  {r0}     %15 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r0}     %16 = LoadPropertyInst (:any) {r0} %15: object, "foo": string
// CHECK-NEXT:  {np0}    %17 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %18 = CallInst (:any) {r0} %16: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %17: undefined, {r1} %14: string|number|bigint, {r2} %12: string|number|bigint, {r3} %10: string|number|bigint, {r4} %8: string|number|bigint, {r5} %6: string|number|bigint, {r6} %4: string|number|bigint, {r7} %2: string|number|bigint, {r8} %0: any
// CHECK-NEXT:                 ReturnInst {np0} %17: undefined
// CHECK-NEXT:function_end
