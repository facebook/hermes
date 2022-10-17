/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a, b) {
  return a ?? b;
}

function f2(a, b) {
  if (a ?? b) {
    return 1;
  } else {
    return 2;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [f1, f2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %f1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %f2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function f1#0#1(a, b)#2
// CHECK-NEXT:frame = [a#2, b#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %4 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %5 = StoreStackInst %4, %3
// CHECK-NEXT:  %6 = BinaryOperatorInst '==', %4, null : null
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [b#2], %0
// CHECK-NEXT:  %9 = StoreStackInst %8, %3
// CHECK-NEXT:  %10 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst %3
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2#0#1(a, b)#3
// CHECK-NEXT:frame = [a#3, b#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#3], %0
// CHECK-NEXT:  %3 = LoadFrameInst [a#3], %0
// CHECK-NEXT:  %4 = BinaryOperatorInst '==', %3, null : null
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = ReturnInst 2 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [b#3], %0
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = CondBranchInst %3, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %13 = BranchInst %BB5
// CHECK-NEXT:function_end
