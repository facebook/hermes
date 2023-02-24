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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst 1: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = ReturnInst 2: number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %12 = CondBranchInst %11: any, %BB6, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:  %14 = CondBranchInst %13: any, %BB4, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:  %16 = CondBranchInst %15: any, %BB3, %BB4
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %18 = BranchInst %BB5
// CHECK-NEXT:function_end
