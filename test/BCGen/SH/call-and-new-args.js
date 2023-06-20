/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-lra -dump-operand-registers %s | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc1      = DeclareGlobalVarInst "test_call": string
// CHECK-NEXT:  $loc1      = DeclareGlobalVarInst "test_new": string
// CHECK-NEXT:  $loc1      = DeclareGlobalVarInst "test_builtin": string
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %test_call(): any, $loc0
// CHECK-NEXT:  $loc1      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $loc2      = StorePropertyLooseInst $loc2, $loc1, "test_call": string
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %test_new(): object, $loc0
// CHECK-NEXT:  $loc2      = StorePropertyLooseInst $loc2, $loc1, "test_new": string
// CHECK-NEXT:  $loc0      = HBCCreateFunctionInst (:object) %test_builtin(): number, $loc0
// CHECK-NEXT:  $loc0      = StorePropertyLooseInst $loc0, $loc1, "test_builtin": string
// CHECK-NEXT:  $loc0      = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = ReturnInst $loc0
// CHECK-NEXT:function_end

// CHECK:function test_call(bar: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $stack[5]  = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  $loc4      = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  $stack[1]  = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  $stack[6]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[4]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallInst (:any) $stack[5], empty: any, empty: any, $loc4, $stack[4], $stack[3], $stack[2], $stack[1], $stack[0]
// CHECK-NEXT:  $loc0      = ReturnInst $loc0
// CHECK-NEXT:function_end

// CHECK:function test_new(bar: any): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc5      = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  $loc0      = LoadPropertyInst (:any) $loc5, "prototype": string
// CHECK-NEXT:  $loc1      = CreateThisInst (:object) $loc0, $loc5
// CHECK-NEXT:  $stack[3]  = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  $stack[1]  = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  $stack[6]  = MovInst (:any) $loc5
// CHECK-NEXT:  $stack[5]  = MovInst (:any) $loc5
// CHECK-NEXT:  $stack[4]  = MovInst (:object) $loc1
// CHECK-NEXT:  $loc0      = ConstructInst (:any) $stack[5], empty: any, empty: any, $loc5, $stack[4], $stack[3], $stack[2], $stack[1], $stack[0]
// CHECK-NEXT:  $loc0      = GetConstructedObjectInst (:object) $loc1, $loc0
// CHECK-NEXT:  $loc0      = ReturnInst $loc0
// CHECK-NEXT:function_end

// CHECK:function test_builtin(a: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc2      = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[1]  = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $stack[4]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  $stack[2]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallBuiltinInst (:any) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, $loc2, undefined: undefined, $stack[1], $stack[0]
// CHECK-NEXT:  $loc0      = ReturnInst $loc0
// CHECK-NEXT:function_end
