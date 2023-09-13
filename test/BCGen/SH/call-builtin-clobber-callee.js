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
// CHECK-NEXT:  $loc0      = DeclareGlobalVarInst "test_call_after_builtin": string
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc1      = HBCCreateFunctionInst (:object) %test_call_after_builtin(): undefined, $loc0
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $loc0      = StorePropertyLooseInst $loc1, $loc0, "test_call_after_builtin": string
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = ReturnInst $np0
// CHECK-NEXT:function_end

// CHECK:function test_call_after_builtin(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $loc1      = TryLoadGlobalPropertyInst (:any) $loc0, "print": string
// CHECK-NEXT:  $loc0      = HBCAllocObjectFromBufferInst (:object) 1: number, "valueOf": string, null: null
// CHECK-NEXT:  $loc2      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc2      = HBCCreateFunctionInst (:object) %valueOf(): number, $loc2
// CHECK-NEXT:  $loc2      = StorePropertyLooseInst $loc2, $loc0, "valueOf": string
// CHECK-NEXT:  $stack[0]  = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $stack[4]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = ImplicitMovInst (:empty) empty: empty
// CHECK-NEXT:  $stack[2]  = ImplicitMovInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[1]  = MovInst (:object) $loc0
// CHECK-NEXT:  $stack[1]  = CallBuiltinInst (:any) [HermesBuiltin.exponentiationOperator]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, $stack[1], $stack[0]
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[4]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $stack[3]  = MovInst (:any) $loc1
// CHECK-NEXT:  $stack[2]  = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $loc0      = CallInst (:any) $stack[3], empty: any, empty: any, $np0, $stack[2], $stack[1]
// CHECK-NEXT:  $loc0      = ReturnInst $np0
// CHECK-NEXT:function_end

// CHECK:arrow valueOf(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $loc0      = ReturnInst $np0
// CHECK-NEXT:function_end
