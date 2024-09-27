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
// CHECK-NEXT:  $Reg2 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "poszero": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "negzero": string
// CHECK-NEXT:  $Reg0 = CreateFunctionInst (:object) $Reg2, %poszero(): functionCode
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "poszero": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg2, %negzero(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "negzero": string
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg2 = ReturnInst $Reg2
// CHECK-NEXT:function_end

// CHECK:function poszero(f: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg0
// CHECK-NEXT:  $Reg4 = ImplicitMovInst (:number) $Reg2
// CHECK-NEXT:  $Reg3 = ImplicitMovInst (:number) $Reg2
// CHECK-NEXT:  $Reg2 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg0, $Reg2, $Reg2
// CHECK-NEXT:  $Reg2 = ReturnInst $Reg2
// CHECK-NEXT:function_end

// CHECK:function negzero(f: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) -0: number
// CHECK-NEXT:  $Reg5 = ImplicitMovInst (:undefined) $Reg0
// CHECK-NEXT:  $Reg4 = ImplicitMovInst (:number) $Reg2
// CHECK-NEXT:  $Reg3 = ImplicitMovInst (:number) $Reg2
// CHECK-NEXT:  $Reg2 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg0, $Reg2, $Reg2
// CHECK-NEXT:  $Reg2 = ReturnInst $Reg2
// CHECK-NEXT:function_end
