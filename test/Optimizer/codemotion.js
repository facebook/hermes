/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra %s -O | %FileCheck --match-full-lines %s

//CHECK-LABEL:function hoist_branch(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %1 = LoadPropertyInst %0, "z" : string
//CHECK-NEXT:  {{.*}}  %2 = LoadPropertyInst %1, "k" : string
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %5 = LoadPropertyInst %2, "r" : string
//CHECK-NEXT:  {{.*}}  %6 = ReturnInst %5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %7 = LoadPropertyInst %2, "t" : string
//CHECK-NEXT:  {{.*}}  %8 = ReturnInst %7
//CHECK-NEXT:function_end
function hoist_branch(x, y) {
  if (x) {
    return y.z.k.r;
  } else {
    return y.z.k.t;
  }
}

//CHECK-LABEL:function hoist_branch_window(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %1 = AsInt32Inst %0
//CHECK-NEXT:  {{.*}}  %2 = LoadPropertyInst %0, "z" : string
//CHECK-NEXT:  {{.*}}  %3 = LoadPropertyInst %2, "k" : string
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %5 = CondBranchInst %4, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %6 = LoadPropertyInst %3, "r" : string
//CHECK-NEXT:  {{.*}}  %7 = BinaryOperatorInst '+', %1 : number, %1 : number
//CHECK-NEXT:  {{.*}}  %8 = BinaryOperatorInst '+', %6, %7 : number
//CHECK-NEXT:  {{.*}}  %9 = ReturnInst %8 : string|number
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %10 = LoadPropertyInst %3, "t" : string
//CHECK-NEXT:  {{.*}}  %11 = ReturnInst %10
//CHECK-NEXT:function_end
function hoist_branch_window(x, y) {
  var n = y | 0;

  if (x) {
    var z = n + n;
    return y.z.k.r + z;
  } else {
    return y.z.k.t;
  }
}

//CHECK-LABEL:function no_hoist_inc_dec(x, y) : number|bigint
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %2 = CondBranchInst %1, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %3 = UnaryOperatorInst '++', %0
//CHECK-NEXT:  {{.*}}  %4 = MovInst %3 : number|bigint
//CHECK-NEXT:  {{.*}}  %5 = BranchInst %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %6 = UnaryOperatorInst '--', %0
//CHECK-NEXT:  {{.*}}  %7 = MovInst %6 : number|bigint
//CHECK-NEXT:  {{.*}}  %8 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}}  %9 = PhiInst %4 : number|bigint, %BB1, %7 : number|bigint, %BB2
//CHECK-NEXT:  {{.*}}  %10 = MovInst %9 : number|bigint
//CHECK-NEXT:  {{.*}}  %11 = ReturnInst %10 : number|bigint
//CHECK-NEXT:function_end
function no_hoist_inc_dec(x, y) {
  if (x) {
    y++;
  } else {
    y--;
  }
  return y;
}

//CHECK-LABEL:function hoist_loop(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %2 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %4 = MovInst %1 : number
//CHECK-NEXT:  {{.*}}  %5 = CompareBranchInst '<', %4 : number, %0, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %6 = PhiInst %4 : number, %BB0, %10 : number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %7 = TryLoadGlobalPropertyInst %2 : object, "print" : string
//CHECK-NEXT:  {{.*}}  %8 = HBCCallNInst %7, %3 : undefined, %6 : number|bigint
//CHECK-NEXT:  {{.*}}  %9 = UnaryOperatorInst '++', %6 : number|bigint
//CHECK-NEXT:  {{.*}}  %10 = MovInst %9 : number|bigint
//CHECK-NEXT:  {{.*}}  %11 = CompareBranchInst '<', %10 : number|bigint, %0, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %12 = ReturnInst %3 : undefined
//CHECK-NEXT:function_end
function hoist_loop(x) {
  for (var i = 0; i < x; i++) {
    print(i);
  }
}

