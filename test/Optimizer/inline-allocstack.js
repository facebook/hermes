/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// XFAIL: true
// TODO: This test fails because we disable try/catch inlining.

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

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [arr: object]

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:  %3 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS1.arr]: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: number]

// CHECK:function foo(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       TryStartInst %BB7, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.i]: number
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = PhiInst (:boolean) true: boolean, %BB2, false: boolean, %BB8
// CHECK-NEXT:  %7 = PhiInst (:number) 0: number, %BB2, %15: number, %BB8
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = LoadFrameInst (:object) %0: environment, [%VS1.arr]: object
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: object, "push": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %12 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %9: object, %11: object
// CHECK-NEXT:  %13 = FAddInst (:number) %7: number, 1: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = PhiInst (:number) %7: number, %BB3, %13: number, %BB4
// CHECK-NEXT:  %16 = FLessThanInst (:boolean) %15: number, 100: number
// CHECK-NEXT:        CondBranchInst %16: boolean, %BB8, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst %BB7, %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:        ThrowInst %19: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: number, [%VS2.i]: number
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS2.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
