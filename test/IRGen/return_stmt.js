/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function test0(x,  y) {
  if (x) { return x; } else { return y; }
  return y;
}

function test1(x,  y) {
  if (x) { } else { return y; }
}

function test2(x,  y) {
  if (x) { return x; } else {  }
}

function test3(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

function test4(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

function test5() {
  return;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [test0, test1, test2, test3, test4, test5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test0#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test2#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test3#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test3" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %test4#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "test4" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %test5#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "test5" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function test0#0#1(x, y)#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test0#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1#0#1(x, y)#3
// CHECK-NEXT:frame = [x#3, y#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test1#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#3], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test2#0#1(x, y)#4
// CHECK-NEXT:frame = [x#4, y#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test2#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test3#0#1(x, y)#5
// CHECK-NEXT:frame = [x#5, y#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test3#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#5], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test4#0#1(x, y)#6
// CHECK-NEXT:frame = [x#6, y#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test4#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#6], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test5#0#1()#7
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test5#0#1()#7}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
