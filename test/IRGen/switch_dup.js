/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo1(x) {
    switch (x) {
        case 10: return 1;
        case 10: return 2;
        case 11: return 3;
    }
}

function foo2(x) {
    switch (x) {
        case 10: return 1;
        case "10": return 2;
        case "10": return 3;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo1": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo1(): any
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo1": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %foo2(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo2": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function foo1(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       SwitchInst %2: any, %BB1, 10: number, %BB2, 11: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function foo2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       SwitchInst %2: any, %BB1, 10: number, %BB2, "10": string, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:       BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