//CHECK-LABEL:function hoist_loop_expression(a, b, call) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 3 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %3 = AsNumberInst %2
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %5 = AsNumberInst %4
//CHECK-NEXT:  {{.*}}  %6 = UnaryOperatorInst '-', %3 : number
//CHECK-NEXT:  {{.*}}  %7 = HBCLoadConstInst 7 : number
//CHECK-NEXT:  {{.*}}  %8 = BinaryOperatorInst '+', %5 : number, %7 : number
//CHECK-NEXT:  {{.*}}  %9 = BinaryOperatorInst '*', %6 : number, %8 : number
//CHECK-NEXT:  {{.*}}  %10 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %11 = HBCCallNInst %0, %1 : undefined, %9 : number
//CHECK-NEXT:  {{.*}}  %12 = BranchInst %BB1
//CHECK-NEXT:function_end
function hoist_loop_expression(a, b, call) {
  a = +a;
  b = +b;
  for (;;) {
    call(-a * (b + 7));
  }
}

//CHECK-LABEL:function hoist_from_multiblock_loop(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %3 = AsNumberInst %2
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %5 = BinaryOperatorInst '*', %4 : number, %3 : number
//CHECK-NEXT:  {{.*}}  %6 = BinaryOperatorInst '*', %5 : number, %3 : number
//CHECK-NEXT:  {{.*}}  %7 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}}  %8 = BinaryOperatorInst '-', %3 : number, %7 : number
//CHECK-NEXT:  {{.*}}  %9 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %10 = TryLoadGlobalPropertyInst %0 : object, "print" : string
//CHECK-NEXT:  {{.*}}  %11 = HBCCallNInst %10, %1 : undefined, %6 : number
//CHECK-NEXT:  {{.*}}  %12 = CondBranchInst %8 : number, %BB2, %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %13 = TryLoadGlobalPropertyInst %0 : object, "print" : string
//CHECK-NEXT:  {{.*}}  %14 = HBCCallNInst %13, %1 : undefined, %6 : number
//CHECK-NEXT:  {{.*}}  %15 = BranchInst %BB1
//CHECK-NEXT:function_end

function hoist_from_multiblock_loop(x) {
  x = +x;
  for (;;) {
    print(3 * x * x);
    if (x - 1) {
      print(3 * x * x);
    }
  }
}

//CHECK-LABEL:function hoist_with_dependencies_in_loop(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %4 = AsNumberInst %3
//CHECK-NEXT:  {{.*}}  %5 = BinaryOperatorInst '*', %4 : number, %4 : number
//CHECK-NEXT:  {{.*}}  %6 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %7 = BinaryOperatorInst '-', %5 : number, %6 : number
//CHECK-NEXT:  {{.*}}  %8 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %9 = CondBranchInst %0, %BB2, %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %10 = ReturnInst %0
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}}  %11 = TryLoadGlobalPropertyInst %1 : object, "print" : string
//CHECK-NEXT:  {{.*}}  %12 = HBCCallNInst %11, %2 : undefined, %7 : number
//CHECK-NEXT:  {{.*}}  %13 = BranchInst %BB1
//CHECK-NEXT:function_end

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

//CHECK-LABEL:function code_sinking(x, y) : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %1 = AsInt32Inst %0
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}}  %3 = BinaryOperatorInst '+', %1 : number, %2 : number
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %5 = CondBranchInst %4, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %6 = ReturnInst %3 : number
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %7 = HBCLoadConstInst 9 : number
//CHECK-NEXT:  {{.*}}  %8 = BinaryOperatorInst '+', %3 : number, %7 : number
//CHECK-NEXT:  {{.*}}  %9 = ReturnInst %8 : number
//CHECK-NEXT:function_end
function code_sinking(x, y) {
  var n = (y | 0) + 2;
  var k = (n) + 9;

  if (x) {
    return n;
  } else {
    return k;
  }
}

//CHECK-LABEL:function code_sinking_in_loop(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %2 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %3 = CondBranchInst %1, %BB2, %BB3
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %5 = ReturnInst %4 : undefined
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}}  %6 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %7 = StorePropertyInst %1, %6 : object, %0
//CHECK-NEXT:  {{.*}}  %8 = BranchInst %BB1
//CHECK-NEXT:function_end
function code_sinking_in_loop(x, y) {
  for (;;) {
    var obj = {};
    if (y) {
      return;
    }
    obj[x] = y;
  }
}
