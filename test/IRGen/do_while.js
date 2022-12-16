/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function simple_do_while_test(cond) {
  do {
  } while (cond);
}

function do_while_break_test(cond) {
  do {
   break;
  } while (cond);
}

function do_while_continue_test(cond) {
  do {
    continue;
  } while (cond);
}

function for_while_do_mixed_test(cond) {
  for (var i = 0; i < 10; i++) {
    while (cond) {
      do {
        continue;
      } while (cond);
      break;
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [simple_do_while_test, do_while_break_test, do_while_continue_test, for_while_do_mixed_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %simple_do_while_test()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "simple_do_while_test" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %do_while_break_test()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "do_while_break_test" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %do_while_continue_test()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "do_while_continue_test" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %for_while_do_mixed_test()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "for_while_do_mixed_test" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function simple_do_while_test(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %cond
// CHECK-NEXT:  %1 = StoreFrameInst %0, [cond]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = LoadFrameInst [cond]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function do_while_break_test(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %cond
// CHECK-NEXT:  %1 = StoreFrameInst %0, [cond]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = LoadFrameInst [cond]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function do_while_continue_test(cond)
// CHECK-NEXT:frame = [cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %cond
// CHECK-NEXT:  %1 = StoreFrameInst %0, [cond]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = LoadFrameInst [cond]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function for_while_do_mixed_test(cond)
// CHECK-NEXT:frame = [i, cond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = LoadParamInst %cond
// CHECK-NEXT:  %2 = StoreFrameInst %1, [cond]
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [i]
// CHECK-NEXT:  %8 = BinaryOperatorInst '<', %7, 10 : number
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst [i]
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB2, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst [i]
// CHECK-NEXT:  %14 = AsNumericInst %13
// CHECK-NEXT:  %15 = UnaryOperatorInst '++', %14 : number|bigint
// CHECK-NEXT:  %16 = StoreFrameInst %15, [i]
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %18 = BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadFrameInst [cond]
// CHECK-NEXT:  %21 = CondBranchInst %20, %BB7, %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %22 = LoadFrameInst [cond]
// CHECK-NEXT:  %23 = CondBranchInst %22, %BB7, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %24 = BranchInst %BB10
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %25 = BranchInst %BB13
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %26 = BranchInst %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %27 = BranchInst %BB12
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %28 = LoadFrameInst [cond]
// CHECK-NEXT:  %29 = CondBranchInst %28, %BB12, %BB14
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %30 = BranchInst %BB15
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %31 = BranchInst %BB13
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %32 = BranchInst %BB11
// CHECK-NEXT:function_end
