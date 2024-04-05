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
// CHECK-NEXT:  %1 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %2 = AllocStackInst (:number) $i: any
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       TryStartInst %BB8, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       StoreStackInst 0: number, %1: number
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS2.i]: number
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %7: boolean
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadStackInst (:number) %1: number
// CHECK-NEXT:        StoreStackInst %10: number, %2: number
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:object) %0: environment, [%VS1.arr]: object
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %14: object, "push": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %3: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %17 = CallInst (:any) %15: any, empty: any, empty: any, undefined: undefined, %14: object, %16: object
// CHECK-NEXT:  %18 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %19 = FAddInst (:number) %18: number, 1: number
// CHECK-NEXT:        StoreStackInst %19: number, %2: number
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %23 = FLessThanInst (:boolean) %22: number, 100: number
// CHECK-NEXT:        CondBranchInst %23: boolean, %BB7, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %28 = LoadStackInst (:number) %2: number
// CHECK-NEXT:        StoreStackInst %28: number, %1: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %28: number, [%VS2.i]: number
// CHECK-NEXT:        StoreStackInst false: boolean, %7: boolean
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %33 = CatchInst (:any)
// CHECK-NEXT:        ThrowInst %33: any
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS2.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
