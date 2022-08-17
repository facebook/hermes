/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL: function days_of_the_week(day, x)
//CHECK-NEXT: frame = [day, x]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %day, [day]
//CHECK-NEXT:   %1 = StoreFrameInst %x, [x]
//CHECK-NEXT:   %2 = LoadFrameInst [day]
//CHECK-NEXT:   %3 = BinaryOperatorInst '===', 0 : number, %2
//CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %5 = LoadFrameInst [day]
//CHECK-NEXT:   %6 = ReturnInst %5
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %7 = StoreFrameInst "?" : string, [day]
//CHECK-NEXT:   %8 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %9 = StoreFrameInst "Sunday" : string, [day]
//CHECK-NEXT:   %10 = BranchInst %BB3
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %11 = BinaryOperatorInst '===', 1 : number, %2
//CHECK-NEXT:   %12 = CondBranchInst %11, %BB5, %BB6
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %13 = StoreFrameInst "Monday" : string, [day]
//CHECK-NEXT:   %14 = BranchInst %BB3
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %15 = LoadFrameInst [x]
//CHECK-NEXT:   %16 = BinaryOperatorInst '===', %15, %2
//CHECK-NEXT:   %17 = CondBranchInst %16, %BB7, %BB8
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %18 = StoreFrameInst "Tuesday" : string, [day]
//CHECK-NEXT:   %19 = BranchInst %BB3
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %20 = BinaryOperatorInst '===', 3 : number, %2
//CHECK-NEXT:   %21 = CondBranchInst %20, %BB9, %BB10
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %22 = StoreFrameInst "Wednesday" : string, [day]
//CHECK-NEXT:   %23 = BranchInst %BB3
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   %24 = BinaryOperatorInst '===', 4 : number, %2
//CHECK-NEXT:   %25 = CondBranchInst %24, %BB11, %BB12
//CHECK-NEXT: %BB11:
//CHECK-NEXT:   %26 = StoreFrameInst "Thursday" : string, [day]
//CHECK-NEXT:   %27 = BranchInst %BB3
//CHECK-NEXT: %BB12:
//CHECK-NEXT:   %28 = BinaryOperatorInst '===', 5 : number, %2
//CHECK-NEXT:   %29 = CondBranchInst %28, %BB13, %BB14
//CHECK-NEXT: %BB13:
//CHECK-NEXT:   %30 = StoreFrameInst "Friday" : string, [day]
//CHECK-NEXT:   %31 = BranchInst %BB3
//CHECK-NEXT: %BB14:
//CHECK-NEXT:   %32 = BinaryOperatorInst '===', 6 : number, %2
//CHECK-NEXT:   %33 = CondBranchInst %32, %BB15, %BB16
//CHECK-NEXT: %BB15:
//CHECK-NEXT:   %34 = StoreFrameInst "Saturday" : string, [day]
//CHECK-NEXT:   %35 = BranchInst %BB3
//CHECK-NEXT: %BB16:
//CHECK-NEXT:   %36 = BranchInst %BB4
//CHECK-NEXT: %BB17:
//CHECK-NEXT:   %37 = BranchInst %BB5
//CHECK-NEXT: %BB18:
//CHECK-NEXT:   %38 = BranchInst %BB7
//CHECK-NEXT: %BB19:
//CHECK-NEXT:   %39 = BranchInst %BB9
//CHECK-NEXT: %BB20:
//CHECK-NEXT:   %40 = BranchInst %BB11
//CHECK-NEXT: %BB21:
//CHECK-NEXT:   %41 = BranchInst %BB13
//CHECK-NEXT: %BB22:
//CHECK-NEXT:   %42 = BranchInst %BB15
//CHECK-NEXT: %BB23:
//CHECK-NEXT:   %43 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function days_of_the_week(day, x) {
  switch (day) {
  default:
    day = "?";
  case 0:
    day = "Sunday";
    break;
  case 1:
    day = "Monday";
    break;
  case x:
    day = "Tuesday";
    break;
  case 3:
    day = "Wednesday";
    break;
  case 4:
    day = "Thursday";
    break;
  case 5:
    day = "Friday";
    break;
  case 6:
    day = "Saturday";
  }
  return day;
}

