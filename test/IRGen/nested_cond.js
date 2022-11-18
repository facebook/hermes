/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo (a, b, c) {
    if (!(!a && c || b && !c))
        return 1;
    else
        return 2;
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

// CHECK:function foo(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:  %2 = StoreFrameInst %c, [c]
// CHECK-NEXT:  %3 = LoadFrameInst [a]
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = ReturnInst 2 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [b]
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB6, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst [c]
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = LoadFrameInst [c]
// CHECK-NEXT:  %13 = CondBranchInst %12, %BB3, %BB4
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:function_end
