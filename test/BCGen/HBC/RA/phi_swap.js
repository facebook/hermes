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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg0 = HBCCreateFunctionEnvironmentInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = HBCCreateFunctionInst (:object) %foo(): functionCode, $Reg0
// CHECK-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "foo": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHECK-NEXT:  $Reg0 = MovInst (:any) $Reg0
// CHECK-NEXT:  $Reg2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB0, $Reg1, %BB1
// CHECK-NEXT:  $Reg0 = PhiInst (:any) $Reg0, %BB0, $Reg0, %BB1
// CHECK-NEXT:  $Reg2 = MovInst (:any) $Reg1
// CHECK-NEXT:  $Reg0 = MovInst (:any) $Reg0
// CHECK-NEXT:  $Reg1 = MovInst (:any) $Reg0
// CHECK-NEXT:  $Reg0 = MovInst (:any) $Reg2
// CHECK-NEXT:  $Reg0 = BranchInst %BB1
// CHECK-NEXT:function_end
