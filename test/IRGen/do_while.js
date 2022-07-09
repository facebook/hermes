/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL:function simple_do_while_test(cond)
//CHECK-NEXT:frame = [cond]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %cond, [cond]
//CHECK-NEXT:    %1 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %2 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %4 = BranchInst %BB2
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %5 = LoadFrameInst [cond]
//CHECK-NEXT:    %6 = CondBranchInst %5, %BB2, %BB4
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %7 = BranchInst %BB5
//CHECK-NEXT:function_end
function simple_do_while_test(cond) {
  do {
  } while (cond);
}

//CHECK-LABEL:function do_while_break_test(cond)
//CHECK-NEXT:frame = [cond]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %cond, [cond]
//CHECK-NEXT:    %1 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %2 = BranchInst %BB3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %4 = BranchInst %BB2
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %5 = LoadFrameInst [cond]
//CHECK-NEXT:    %6 = CondBranchInst %5, %BB2, %BB3
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %7 = BranchInst %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %8 = BranchInst %BB5
//CHECK-NEXT:function_end
function do_while_break_test(cond) {
  do {
   break;
  } while (cond);
}

//CHECK-LABEL:function do_while_continue_test(cond)
//CHECK-NEXT:frame = [cond]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %cond, [cond]
//CHECK-NEXT:    %1 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %2 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %4 = BranchInst %BB2
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %5 = LoadFrameInst [cond]
//CHECK-NEXT:    %6 = CondBranchInst %5, %BB2, %BB4
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %7 = BranchInst %BB5
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %8 = BranchInst %BB3
//CHECK-NEXT:function_end
function do_while_continue_test(cond) {
  do {
    continue;
  } while (cond);
}

//CHECK-LABEL:function for_while_do_mixed_test(cond)
//CHECK-NEXT:frame = [i, cond]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst %cond, [cond]
//CHECK-NEXT:    %2 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %3 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %4 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %5 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '<', %6, 10 : number
//CHECK-NEXT:    %8 = CondBranchInst %7, %BB2, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %9 = LoadFrameInst [i]
//CHECK-NEXT:    %10 = BinaryOperatorInst '<', %9, 10 : number
//CHECK-NEXT:    %11 = CondBranchInst %10, %BB2, %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %12 = LoadFrameInst [i]
//CHECK-NEXT:    %13 = AsNumericInst %12
//CHECK-NEXT:    %14 = UnaryOperatorInst '++', %13 : number|bigint
//CHECK-NEXT:    %15 = StoreFrameInst %14, [i]
//CHECK-NEXT:    %16 = BranchInst %BB5
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %17 = BranchInst %BB8
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %18 = BranchInst %BB6
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %19 = LoadFrameInst [cond]
//CHECK-NEXT:    %20 = CondBranchInst %19, %BB7, %BB9
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %21 = LoadFrameInst [cond]
//CHECK-NEXT:    %22 = CondBranchInst %21, %BB7, %BB9
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %23 = BranchInst %BB10
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %24 = BranchInst %BB13
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %25 = BranchInst %BB9
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %26 = BranchInst %BB12
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %27 = LoadFrameInst [cond]
//CHECK-NEXT:    %28 = CondBranchInst %27, %BB12, %BB14
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %29 = BranchInst %BB15
//CHECK-NEXT:  %BB16:
//CHECK-NEXT:    %30 = BranchInst %BB13
//CHECK-NEXT:  %BB17:
//CHECK-NEXT:    %31 = BranchInst %BB11
//CHECK-NEXT:function_end
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
