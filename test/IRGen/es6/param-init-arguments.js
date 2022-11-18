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
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %a, undefined : undefined
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst %a, %BB0, %0 : object, %BB2
// CHECK-NEXT:  %5 = StoreFrameInst %4, [a]
// CHECK-NEXT:  %6 = LoadFrameInst [a]
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
