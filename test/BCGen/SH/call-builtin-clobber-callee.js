/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-lra %s | %FileCheckOrRegen --match-full-lines %s

/// Ensure that the callee stack entry is clobbered by the builtin call.
function test_call_after_builtin() {
    print({valueOf: () => 2} ** 3);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:               DeclareGlobalVarInst "test_call_after_builtin": string
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc1      = HBCCreateFunctionInst (:object) %test_call_after_builtin(): undefined, $loc0: environment
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:               StorePropertyLooseInst $loc1: object, $loc0: object, "test_call_after_builtin": string
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:               ReturnInst $np0: undefined
// CHECK-NEXT:function_end

// CHECK:function test_call_after_builtin(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $loc1      = TryLoadGlobalPropertyInst (:any) $loc0: object, "print": string
// CHECK-NEXT:  $loc0      = HBCAllocObjectFromBufferInst (:object) 1: number, "valueOf": string, null: null
// CHECK-NEXT:  $loc2      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %valueOf(): number, $loc2: environment
// CHECK-NEXT:               StorePropertyLooseInst $loc2: object, $loc0: object, "valueOf": string
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $stack[4]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  $stack[2]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[1]  = MovInst (:object) $loc0: object
// CHECK-NEXT:  $stack[1]  = CallBuiltinInst (:any) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, $stack[1]: object, $stack[0]: number
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[4]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = MovInst (:any) $loc1: any
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallInst (:any) $stack[3]: any, empty: any, empty: any, $np0: undefined, $stack[2]: undefined, $stack[1]: any
// CHECK-NEXT:               ReturnInst $np0: undefined
// CHECK-NEXT:function_end

// CHECK:arrow valueOf(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:               ReturnInst $np0: number
// CHECK-NEXT:function_end
