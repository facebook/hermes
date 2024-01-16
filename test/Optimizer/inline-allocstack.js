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
  var arr = [];
  function bar() {
        try {
            for (let i = 0; i < 100; arr.push(()=>i), ++i);
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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:  %1 = CallInst (:object) %0: object, %""(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [arr: object, bar: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: object, [bar]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:  %3 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:       StoreFrameInst %3: object, [arr]: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [i: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %1 = AllocStackInst (:number) $i: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreStackInst 0: number, %1: number
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: number
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %7: boolean
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = LoadStackInst (:number) %1: number
// CHECK-NEXT:        StoreStackInst %10: number, %0: number
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %15 = FLessThanInst (:boolean) %14: number, 100: number
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB6, %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = LoadFrameInst (:object) [arr@""]: object
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %17: object, "push": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %" 1#"(): functionCode
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, empty: any, undefined: undefined, %17: object, %19: object
// CHECK-NEXT:  %21 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %22 = FAddInst (:number) %21: number, 1: number
// CHECK-NEXT:        StoreStackInst %22: number, %0: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %25 = LoadStackInst (:number) %0: number
// CHECK-NEXT:        StoreStackInst %25: number, %1: number
// CHECK-NEXT:        StoreFrameInst %25: number, [i]: number
// CHECK-NEXT:        StoreStackInst false: boolean, %7: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [bar@""]: object
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %bar(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number) [i@bar]: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end
