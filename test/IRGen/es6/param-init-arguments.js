/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

// Ensure that we can use "arguments" when initializing formal parameters.

function foo(a = arguments) {
    return a;
}
//CHECK-LABEL:function foo(a)
//CHECK-NEXT:frame = [a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %a, undefined : undefined
//CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %3 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = PhiInst %a, %BB0, %0 : object, %BB2
//CHECK-NEXT:  %5 = StoreFrameInst %4, [a]
//CHECK-NEXT:  %6 = LoadFrameInst [a]
//CHECK-NEXT:  %7 = ReturnInst %6
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
