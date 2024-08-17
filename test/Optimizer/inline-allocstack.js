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
// CHECK-NEXT:       TryStartInst %BB7, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreStackInst 0: number, %1: number
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS2.i]: number
// CHECK-NEXT:  %8 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %8: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadStackInst (:number) %1: number
// CHECK-NEXT:        StoreStackInst %11: number, %2: number
// CHECK-NEXT:  %13 = LoadStackInst (:boolean) %8: boolean
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:object) %0: environment, [%VS1.arr]: object
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %15: object, "push": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %3: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %18 = CallInst (:any) %16: any, empty: any, false: boolean, empty: any, undefined: undefined, %15: object, %17: object
// CHECK-NEXT:  %19 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %20 = FAddInst (:number) %19: number, 1: number
// CHECK-NEXT:        StoreStackInst %20: number, %2: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %23 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %24 = FLessThanInst (:boolean) %23: number, 100: number
// CHECK-NEXT:        CondBranchInst %24: boolean, %BB8, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst %BB7, %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:        ThrowInst %27: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = LoadStackInst (:number) %2: number
// CHECK-NEXT:        StoreStackInst %29: number, %1: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %29: number, [%VS2.i]: number
// CHECK-NEXT:        StoreStackInst false: boolean, %8: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS2.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
