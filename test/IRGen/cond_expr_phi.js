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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [condExpr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %condExpr()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "condExpr" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function condExpr(a, b, c, d)
// CHECK-NEXT:frame = [a, b, c, d]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadParamInst %c
// CHECK-NEXT:  %5 = StoreFrameInst %4, [c]
// CHECK-NEXT:  %6 = LoadParamInst %d
// CHECK-NEXT:  %7 = StoreFrameInst %6, [d]
// CHECK-NEXT:  %8 = LoadFrameInst [a]
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst [d]
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %13 = LoadFrameInst [b]
// CHECK-NEXT:  %14 = StoreStackInst %13, %12
// CHECK-NEXT:  %15 = CondBranchInst %13, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = PhiInst %21, %BB4, %10, %BB2
// CHECK-NEXT:  %17 = ReturnInst %16
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = LoadFrameInst [c]
// CHECK-NEXT:  %19 = StoreStackInst %18, %12
// CHECK-NEXT:  %20 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = LoadStackInst %12
// CHECK-NEXT:  %22 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
