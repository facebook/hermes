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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = HBCCreateFunctionEnvironmentInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  $Reg2 = HBCCreateFunctionInst (:object) %foo(): functionCode, $Reg0
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "foo": string
// CHECK-NEXT:  $Reg0 = HBCCreateFunctionInst (:object) %bar(): functionCode, $Reg0
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "bar": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function foo(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function bar(a: any, b: any, c: any, d: any, e: any, f: any, g: any, h: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg9 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  $Reg8 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg9
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  $Reg7 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg8
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %d: any
// CHECK-NEXT:  $Reg6 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg7
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  $Reg5 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg6
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  $Reg4 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg5
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %g: any
// CHECK-NEXT:  $Reg3 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg4
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %h: any
// CHECK-NEXT:  $Reg2 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg9
// CHECK-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg0, "foo": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = CallInst (:any) $Reg1, empty: any, empty: any, undefined: undefined, $Reg0, $Reg2, $Reg3, $Reg4, $Reg5, $Reg6, $Reg7, $Reg8, $Reg9
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end
