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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:               DeclareGlobalVarInst "test_call": string
// CHECK-NEXT:               DeclareGlobalVarInst "test_new": string
// CHECK-NEXT:               DeclareGlobalVarInst "test_builtin": string
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %test_call(): any, $loc0: environment
// CHECK-NEXT:  $loc1      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:               StorePropertyLooseInst $loc2: object, $loc1: object, "test_call": string
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %test_new(): object, $loc0: environment
// CHECK-NEXT:               StorePropertyLooseInst $loc2: object, $loc1: object, "test_new": string
// CHECK-NEXT:  $loc0      = HBCCreateFunctionInst (:object) %test_builtin(): number, $loc0: environment
// CHECK-NEXT:               StorePropertyLooseInst $loc0: object, $loc1: object, "test_builtin": string
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:               ReturnInst $np0: undefined
// CHECK-NEXT:function_end

// CHECK:function test_call(bar: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $stack[5]  = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  $np4       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  $stack[1]  = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  $stack[6]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[4]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallInst (:any) $stack[5]: any, empty: any, empty: any, $np4: undefined, $stack[4]: undefined, $stack[3]: number, $stack[2]: number, $stack[1]: number, $stack[0]: number
// CHECK-NEXT:               ReturnInst $loc0: any
// CHECK-NEXT:function_end

// CHECK:function test_new(bar: any): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc0      = LoadParamInst (:any) %bar: any
// CHECK-NEXT:  $loc1      = LoadPropertyInst (:any) $loc0: any, "prototype": string
// CHECK-NEXT:  $loc1      = CreateThisInst (:object) $loc1: any, $loc0: any
// CHECK-NEXT:  $stack[3]  = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:number) 11: number
// CHECK-NEXT:  $stack[1]  = HBCLoadConstInst (:number) 12: number
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 13: number
// CHECK-NEXT:  $stack[6]  = MovInst (:any) $loc0: any
// CHECK-NEXT:  $stack[5]  = MovInst (:any) $loc0: any
// CHECK-NEXT:  $stack[4]  = MovInst (:object) $loc1: object
// CHECK-NEXT:  $loc0      = CallInst (:any) $stack[5]: any, empty: any, empty: any, $loc0: any, $stack[4]: object, $stack[3]: number, $stack[2]: number, $stack[1]: number, $stack[0]: number
// CHECK-NEXT:  $loc0      = GetConstructedObjectInst (:object) $loc1: object, $loc0: any
// CHECK-NEXT:               ReturnInst $loc0: object
// CHECK-NEXT:function_end

// CHECK:function test_builtin(a: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $stack[1]  = LoadParamInst (:any) %a: any
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $stack[4]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  $stack[2]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallBuiltinInst (:any) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, $stack[1]: any, $stack[0]: number
// CHECK-NEXT:               ReturnInst $loc0: any
// CHECK-NEXT:function_end
