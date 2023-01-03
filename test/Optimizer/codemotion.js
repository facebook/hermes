/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [hoist_branch, hoist_branch_window, no_hoist_inc_dec, hoist_loop, hoist_loop_expression, hoist_from_multiblock_loop, hoist_with_dependencies_in_loop, code_sinking, code_sinking_in_loop]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %hoist_branch()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "hoist_branch" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %hoist_branch_window()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "hoist_branch_window" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %no_hoist_inc_dec() : number|bigint
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "no_hoist_inc_dec" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %hoist_loop() : undefined
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "hoist_loop" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %hoist_loop_expression()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "hoist_loop_expression" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %hoist_from_multiblock_loop()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "hoist_from_multiblock_loop" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %hoist_with_dependencies_in_loop()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "hoist_with_dependencies_in_loop" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %code_sinking() : number
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "code_sinking" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %code_sinking_in_loop() : undefined
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "code_sinking_in_loop" : string
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_branch(x, y)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadPropertyInst %1, "z" : string
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "k" : string
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "r" : string
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadPropertyInst %1, "z" : string
// CHECK-NEXT:  %8 = LoadPropertyInst %7, "k" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "t" : string
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function hoist_branch_window(x, y)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = AsInt32Inst %1
// CHECK-NEXT:  %3 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2 : number, %2 : number
// CHECK-NEXT:  %5 = LoadPropertyInst %1, "z" : string
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "k" : string
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "r" : string
// CHECK-NEXT:  %8 = BinaryOperatorInst '+', %7, %4 : number
// CHECK-NEXT:  %9 = ReturnInst %8 : string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadPropertyInst %1, "z" : string
// CHECK-NEXT:  %11 = LoadPropertyInst %10, "k" : string
// CHECK-NEXT:  %12 = LoadPropertyInst %11, "t" : string
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function no_hoist_inc_dec(x, y) : number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %1
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = UnaryOperatorInst '--', %1
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst %3 : number|bigint, %BB1, %5 : number|bigint, %BB2
// CHECK-NEXT:  %8 = ReturnInst %7 : number|bigint
// CHECK-NEXT:function_end

// CHECK:function hoist_loop(x) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = BinaryOperatorInst '<', 0 : number, %0
// CHECK-NEXT:  %2 = CondBranchInst %1 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst 0 : number, %BB0, %6 : number, %BB1
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined, %3 : number
// CHECK-NEXT:  %6 = UnaryOperatorInst '++', %3 : number
// CHECK-NEXT:  %7 = BinaryOperatorInst '<', %6 : number, %0
// CHECK-NEXT:  %8 = CondBranchInst %7 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_loop_expression(a, b, call)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = LoadParamInst %b
// CHECK-NEXT:  %2 = LoadParamInst %call
// CHECK-NEXT:  %3 = AsNumberInst %0
// CHECK-NEXT:  %4 = AsNumberInst %1
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = UnaryOperatorInst '-', %3 : number
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %4 : number, 7 : number
// CHECK-NEXT:  %8 = BinaryOperatorInst '*', %6 : number, %7 : number
// CHECK-NEXT:  %9 = CallInst %2, undefined : undefined, %8 : number
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_from_multiblock_loop(x)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = AsNumberInst %0
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %4 = BinaryOperatorInst '*', 3 : number, %1 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '*', %4 : number, %1 : number
// CHECK-NEXT:  %6 = CallInst %3, undefined : undefined, %5 : number
// CHECK-NEXT:  %7 = BinaryOperatorInst '-', %1 : number, 1 : number
// CHECK-NEXT:  %8 = CondBranchInst %7 : number, %BB2, %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined, %5 : number
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_with_dependencies_in_loop(x, y)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = AsNumberInst %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BinaryOperatorInst '*', %2 : number, %2 : number
// CHECK-NEXT:  %5 = CondBranchInst %1, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst %1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %8 = BinaryOperatorInst '-', %4 : number, 3 : number
// CHECK-NEXT:  %9 = CallInst %7, undefined : undefined, %8 : number
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function code_sinking(x, y) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = AsInt32Inst %1
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %2 : number, 2 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %3 : number, 9 : number
// CHECK-NEXT:  %5 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %3 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = ReturnInst %4 : number
// CHECK-NEXT:function_end

// CHECK:function code_sinking_in_loop(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %4 = CondBranchInst %1, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = StorePropertyLooseInst %1, %3 : object, %0
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:function_end
