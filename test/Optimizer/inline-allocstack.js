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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst (:object) %1: object, %""(): functionCode, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [arr: object, bar: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [bar]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:  %5 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [arr]: object
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [i: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %1 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %bar(): any, %2: environment
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreStackInst 0: number, %1: number
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [i]: number
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadStackInst (:number) %1: number
// CHECK-NEXT:        StoreStackInst %12: number, %0: number
// CHECK-NEXT:  %14 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %17 = FLessThanInst (:boolean) %16: number, 100: number
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB6, %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = LoadFrameInst (:object) %2: environment, [arr@""]: object
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: object, "push": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %3: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, empty: any, undefined: undefined, %19: object, %21: object
// CHECK-NEXT:  %23 = LoadStackInst (:number) %0: number
// CHECK-NEXT:  %24 = FAddInst (:number) %23: number, 1: number
// CHECK-NEXT:        StoreStackInst %24: number, %0: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = LoadStackInst (:number) %0: number
// CHECK-NEXT:        StoreStackInst %27: number, %1: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %27: number, [i]: number
// CHECK-NEXT:        StoreStackInst false: boolean, %9: boolean
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
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [bar@""]: object
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %bar(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %bar(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [i@bar]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
