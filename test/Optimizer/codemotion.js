/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra %s -O | %FileCheckOrRegen --match-full-lines %s

function hoist_branch(x, y) {
  if (x) {
    return y.z.k.r;
  } else {
    return y.z.k.t;
  }
}

function hoist_branch_window(x, y) {
  var n = y | 0;

  if (x) {
    var z = n + n;
    return y.z.k.r + z;
  } else {
    return y.z.k.t;
  }
}

function no_hoist_inc_dec(x, y) {
  if (x) {
    y++;
  } else {
    y--;
  }
  return y;
}

function hoist_loop(x) {
  for (var i = 0; i < x; i++) {
    print(i);
  }
}

function hoist_loop_expression(a, b, call) {
  a = +a;
  b = +b;
  for (;;) {
    call(-a * (b + 7));
  }
}

function hoist_from_multiblock_loop(x) {
  x = +x;
  for (;;) {
    print(3 * x * x);
    if (x - 1) {
      print(3 * x * x);
    }
  }
}

function hoist_with_dependencies_in_loop(x, y) {
  x = +x;
  for (;;) {
    var x2 = x * x;
    if (y) {
      return y;
    }
    print(x2 - 3);
  }
}

function code_sinking(x, y) {
  var n = (y | 0) + 2;
  var k = (n) + 9;

  if (x) {
    return n;
  } else {
    return k;
  }
}

