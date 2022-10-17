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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b, c)#2
// CHECK-NEXT:frame = [a#2, b#2, c#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = ReturnInst 2 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [b#2], %0
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB6, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadFrameInst [c#2], %0
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB4, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst [c#2], %0
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB3, %BB4
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %16 = BranchInst %BB5
// CHECK-NEXT:function_end
