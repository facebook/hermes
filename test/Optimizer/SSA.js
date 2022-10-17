/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

function simple(x, y) {
  var t = 9;
  if (x) { t = 19; }
  return t;
}

function control_flow(x, y) {
  var t = 9;

  if (x) {
    t = 19;
  } else {
    t = 12;
    if (y) {
      t = 15;
    } else {
      t = 4;
    }
  }
  t = t + 1;
  if (x - 1) {
    t = y;
  } else {
    if (y) { t = 15; }
  }

  return t;
}

function control_catch(x, y) {
  var t = 9;

  if (x) {
    try {
      if (j) {
        throw 3;
      }
      t = 19;
    } catch (e) {
      return t;
    }
  }

  return t;
}

function multi(x, y) {
  var t0 = 9;
  var t1 = 9;
  if (x) { t0 = 19; t1 = t0; }
  return t0 + t1;
}

// Make sure that we are not promoting allocas inside try-catch sections.
function badThrow() {
  var result = -1;
  try {
      result = 100;
      throw "hello";
  } catch (e) {}
  return result;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [simple, control_flow, control_catch, multi, badThrow]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple#0#1()#2 : number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %control_flow#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "control_flow" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %control_catch#0#1()#4 : number, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "control_catch" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %multi#0#1()#5 : number, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "multi" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %badThrow#0#1()#6 : number, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "badThrow" : string
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple#0#1(x, y)#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple#0#1()#2}
// CHECK-NEXT:  %1 = CondBranchInst %x, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = PhiInst 19 : number, %BB1, 9 : number, %BB0
// CHECK-NEXT:  %4 = ReturnInst %3 : number
// CHECK-NEXT:function_end

// CHECK:function control_flow#0#1(x, y)#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{control_flow#0#1()#3}
// CHECK-NEXT:  %1 = CondBranchInst %x, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = CondBranchInst %y, %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst 4 : number, %BB3, 19 : number, %BB0, 15 : number, %BB2
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %3 : number, 1 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '-', %x, 1 : number
// CHECK-NEXT:  %6 = CondBranchInst %5 : number, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = CondBranchInst %y, %BB6, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = PhiInst 15 : number, %BB6, %4 : number, %BB5, %y, %BB1
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %11 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function control_catch#0#1(x, y)#4 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $t
// CHECK-NEXT:  %1 = CreateScopeInst %S{control_catch#0#1()#4}
// CHECK-NEXT:  %2 = StoreStackInst 9 : number, %0 : number
// CHECK-NEXT:  %3 = CondBranchInst %x, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadStackInst %0 : number
// CHECK-NEXT:  %6 = ReturnInst %5 : number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = CatchInst
// CHECK-NEXT:  %8 = LoadStackInst %0 : number
// CHECK-NEXT:  %9 = ReturnInst %8 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "j" : string
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = ThrowInst 3 : number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = StoreStackInst 19 : number, %0 : number
// CHECK-NEXT:  %14 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %15 = TryEndInst
// CHECK-NEXT:  %16 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function multi#0#1(x, y)#5 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{multi#0#1()#5}
// CHECK-NEXT:  %1 = CondBranchInst %x, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = PhiInst 19 : number, %BB1, 9 : number, %BB0
// CHECK-NEXT:  %4 = PhiInst 19 : number, %BB1, 9 : number, %BB0
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3 : number, %4 : number
// CHECK-NEXT:  %6 = ReturnInst %5 : number
// CHECK-NEXT:function_end

// CHECK:function badThrow#0#1()#6 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $result
// CHECK-NEXT:  %1 = CreateScopeInst %S{badThrow#0#1()#6}
// CHECK-NEXT:  %2 = StoreStackInst -1 : number, %0 : number
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadStackInst %0 : number
// CHECK-NEXT:  %6 = ReturnInst %5 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = StoreStackInst 100 : number, %0 : number
// CHECK-NEXT:  %8 = ThrowInst "hello" : string
// CHECK-NEXT:function_end
