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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [test0, test1, test2, test3, test4, test5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %test0()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "test0" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test3()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "test3" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %test4()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "test4" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %test5()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "test5" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function test0(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [y]
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test2(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test3(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [x]
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test4(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [x]
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test5()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
