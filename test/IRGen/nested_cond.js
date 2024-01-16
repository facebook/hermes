/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo (a, b, c) {
    if (!(!a && c || b && !c))
        return 1;
    else
        return 2;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB5, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:        CondBranchInst %12: any, %BB4, %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:        CondBranchInst %14: any, %BB3, %BB4
// CHECK-NEXT:function_end
