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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [bar, continue_test, break_test, break_label, continue_label, nested_label]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %bar#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %continue_test#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "continue_test" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %break_test#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "break_test" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %break_label#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "break_label" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %continue_label#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "continue_label" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %nested_label#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "nested_label" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function bar#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function continue_test#0#1(cond)#3
// CHECK-NEXT:frame = [cond#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{continue_test#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %cond, [cond#3], %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [cond#3], %0
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %7 = LoadFrameInst [cond#3], %0
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function break_test#0#1(cond)#4
// CHECK-NEXT:frame = [cond#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{break_test#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %cond, [cond#4], %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [cond#4], %0
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst [cond#4], %0
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %10 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function break_label#0#1(cond)#5
// CHECK-NEXT:frame = [cond#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{break_label#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %cond, [cond#5], %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [cond#5], %0
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = LoadFrameInst [cond#5], %0
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %10 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %11 = BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:function continue_label#0#1(cond)#6
// CHECK-NEXT:frame = [cond#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{continue_label#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %cond, [cond#6], %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [cond#6], %0
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = LoadFrameInst [cond#6], %0
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function nested_label#0#1(cond)#7
// CHECK-NEXT:frame = [cond#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nested_label#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %cond, [cond#7], %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [cond#7], %0
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = LoadFrameInst [cond#7], %0
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %11 = BranchInst %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %12 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %13 = BranchInst %BB8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst [cond#7], %0
// CHECK-NEXT:  %15 = CondBranchInst %14, %BB9, %BB11
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %16 = LoadFrameInst [cond#7], %0
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB9, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %18 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %19 = BranchInst %BB10
// CHECK-NEXT:function_end