function code_sinking_in_loop(x, y) {
  for (;;) {
    var obj = {};
    if (y) {
      return;
    }
    obj[x] = y;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [hoist_branch, hoist_branch_window, no_hoist_inc_dec, hoist_loop, hoist_loop_expression, hoist_from_multiblock_loop, hoist_with_dependencies_in_loop, code_sinking, code_sinking_in_loop]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...19) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCCreateFunctionInst %hoist_branch#0#1()#2, %0
// CHECK-NEXT:  $Reg1 @2 [3...20) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "hoist_branch" : string
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%4 = HBCCreateFunctionInst %hoist_branch_window#0#1()#3, %0
// CHECK-NEXT:  $Reg2 @5 [empty]	%5 = StorePropertyInst %4 : closure, %2 : object, "hoist_branch_window" : string
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%6 = HBCCreateFunctionInst %no_hoist_inc_dec#0#1()#4 : number|bigint, %0
// CHECK-NEXT:  $Reg2 @7 [empty]	%7 = StorePropertyInst %6 : closure, %2 : object, "no_hoist_inc_dec" : string
// CHECK-NEXT:  $Reg2 @8 [9...10) 	%8 = HBCCreateFunctionInst %hoist_loop#0#1()#5 : undefined, %0
// CHECK-NEXT:  $Reg2 @9 [empty]	%9 = StorePropertyInst %8 : closure, %2 : object, "hoist_loop" : string
// CHECK-NEXT:  $Reg2 @10 [11...12) 	%10 = HBCCreateFunctionInst %hoist_loop_expression#0#1()#6 : undefined, %0
// CHECK-NEXT:  $Reg2 @11 [empty]	%11 = StorePropertyInst %10 : closure, %2 : object, "hoist_loop_expression" : string
// CHECK-NEXT:  $Reg2 @12 [13...14) 	%12 = HBCCreateFunctionInst %hoist_from_multiblock_loop#0#1()#7 : undefined, %0
// CHECK-NEXT:  $Reg2 @13 [empty]	%13 = StorePropertyInst %12 : closure, %2 : object, "hoist_from_multiblock_loop" : string
// CHECK-NEXT:  $Reg2 @14 [15...16) 	%14 = HBCCreateFunctionInst %hoist_with_dependencies_in_loop#0#1()#8, %0
// CHECK-NEXT:  $Reg2 @15 [empty]	%15 = StorePropertyInst %14 : closure, %2 : object, "hoist_with_dependencies_in_loop" : string
// CHECK-NEXT:  $Reg2 @16 [17...18) 	%16 = HBCCreateFunctionInst %code_sinking#0#1()#9 : number, %0
// CHECK-NEXT:  $Reg2 @17 [empty]	%17 = StorePropertyInst %16 : closure, %2 : object, "code_sinking" : string
// CHECK-NEXT:  $Reg0 @18 [19...20) 	%18 = HBCCreateFunctionInst %code_sinking_in_loop#0#1()#10 : undefined, %0
// CHECK-NEXT:  $Reg0 @19 [empty]	%19 = StorePropertyInst %18 : closure, %2 : object, "code_sinking_in_loop" : string
// CHECK-NEXT:  $Reg0 @20 [21...22) 	%20 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @21 [empty]	%21 = ReturnInst %20 : undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_branch#0#1(x, y)#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg0 @1 [2...3) 	%1 = LoadPropertyInst %0, "z" : string
// CHECK-NEXT:  $Reg0 @2 [3...8) 	%2 = LoadPropertyInst %1, "k" : string
// CHECK-NEXT:  $Reg1 @3 [4...5) 	%3 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg1 @4 [empty]	%4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%5 = LoadPropertyInst %2, "r" : string
// CHECK-NEXT:  $Reg0 @8 [empty]	%6 = ReturnInst %5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg1 @5 [6...7) 	%7 = LoadPropertyInst %2, "t" : string
// CHECK-NEXT:  $Reg1 @6 [empty]	%8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function hoist_branch_window#0#1(x, y)#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 @0 [1...3) 	%0 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg0 @1 [2...10) 	%1 = AsInt32Inst %0
// CHECK-NEXT:  $Reg1 @2 [3...4) 	%2 = LoadPropertyInst %0, "z" : string
// CHECK-NEXT:  $Reg1 @3 [4...9) 	%3 = LoadPropertyInst %2, "k" : string
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%4 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2 @5 [empty]	%5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 @8 [9...11) 	%6 = LoadPropertyInst %3, "r" : string
// CHECK-NEXT:  $Reg0 @9 [10...11) 	%7 = BinaryOperatorInst '+', %1 : number, %1 : number
// CHECK-NEXT:  $Reg0 @10 [11...12) 	%8 = BinaryOperatorInst '+', %6, %7 : number
// CHECK-NEXT:  $Reg0 @11 [empty]	%9 = ReturnInst %8 : string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%10 = LoadPropertyInst %3, "t" : string
// CHECK-NEXT:  $Reg2 @7 [empty]	%11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function no_hoist_inc_dec#0#1(x, y)#2 : number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 @0 [1...7) 	%0 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg0 @1 [2...3) 	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @2 [empty]	%2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 @6 [7...8) 	%3 = UnaryOperatorInst '++', %0
// CHECK-NEXT:  $Reg0 @7 [8...10) 	%4 = MovInst %3 : number|bigint
// CHECK-NEXT:  $Reg1 @8 [empty]	%5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @3 [4...5) 	%6 = UnaryOperatorInst '--', %0
// CHECK-NEXT:  $Reg0 @4 [5...10) 	%7 = MovInst %6 : number|bigint
// CHECK-NEXT:  $Reg2 @5 [empty]	%8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @9 [4...11) 	%9 = PhiInst %4 : number|bigint, %BB1, %7 : number|bigint, %BB2
// CHECK-NEXT:  $Reg0 @10 [4...12) 	%10 = MovInst %9 : number|bigint
// CHECK-NEXT:  $Reg0 @11 [empty]	%11 = ReturnInst %10 : number|bigint
// CHECK-NEXT:function_end

// CHECK:function hoist_loop#0#1(x)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg3 @0 [1...12) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2 @1 [2...5) 	%1 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg1 @2 [3...12) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @3 [4...13) 	%3 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg2 @4 [5...7) 	%4 = MovInst %1 : number
// CHECK-NEXT:  $Reg4 @5 [empty]	%5 = CompareBranchInst '<', %4 : number, %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg2 @6 [2...12) 	%6 = PhiInst %4 : number, %BB0, %10 : number|bigint, %BB1
// CHECK-NEXT:  $Reg4 @7 [8...9) 	%7 = TryLoadGlobalPropertyInst %2 : object, "print" : string
// CHECK-NEXT:  $Reg4 @8 [empty]	%8 = HBCCallNInst %7, %3 : undefined, %6 : number|bigint
// CHECK-NEXT:  $Reg2 @9 [10...11) 	%9 = UnaryOperatorInst '++', %6 : number|bigint
// CHECK-NEXT:  $Reg2 @10 [11...12) 	%10 = MovInst %9 : number|bigint
// CHECK-NEXT:  $Reg1 @11 [empty]	%11 = CompareBranchInst '<', %10 : number|bigint, %0, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @12 [empty]	%12 = ReturnInst %3 : undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_loop_expression#0#1(a, b, call)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg2 @0 [1...13) 	%0 = HBCLoadParamInst 3 : number
// CHECK-NEXT:  $Reg1 @1 [2...13) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @3 [4...7) 	%3 = AsNumberInst %2
// CHECK-NEXT:  $Reg3 @4 [5...6) 	%4 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg4 @5 [6...9) 	%5 = AsNumberInst %4
// CHECK-NEXT:  $Reg3 @6 [7...10) 	%6 = UnaryOperatorInst '-', %3 : number
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%7 = HBCLoadConstInst 7 : number
// CHECK-NEXT:  $Reg0 @8 [9...10) 	%8 = BinaryOperatorInst '+', %5 : number, %7 : number
// CHECK-NEXT:  $Reg0 @9 [10...13) 	%9 = BinaryOperatorInst '*', %6 : number, %8 : number
// CHECK-NEXT:  $Reg3 @10 [empty]	%10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg3 @11 [empty]	%11 = HBCCallNInst %0, %1 : undefined, %9 : number
// CHECK-NEXT:  $Reg0 @12 [empty]	%12 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_from_multiblock_loop#0#1(x)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg3 @0 [1...16) 	%0 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @1 [2...16) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg4 @3 [4...9) 	%3 = AsNumberInst %2
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg0 @5 [6...7) 	%5 = BinaryOperatorInst '*', %4 : number, %3 : number
// CHECK-NEXT:  $Reg1 @6 [7...16) 	%6 = BinaryOperatorInst '*', %5 : number, %3 : number
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%7 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @8 [9...16) 	%8 = BinaryOperatorInst '-', %3 : number, %7 : number
// CHECK-NEXT:  $Reg4 @9 [empty]	%9 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg4 @10 [11...12) 	%10 = TryLoadGlobalPropertyInst %0 : object, "print" : string
// CHECK-NEXT:  $Reg4 @11 [empty]	%11 = HBCCallNInst %10, %1 : undefined, %6 : number
// CHECK-NEXT:  $Reg4 @12 [empty]	%12 = CondBranchInst %8 : number, %BB2, %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg4 @13 [14...15) 	%13 = TryLoadGlobalPropertyInst %0 : object, "print" : string
// CHECK-NEXT:  $Reg4 @14 [empty]	%14 = HBCCallNInst %13, %1 : undefined, %6 : number
// CHECK-NEXT:  $Reg0 @15 [empty]	%15 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_with_dependencies_in_loop#0#1(x, y)#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...14) 	%0 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg3 @1 [2...13) 	%1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @2 [3...13) 	%2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @3 [4...5) 	%3 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg1 @4 [5...6) 	%4 = AsNumberInst %3
// CHECK-NEXT:  $Reg4 @5 [6...8) 	%5 = BinaryOperatorInst '*', %4 : number, %4 : number
// CHECK-NEXT:  $Reg1 @6 [7...8) 	%6 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg1 @7 [8...13) 	%7 = BinaryOperatorInst '-', %5 : number, %6 : number
// CHECK-NEXT:  $Reg4 @8 [empty]	%8 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg4 @9 [empty]	%9 = CondBranchInst %0, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @13 [empty]	%10 = ReturnInst %0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg4 @10 [11...12) 	%11 = TryLoadGlobalPropertyInst %1 : object, "print" : string
// CHECK-NEXT:  $Reg4 @11 [empty]	%12 = HBCCallNInst %11, %2 : undefined, %7 : number
// CHECK-NEXT:  $Reg1 @12 [empty]	%13 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function code_sinking#0#1(x, y)#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg1 @1 [2...4) 	%1 = AsInt32Inst %0
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg0 @3 [4...10) 	%3 = BinaryOperatorInst '+', %1 : number, %2 : number
// CHECK-NEXT:  $Reg1 @4 [5...6) 	%4 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg1 @5 [empty]	%5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @9 [empty]	%6 = ReturnInst %3 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg1 @6 [7...8) 	%7 = HBCLoadConstInst 9 : number
// CHECK-NEXT:  $Reg1 @7 [8...9) 	%8 = BinaryOperatorInst '+', %3 : number, %7 : number
// CHECK-NEXT:  $Reg1 @8 [empty]	%9 = ReturnInst %8 : number
// CHECK-NEXT:function_end

// CHECK:function code_sinking_in_loop#0#1(x, y)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 @0 [1...7) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [2...7) 	%1 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg2 @2 [empty]	%2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = CondBranchInst %1, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @8 [empty]	%5 = ReturnInst %4 : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%6 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg2 @5 [empty]	%7 = StorePropertyInst %1, %6 : object, %0
// CHECK-NEXT:  $Reg0 @6 [empty]	%8 = BranchInst %BB1
// CHECK-NEXT:function_end
