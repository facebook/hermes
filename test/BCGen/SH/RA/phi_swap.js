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
// CHECK-NEXT:  $loc0      = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc1      = HBCCreateFunctionInst (:object) %foo(): any, $loc0: environment
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $loc0      = StorePropertyLooseInst $loc1: object, $loc0: object, "foo": string
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = ReturnInst $np0: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc1      = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $loc0      = LoadParamInst (:any) %b: any
// CHECK-NEXT:  $loc1      = MovInst (:any) $loc1: any
// CHECK-NEXT:  $loc0      = MovInst (:any) $loc0: any
// CHECK-NEXT:  $loc2      = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $loc1      = PhiInst (:any) $loc1: any, %BB0, $loc1: any, %BB1
// CHECK-NEXT:  $loc0      = PhiInst (:any) $loc0: any, %BB0, $loc0: any, %BB1
// CHECK-NEXT:  $loc2      = MovInst (:any) $loc1: any
// CHECK-NEXT:  $loc0      = MovInst (:any) $loc0: any
// CHECK-NEXT:  $loc1      = MovInst (:any) $loc0: any
// CHECK-NEXT:  $loc0      = MovInst (:any) $loc2: any
// CHECK-NEXT:  $loc0      = BranchInst %BB1
// CHECK-NEXT:function_end
