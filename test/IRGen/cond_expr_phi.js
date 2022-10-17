/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -dump-ir %s -O

function condExpr(a,b,c,d) {
    return a ? b || c : d;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [condExpr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %condExpr#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "condExpr" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function condExpr#0#1(a, b, c, d)#2
// CHECK-NEXT:frame = [a#2, b#2, c#2, d#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{condExpr#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst %d, [d#2], %0
// CHECK-NEXT:  %5 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [d#2], %0
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %10 = LoadFrameInst [b#2], %0
// CHECK-NEXT:  %11 = StoreStackInst %10, %9
// CHECK-NEXT:  %12 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = PhiInst %18, %BB4, %7, %BB2
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = LoadFrameInst [c#2], %0
// CHECK-NEXT:  %16 = StoreStackInst %15, %9
// CHECK-NEXT:  %17 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadStackInst %9
// CHECK-NEXT:  %19 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
