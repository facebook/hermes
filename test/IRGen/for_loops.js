/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink(a) { }

function simple_for_loop() {
  for (var i = 0; i < 10; i = i + 1) { sink(i) }
}

function simple_for_loop_break() {
  for (var i = 0; i < 10; i = i + 1) { break; }
}

function simple_for_loop_break_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { break fail; }
}

function simple_for_loop_continue() {
  for (var i = 0; i < 10; i = i + 1) { continue; }
}

function simple_for_loop_continue_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { continue fail; }
}

function for_loop_match(a,b,c,d,e,f) {
  for (a(); b(); c()) { d(); break; e(); }
}

function naked_for_loop() {
  for (;;) { }
}

// Make sure we are not crashing on expressions in the update and init field.
function test_init_update_exprs(param1) {
  for (var i = 0; false ; i++) { }
  for (4        ; false ; --i) { }
  for (param1   ; false ; 2)   { }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [sink, simple_for_loop, simple_for_loop_break, simple_for_loop_break_label, simple_for_loop_continue, simple_for_loop_continue_label, for_loop_match, naked_for_loop, test_init_update_exprs]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simple_for_loop#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simple_for_loop" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %simple_for_loop_break#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "simple_for_loop_break" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %simple_for_loop_break_label#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "simple_for_loop_break_label" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %simple_for_loop_continue#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "simple_for_loop_continue" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %simple_for_loop_continue_label#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "simple_for_loop_continue_label" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %for_loop_match#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "for_loop_match" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %naked_for_loop#0#1()#9, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "naked_for_loop" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %test_init_update_exprs#0#1()#10, %0
// CHECK-NEXT:  %18 = StorePropertyInst %17 : closure, globalObject : object, "test_init_update_exprs" : string
// CHECK-NEXT:  %19 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %20 = StoreStackInst undefined : undefined, %19
// CHECK-NEXT:  %21 = LoadStackInst %19
// CHECK-NEXT:  %22 = ReturnInst %21
// CHECK-NEXT:function_end

// CHECK:function sink#0#1(a)#2
// CHECK-NEXT:frame = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop#0#1()#3
// CHECK-NEXT:frame = [i#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_loop#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#3], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %5 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %6 = CallInst %4, undefined : undefined, %5
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %13 = BinaryOperatorInst '<', %12, 10 : number
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 1 : number
// CHECK-NEXT:  %17 = StoreFrameInst %16, [i#3], %0
// CHECK-NEXT:  %18 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break#0#1()#4
// CHECK-NEXT:frame = [i#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_loop_break#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#4], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '<', %6, 10 : number
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 1 : number
// CHECK-NEXT:  %14 = StoreFrameInst %13, [i#4], %0
// CHECK-NEXT:  %15 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break_label#0#1()#5
// CHECK-NEXT:frame = [i#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_loop_break_label#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#5], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %8 = BinaryOperatorInst '<', %7, 10 : number
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %13, 1 : number
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i#5], %0
// CHECK-NEXT:  %16 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue#0#1()#6
// CHECK-NEXT:frame = [i#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_loop_continue#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#6], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '<', %6, 10 : number
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 1 : number
// CHECK-NEXT:  %14 = StoreFrameInst %13, [i#6], %0
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue_label#0#1()#7
// CHECK-NEXT:frame = [i#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_loop_continue_label#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#7], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#7], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %8 = BinaryOperatorInst '<', %7, 10 : number
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %10 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB3, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %13, 1 : number
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i#7], %0
// CHECK-NEXT:  %16 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function for_loop_match#0#1(a, b, c, d, e, f)#8
// CHECK-NEXT:frame = [a#8, b#8, c#8, d#8, e#8, f#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{for_loop_match#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#8], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#8], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#8], %0
// CHECK-NEXT:  %4 = StoreFrameInst %d, [d#8], %0
// CHECK-NEXT:  %5 = StoreFrameInst %e, [e#8], %0
// CHECK-NEXT:  %6 = StoreFrameInst %f, [f#8], %0
// CHECK-NEXT:  %7 = LoadFrameInst [a#8], %0
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst [d#8], %0
// CHECK-NEXT:  %11 = CallInst %10, undefined : undefined
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst [b#8], %0
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = CondBranchInst %15, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = LoadFrameInst [b#8], %0
// CHECK-NEXT:  %18 = CallInst %17, undefined : undefined
// CHECK-NEXT:  %19 = CondBranchInst %18, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst [c#8], %0
// CHECK-NEXT:  %21 = CallInst %20, undefined : undefined
// CHECK-NEXT:  %22 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = LoadFrameInst [e#8], %0
// CHECK-NEXT:  %24 = CallInst %23, undefined : undefined
// CHECK-NEXT:  %25 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function naked_for_loop#0#1()#9
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{naked_for_loop#0#1()#9}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function test_init_update_exprs#0#1(param1)#10
// CHECK-NEXT:frame = [param1#10, i#10]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_init_update_exprs#0#1()#10}
// CHECK-NEXT:  %1 = StoreFrameInst %param1, [param1#10], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [i#10], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i#10], %0
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BranchInst %BB5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = CondBranchInst false : boolean, %BB2, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %8 = CondBranchInst false : boolean, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#10], %0
// CHECK-NEXT:  %10 = AsNumericInst %9
// CHECK-NEXT:  %11 = UnaryOperatorInst '++', %10 : number|bigint
// CHECK-NEXT:  %12 = StoreFrameInst %11, [i#10], %0
// CHECK-NEXT:  %13 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %15 = LoadFrameInst [param1#10], %0
// CHECK-NEXT:  %16 = BranchInst %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = CondBranchInst false : boolean, %BB7, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %18 = CondBranchInst false : boolean, %BB7, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %19 = LoadFrameInst [i#10], %0
// CHECK-NEXT:  %20 = UnaryOperatorInst '--', %19
// CHECK-NEXT:  %21 = StoreFrameInst %20, [i#10], %0
// CHECK-NEXT:  %22 = BranchInst %BB11
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %23 = BranchInst %BB13
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %25 = CondBranchInst false : boolean, %BB12, %BB14
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %26 = CondBranchInst false : boolean, %BB12, %BB14
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %27 = BranchInst %BB15
// CHECK-NEXT:function_end
