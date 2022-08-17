/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O -fno-inline | %FileCheck %s --match-full-lines


//CHECK-LABEL:function store_x_not_captured() : number
//CHECK-NEXT:frame = [y : number]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 3 : number, [y] : number
//CHECK-NEXT:  %1 = CreateFunctionInst %foo() : undefined
//CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
//CHECK-NEXT:  %3 = ReturnInst 9 : number
//CHECK-NEXT:function_end
function store_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  x = 9;
  return x;
}

//CHECK-LABEL:function store_x_is_captured() : number
//CHECK-NEXT:frame = [y : number, x : number]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 3 : number, [y] : number
//CHECK-NEXT:  %1 = StoreFrameInst 4 : number, [x] : number
//CHECK-NEXT:  %2 = CreateFunctionInst %"foo 1#"() : undefined
//CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
//CHECK-NEXT:  %4 = StoreFrameInst 9 : number, [x] : number
//CHECK-NEXT:  %5 = ReturnInst 9 : number
//CHECK-NEXT:function_end
function store_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = x; }
  k();
  x = 9;
  return x;
}

//CHECK-LABEL:function load_x_not_captured() : number
//CHECK-NEXT:frame = [y : number]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 3 : number, [y] : number
//CHECK-NEXT:  %1 = CreateFunctionInst %"foo 2#"() : undefined
//CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
//CHECK-NEXT:  %3 = ReturnInst 4 : number
//CHECK-NEXT:function_end
function load_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  return x;
}

//CHECK-LABEL:function load_x_is_captured() : number
//CHECK-NEXT:frame = [x : number]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 4 : number, [x] : number
//CHECK-NEXT:  %1 = CreateFunctionInst %"foo 3#"() : undefined
//CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
//CHECK-NEXT:  %3 = LoadFrameInst [x] : number
//CHECK-NEXT:  %4 = ReturnInst %3 : number
//CHECK-NEXT:function_end
function load_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { x = 12; }
  k();
  return x;
}

//CHECK-LABEL:function load_x_captured_as_load() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %"foo 4#"() : undefined
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %2 = ReturnInst 4 : number
//CHECK-NEXT:function_end
function load_x_captured_as_load() {
  var y = 3;
  var x = 4;
  var k = function foo() {  print(x); }
  k();
  return x;
}

function abort() {
  throw 42;
}

//CHECK-LABEL:function "foo 5#"(x)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $a
//CHECK-NEXT:  %1 = StoreStackInst %x, %0
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = LoadStackInst %0
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 100 : number
//CHECK-NEXT:  %6 = StoreStackInst %5 : string|number, %0
//CHECK-NEXT:  %7 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadStackInst %0
//CHECK-NEXT:  %9 = ReturnInst %8
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %10 = LoadStackInst %0
//CHECK-NEXT:  %11 = BinaryOperatorInst '+', %10, 1 : number
//CHECK-NEXT:  %12 = StoreStackInst %11 : string|number, %0
//CHECK-NEXT:  %13 = LoadPropertyInst globalObject : object, "abort" : string
//CHECK-NEXT:  %14 = CallInst %13, undefined : undefined
//CHECK-NEXT:  %15 = BinaryOperatorInst '+', %11 : string|number, 2 : number
//CHECK-NEXT:  %16 = StoreStackInst %15 : string|number, %0
//CHECK-NEXT:  %17 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %18 = TryEndInst
//CHECK-NEXT:  %19 = BranchInst %BB3
//CHECK-NEXT:function_end
function foo(x) {
  var a = x;
    try {
      a += 1;
      abort();
      a += 2;
    } catch (e) {
      a += 100;
    }
    return a;
}


//CHECK-LABEL:function postponed_store_in_use_block(x) : undefined
//CHECK-NEXT: frame = [x]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:   %1 = CreateFunctionInst %""()
//CHECK-NEXT:   %2 = BinaryOperatorInst '+', 0 : number, %1 : closure
//CHECK-NEXT:   %3 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function postponed_store_in_use_block(x) {
  switch (0) {
    // This caused a crash because StackPromotion inserted an invalid load.
    case 0: 0 + function() { return x; }
  }
}
