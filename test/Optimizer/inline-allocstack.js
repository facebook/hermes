/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test that the type of AllocStackInst is correctly copied during inlining
// so it matches the type of LoadStackInst. The problematic stack allocation
// was specifically for the boolean flag used by the scoped for loop.

"use strict";

(function () {
  function bar() {
        try {
            for (let i = 0; i < 100; ++i);
        } catch (e) {
            throw e;
        }
    }

    function foo() {
        bar();
    }

    return foo;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): object
// CHECK-NEXT:  %1 = CallInst (:object) %0: object, %""(): object, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): undefined
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %1 = AllocStackInst (:number) $i: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreStackInst 0: number, %1: number
// CHECK-NEXT:  %4 = AllocStackInst (:boolean) $?anon_0_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %4: boolean
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadStackInst (:number) %1: number
// CHECK-NEXT:       StoreStackInst %7: number, %0: number
// CHECK-NEXT:  %9 = LoadStackInst (:boolean) %4: boolean
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %12 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:        StoreStackInst %12: number, %0: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %16 = FLessThanInst (:boolean) %15: number, 100: number
// CHECK-NEXT:        CondBranchInst %16: boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = LoadStackInst (:number) %0: number
// CHECK-NEXT:        StoreStackInst %21: number, %1: number
// CHECK-NEXT:        StoreStackInst false: boolean, %4: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = CatchInst (:any)
// CHECK-NEXT:        ThrowInst %25: any
// CHECK-NEXT:function_end
