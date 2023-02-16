/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that we can use "arguments" when initializing formal parameters.

function foo(a = arguments) {
    return a;
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
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %3 = BinaryStrictlyNotEqualInst (:any) %2: any, undefined: undefined
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst (:any) %2: any, %BB0, %0: object, %BB2
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [a]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %9 = ReturnInst (:any) %8: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
