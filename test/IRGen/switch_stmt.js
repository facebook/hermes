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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [days_of_the_week, simple_xor, simple_xor2, simple_test0, simple_test1, fallthrough]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %days_of_the_week#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "days_of_the_week" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simple_xor#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simple_xor" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %simple_xor2#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "simple_xor2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %simple_test0#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "simple_test0" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %simple_test1#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "simple_test1" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %fallthrough#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "fallthrough" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function days_of_the_week#0#1(day, x)#2
// CHECK-NEXT:frame = [day#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{days_of_the_week#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %day, [day#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [day#2], %0
// CHECK-NEXT:  %4 = BinaryOperatorInst '===', 0 : number, %3
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = LoadFrameInst [day#2], %0
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = StoreFrameInst "?" : string, [day#2], %0
// CHECK-NEXT:  %9 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = StoreFrameInst "Sunday" : string, [day#2], %0
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = BinaryOperatorInst '===', 1 : number, %3
// CHECK-NEXT:  %13 = CondBranchInst %12, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = StoreFrameInst "Monday" : string, [day#2], %0
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '===', %16, %3
// CHECK-NEXT:  %18 = CondBranchInst %17, %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %19 = StoreFrameInst "Tuesday" : string, [day#2], %0
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %21 = BinaryOperatorInst '===', 3 : number, %3
// CHECK-NEXT:  %22 = CondBranchInst %21, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %23 = StoreFrameInst "Wednesday" : string, [day#2], %0
// CHECK-NEXT:  %24 = BranchInst %BB3
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %25 = BinaryOperatorInst '===', 4 : number, %3
// CHECK-NEXT:  %26 = CondBranchInst %25, %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %27 = StoreFrameInst "Thursday" : string, [day#2], %0
// CHECK-NEXT:  %28 = BranchInst %BB3
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %29 = BinaryOperatorInst '===', 5 : number, %3
// CHECK-NEXT:  %30 = CondBranchInst %29, %BB13, %BB14
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %31 = StoreFrameInst "Friday" : string, [day#2], %0
// CHECK-NEXT:  %32 = BranchInst %BB3
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %33 = BinaryOperatorInst '===', 6 : number, %3
// CHECK-NEXT:  %34 = CondBranchInst %33, %BB15, %BB16
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %35 = StoreFrameInst "Saturday" : string, [day#2], %0
// CHECK-NEXT:  %36 = BranchInst %BB3
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %37 = BranchInst %BB4
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %38 = BranchInst %BB5
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %39 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %40 = BranchInst %BB9
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %41 = BranchInst %BB11
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %42 = BranchInst %BB13
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %43 = BranchInst %BB15
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %44 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_xor#0#1(b)#3
// CHECK-NEXT:frame = [b#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_xor#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [b#3], %0
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

// CHECK:function simple_xor2#0#1(b)#4
// CHECK-NEXT:frame = [b#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_xor2#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [b#4], %0
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

// CHECK:function simple_test0#0#1(b)#5
// CHECK-NEXT:frame = [b#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_test0#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b#5], %0
// CHECK-NEXT:  %2 = LoadFrameInst [b#5], %0
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

// CHECK:function simple_test1#0#1(b)#6
// CHECK-NEXT:frame = [b#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_test1#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b#6], %0
// CHECK-NEXT:  %2 = LoadFrameInst [b#6], %0
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

// CHECK:function fallthrough#0#1(b)#7
// CHECK-NEXT:frame = [b#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{fallthrough#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b#7], %0
// CHECK-NEXT:  %2 = LoadFrameInst [b#7], %0
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