//CHECK-LABEL: function simple_xor(b)
//CHECK-NEXT: frame = [b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %b, [b]
//CHECK-NEXT:   %1 = LoadFrameInst [b]
//CHECK-NEXT:   %2 = SwitchInst %1, %BB1, 1 : number, %BB2, 0 : number, %BB3
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %3 = ReturnInst "invalid" : string
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %4 = ReturnInst 0 : number
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %5 = BranchInst %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %6 = ReturnInst 1 : number
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %7 = BranchInst %BB1
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %8 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function simple_xor(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  }
  return "invalid";
}

//CHECK-LABEL: function simple_xor2(b)
//CHECK-NEXT: frame = [b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %b, [b]
//CHECK-NEXT:   %1 = LoadFrameInst [b]
//CHECK-NEXT:   %2 = SwitchInst %1, %BB1, 1 : number, %BB2, 0 : number, %BB3
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %3 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %4 = ReturnInst 0 : number
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %5 = BranchInst %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %6 = ReturnInst 1 : number
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %7 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %8 = ReturnInst "invalid" : string
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %9 = BranchInst %BB4
//CHECK-NEXT: function_end
function simple_xor2(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  default: return "invalid"
  }
}

//CHECK-LABEL: function simple_test0(b)
//CHECK-NEXT: frame = [b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %b, [b]
//CHECK-NEXT:   %1 = LoadFrameInst [b]
//CHECK-NEXT:   %2 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:   %3 = BinaryOperatorInst '===', %2, %1
//CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %6 = BinaryOperatorInst '+', 4 : number, 5 : number
//CHECK-NEXT:   %7 = ReturnInst %6
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %8 = BinaryOperatorInst '+', 2 : number, 3 : number
//CHECK-NEXT:   %9 = BinaryOperatorInst '===', %8, %1
//CHECK-NEXT:   %10 = CondBranchInst %9, %BB4, %BB5
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %11 = BinaryOperatorInst '+', 6 : number, 7 : number
//CHECK-NEXT:   %12 = ReturnInst %11
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %13 = BranchInst %BB6
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %14 = BinaryOperatorInst '+', 8 : number, 9 : number
//CHECK-NEXT:   %15 = ReturnInst %14
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %16 = BranchInst %BB4
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %17 = BranchInst %BB6
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %18 = BranchInst %BB3
//CHECK-NEXT: function_end
function simple_test0(b) {
  switch (b) {
  case 1+2: return 4+5;
  case 2+3: return 6+7;
  default: return 8+9;
  }
}

//CHECK-LABEL: function simple_test1(b)
//CHECK-NEXT: frame = [b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %b, [b]
//CHECK-NEXT:   %1 = LoadFrameInst [b]
//CHECK-NEXT:   %2 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:   %3 = BinaryOperatorInst '===', %2, %1
//CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %6 = BinaryOperatorInst '+', 4 : number, 5 : number
//CHECK-NEXT:   %7 = ReturnInst %6
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %8 = BinaryOperatorInst '+', 2 : number, 3 : number
//CHECK-NEXT:   %9 = BinaryOperatorInst '===', %8, %1
//CHECK-NEXT:   %10 = CondBranchInst %9, %BB4, %BB5
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %11 = BranchInst %BB3
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %12 = BranchInst %BB6
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %13 = BinaryOperatorInst '+', 8 : number, 9 : number
//CHECK-NEXT:   %14 = ReturnInst %13
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %15 = BranchInst %BB4
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %16 = BranchInst %BB6
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %17 = BranchInst %BB3
//CHECK-NEXT: function_end
function simple_test1(b) {
  switch (b) {
  case 1+2: return 4+5;
  case 2+3: break;
  default: return 8+9;
  }
}

//CHECK-LABEL: function fallthrough(b)
//CHECK-NEXT: frame = [b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %b, [b]
//CHECK-NEXT:   %1 = LoadFrameInst [b]
//CHECK-NEXT:   %2 = SwitchInst %1, %BB1, 0 : number, %BB2, 1 : number, %BB3
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %3 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %4 = BranchInst %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %5 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %6 = BranchInst %BB4
//CHECK-NEXT: function_end
function fallthrough(b) {
  switch (b) {
  case 0: null
  case 1: null
  default: null
  }
}

