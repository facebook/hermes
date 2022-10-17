/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s --match-full-lines

function store_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  x = 9;
  return x;
}

function store_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = x; }
  k();
  x = 9;
  return x;
}

function load_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  return x;
}

function load_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { x = 12; }
  k();
  return x;
}

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

function postponed_store_in_use_block(x) {
  switch (0) {
    // This caused a crash because StackPromotion inserted an invalid load.
    case 0: 0 + function() { return x; }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [store_x_not_captured, store_x_is_captured, load_x_not_captured, load_x_is_captured, load_x_captured_as_load, abort, foo, postponed_store_in_use_block]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %store_x_not_captured#0#1()#2 : number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "store_x_not_captured" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %store_x_is_captured#0#1()#4 : number, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "store_x_is_captured" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %load_x_not_captured#0#1()#6 : number, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "load_x_not_captured" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %load_x_is_captured#0#1()#8 : number, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "load_x_is_captured" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %load_x_captured_as_load#0#1()#10 : number, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "load_x_captured_as_load" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %abort#0#1()#12 : undefined, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "abort" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %"foo 5#"#0#1()#13, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %postponed_store_in_use_block#0#1()#14 : undefined, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "postponed_store_in_use_block" : string
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_not_captured#0#1()#2 : number
// CHECK-NEXT:frame = [y#2 : number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{store_x_not_captured#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst 3 : number, [y#2] : number, %0
// CHECK-NEXT:  %2 = CreateFunctionInst %foo#1#2()#3 : undefined, %0
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst 9 : number
// CHECK-NEXT:function_end

// CHECK:function foo#1#2()#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#1#2()#3}
// CHECK-NEXT:  %1 = StoreFrameInst 12 : number, [y#2@store_x_not_captured] : number, %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_is_captured#0#1()#4 : number
// CHECK-NEXT:frame = [y#4 : number, x#4 : number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{store_x_is_captured#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst 3 : number, [y#4] : number, %0
// CHECK-NEXT:  %2 = StoreFrameInst 4 : number, [x#4] : number, %0
// CHECK-NEXT:  %3 = CreateFunctionInst %"foo 1#"#1#4()#5 : undefined, %0
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined
// CHECK-NEXT:  %5 = StoreFrameInst 9 : number, [x#4] : number, %0
// CHECK-NEXT:  %6 = ReturnInst 9 : number
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"#1#4()#5 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"foo 1#"#1#4()#5}
// CHECK-NEXT:  %1 = LoadFrameInst [x#4@store_x_is_captured] : number, %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : number, [y#4@store_x_is_captured] : number, %0
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function load_x_not_captured#0#1()#6 : number
// CHECK-NEXT:frame = [y#6 : number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{load_x_not_captured#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst 3 : number, [y#6] : number, %0
// CHECK-NEXT:  %2 = CreateFunctionInst %"foo 2#"#1#6()#7 : undefined, %0
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst 4 : number
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"#1#6()#7 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"foo 2#"#1#6()#7}
// CHECK-NEXT:  %1 = StoreFrameInst 12 : number, [y#6@load_x_not_captured] : number, %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function load_x_is_captured#0#1()#8 : number
// CHECK-NEXT:frame = [x#8 : number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{load_x_is_captured#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst 4 : number, [x#8] : number, %0
// CHECK-NEXT:  %2 = CreateFunctionInst %"foo 3#"#1#8()#9 : undefined, %0
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = LoadFrameInst [x#8] : number, %0
// CHECK-NEXT:  %5 = ReturnInst %4 : number
// CHECK-NEXT:function_end

// CHECK:function "foo 3#"#1#8()#9 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"foo 3#"#1#8()#9}
// CHECK-NEXT:  %1 = StoreFrameInst 12 : number, [x#8@load_x_is_captured] : number, %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function load_x_captured_as_load#0#1()#10 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{load_x_captured_as_load#0#1()#10}
// CHECK-NEXT:  %1 = CreateFunctionInst %"foo 4#"#1#10()#11 : undefined, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst 4 : number
// CHECK-NEXT:function_end

// CHECK:function "foo 4#"#1#10()#11 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"foo 4#"#1#10()#11}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function abort#0#1()#12 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{abort#0#1()#12}
// CHECK-NEXT:  %1 = ThrowInst 42 : number
// CHECK-NEXT:function_end

// CHECK:function "foo 5#"#0#1(x)#13
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $a
// CHECK-NEXT:  %1 = CreateScopeInst %S{"foo 5#"#0#1()#13}
// CHECK-NEXT:  %2 = StoreStackInst %x, %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadStackInst %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 100 : number
// CHECK-NEXT:  %7 = StoreStackInst %6 : string|number, %0
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadStackInst %0
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst %0
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 1 : number
// CHECK-NEXT:  %13 = StoreStackInst %12 : string|number, %0
// CHECK-NEXT:  %14 = LoadPropertyInst globalObject : object, "abort" : string
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %12 : string|number, 2 : number
// CHECK-NEXT:  %17 = StoreStackInst %16 : string|number, %0
// CHECK-NEXT:  %18 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = TryEndInst
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function postponed_store_in_use_block#0#1(x)#14 : undefined
// CHECK-NEXT:frame = [x#14]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{postponed_store_in_use_block#0#1()#14}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#14], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %""#1#14()#15, %0
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', 0 : number, %2 : closure
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""#1#14()#15
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#1#14()#15}
// CHECK-NEXT:  %1 = LoadFrameInst [x#14@postponed_store_in_use_block], %0
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end
