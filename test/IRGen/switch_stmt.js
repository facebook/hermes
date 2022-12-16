/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

function simple_xor(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  }
  return "invalid";
}

function simple_xor2(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  default: return "invalid"
  }
}

function simple_test0(b) {
  switch (b) {
  case 1+2: return 4+5;
  case 2+3: return 6+7;
  default: return 8+9;
  }
}

function simple_test1(b) {
  switch (b) {
  case 1+2: return 4+5;
  case 2+3: break;
  default: return 8+9;
  }
}

function fallthrough(b) {
  switch (b) {
  case 0: null
  case 1: null
  default: null
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [days_of_the_week, simple_xor, simple_xor2, simple_test0, simple_test1, fallthrough]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %days_of_the_week()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "days_of_the_week" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %simple_xor()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "simple_xor" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %simple_xor2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "simple_xor2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %simple_test0()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "simple_test0" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %simple_test1()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "simple_test1" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %fallthrough()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "fallthrough" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function days_of_the_week(day, x)
// CHECK-NEXT:frame = [day, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %day
// CHECK-NEXT:  %1 = StoreFrameInst %0, [day]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [day]
// CHECK-NEXT:  %5 = BinaryOperatorInst '===', 0 : number, %4
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadFrameInst [day]
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = StoreFrameInst "?" : string, [day]
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = StoreFrameInst "Sunday" : string, [day]
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = BinaryOperatorInst '===', 1 : number, %4
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = StoreFrameInst "Monday" : string, [day]
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = LoadFrameInst [x]
// CHECK-NEXT:  %18 = BinaryOperatorInst '===', %17, %4
// CHECK-NEXT:  %19 = CondBranchInst %18, %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = StoreFrameInst "Tuesday" : string, [day]
// CHECK-NEXT:  %21 = BranchInst %BB3
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = BinaryOperatorInst '===', 3 : number, %4
// CHECK-NEXT:  %23 = CondBranchInst %22, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %24 = StoreFrameInst "Wednesday" : string, [day]
// CHECK-NEXT:  %25 = BranchInst %BB3
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = BinaryOperatorInst '===', 4 : number, %4
// CHECK-NEXT:  %27 = CondBranchInst %26, %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %28 = StoreFrameInst "Thursday" : string, [day]
// CHECK-NEXT:  %29 = BranchInst %BB3
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %30 = BinaryOperatorInst '===', 5 : number, %4
// CHECK-NEXT:  %31 = CondBranchInst %30, %BB13, %BB14
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %32 = StoreFrameInst "Friday" : string, [day]
// CHECK-NEXT:  %33 = BranchInst %BB3
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = BinaryOperatorInst '===', 6 : number, %4
// CHECK-NEXT:  %35 = CondBranchInst %34, %BB15, %BB16
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %36 = StoreFrameInst "Saturday" : string, [day]
// CHECK-NEXT:  %37 = BranchInst %BB3
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %38 = BranchInst %BB4
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %39 = BranchInst %BB5
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %40 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %41 = BranchInst %BB9
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %42 = BranchInst %BB11
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %43 = BranchInst %BB13
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %44 = BranchInst %BB15
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %45 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_xor(b)
// CHECK-NEXT:frame = [b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %b
// CHECK-NEXT:  %1 = StoreFrameInst %0, [b]
// CHECK-NEXT:  %2 = LoadFrameInst [b]
// CHECK-NEXT:  %3 = SwitchInst %2, %BB1, 1 : number, %BB2, 0 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst "invalid" : string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 0 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst 1 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_xor2(b)
// CHECK-NEXT:frame = [b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %b
// CHECK-NEXT:  %1 = StoreFrameInst %0, [b]
// CHECK-NEXT:  %2 = LoadFrameInst [b]
// CHECK-NEXT:  %3 = SwitchInst %2, %BB1, 1 : number, %BB2, 0 : number, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 0 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst 1 : number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst "invalid" : string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %10 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_test0(b)
// CHECK-NEXT:frame = [b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %b
// CHECK-NEXT:  %1 = StoreFrameInst %0, [b]
// CHECK-NEXT:  %2 = LoadFrameInst [b]
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '===', %3, %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', 4 : number, 5 : number
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', 2 : number, 3 : number
// CHECK-NEXT:  %10 = BinaryOperatorInst '===', %9, %2
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', 6 : number, 7 : number
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', 8 : number, 9 : number
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = BranchInst %BB4
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %19 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_test1(b)
// CHECK-NEXT:frame = [b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %b
// CHECK-NEXT:  %1 = StoreFrameInst %0, [b]
// CHECK-NEXT:  %2 = LoadFrameInst [b]
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '===', %3, %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', 4 : number, 5 : number
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', 2 : number, 3 : number
// CHECK-NEXT:  %10 = BinaryOperatorInst '===', %9, %2
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', 8 : number, 9 : number
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %17 = BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function fallthrough(b)
// CHECK-NEXT:frame = [b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %b
// CHECK-NEXT:  %1 = StoreFrameInst %0, [b]
// CHECK-NEXT:  %2 = LoadFrameInst [b]
// CHECK-NEXT:  %3 = SwitchInst %2, %BB1, 0 : number, %BB2, 1 : number, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB4
// CHECK-NEXT:function_end
