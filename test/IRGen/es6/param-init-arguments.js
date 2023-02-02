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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %2 = LoadParamInst %a
// CHECK-NEXT:  %3 = BinaryOperatorInst '!==', %2, undefined : undefined
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst %2, %BB0, %0 : object, %BB2
// CHECK-NEXT:  %7 = StoreFrameInst %6, [a]
// CHECK-NEXT:  %8 = LoadFrameInst [a]
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
