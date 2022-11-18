/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function bar() { return 1 }

function continue_test(cond) {
  while (cond) { continue; }
}

function break_test(cond) {
  while (cond) { break; }
}

function break_label(cond) {
  fail:
  while (cond) { break fail; }
}

function continue_label(cond) {
  fail:
  while (cond) { continue fail; }
}

function nested_label(cond) {
fail1:
  while (cond) {

fail2:
    while (cond) { continue fail2; }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [bar, continue_test, break_test, break_label, continue_label, nested_label]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %bar()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %continue_test()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "continue_test" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %break_test()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "break_test" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %break_label()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "break_label" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %continue_label()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "continue_label" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %nested_label()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "nested_label" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function bar()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function continue_test(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %cond, [cond]
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [cond]
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = LoadFrameInst [cond]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function break_test(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %cond, [cond]
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [cond]
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = LoadFrameInst [cond]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function break_label(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %cond, [cond]
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [cond]
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %7 = LoadFrameInst [cond]
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:function continue_label(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %cond, [cond]
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [cond]
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %7 = LoadFrameInst [cond]
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function nested_label(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %cond, [cond]
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [cond]
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %7 = LoadFrameInst [cond]
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %9 = BranchInst %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %10 = BranchInst %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %11 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %12 = BranchInst %BB8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = LoadFrameInst [cond]
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB9, %BB11
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %15 = LoadFrameInst [cond]
// CHECK-NEXT:  %16 = CondBranchInst %15, %BB9, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %17 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %18 = BranchInst %BB10
// CHECK-NEXT:function_end
