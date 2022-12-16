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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [sink, simple_for_loop, simple_for_loop_break, simple_for_loop_break_label, simple_for_loop_continue, simple_for_loop_continue_label, for_loop_match, naked_for_loop, test_init_update_exprs]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %simple_for_loop()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "simple_for_loop" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %simple_for_loop_break()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "simple_for_loop_break" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %simple_for_loop_break_label()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "simple_for_loop_break_label" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %simple_for_loop_continue()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "simple_for_loop_continue" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %simple_for_loop_continue_label()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "simple_for_loop_continue_label" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %for_loop_match()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "for_loop_match" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %naked_for_loop()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "naked_for_loop" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %test_init_update_exprs()
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "test_init_update_exprs" : string
// CHECK-NEXT:  %18 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %18
// CHECK-NEXT:  %20 = LoadStackInst %18
// CHECK-NEXT:  %21 = ReturnInst %20
// CHECK-NEXT:function_end

// CHECK:function sink(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = CallInst %3, undefined : undefined, %4
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadFrameInst [i]
// CHECK-NEXT:  %12 = BinaryOperatorInst '<', %11, 10 : number
// CHECK-NEXT:  %13 = CondBranchInst %12, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst [i]
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', %14, 1 : number
// CHECK-NEXT:  %16 = StoreFrameInst %15, [i]
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [i]
// CHECK-NEXT:  %6 = BinaryOperatorInst '<', %5, 10 : number
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadFrameInst [i]
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 1 : number
// CHECK-NEXT:  %13 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break_label()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [i]
// CHECK-NEXT:  %7 = BinaryOperatorInst '<', %6, 10 : number
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = LoadFrameInst [i]
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = LoadFrameInst [i]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 1 : number
// CHECK-NEXT:  %14 = StoreFrameInst %13, [i]
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %16 = BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst [i]
// CHECK-NEXT:  %6 = BinaryOperatorInst '<', %5, 10 : number
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB2, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB2, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst [i]
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 1 : number
// CHECK-NEXT:  %13 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue_label()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [i]
// CHECK-NEXT:  %7 = BinaryOperatorInst '<', %6, 10 : number
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %9 = LoadFrameInst [i]
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB3, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = LoadFrameInst [i]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %12, 1 : number
// CHECK-NEXT:  %14 = StoreFrameInst %13, [i]
// CHECK-NEXT:  %15 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function for_loop_match(a, b, c, d, e, f)
// CHECK-NEXT:frame = [a, b, c, d, e, f]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadParamInst %c
// CHECK-NEXT:  %5 = StoreFrameInst %4, [c]
// CHECK-NEXT:  %6 = LoadParamInst %d
// CHECK-NEXT:  %7 = StoreFrameInst %6, [d]
// CHECK-NEXT:  %8 = LoadParamInst %e
// CHECK-NEXT:  %9 = StoreFrameInst %8, [e]
// CHECK-NEXT:  %10 = LoadParamInst %f
// CHECK-NEXT:  %11 = StoreFrameInst %10, [f]
// CHECK-NEXT:  %12 = LoadFrameInst [a]
// CHECK-NEXT:  %13 = CallInst %12, undefined : undefined
// CHECK-NEXT:  %14 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = LoadFrameInst [d]
// CHECK-NEXT:  %16 = CallInst %15, undefined : undefined
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %19 = LoadFrameInst [b]
// CHECK-NEXT:  %20 = CallInst %19, undefined : undefined
// CHECK-NEXT:  %21 = CondBranchInst %20, %BB2, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadFrameInst [b]
// CHECK-NEXT:  %23 = CallInst %22, undefined : undefined
// CHECK-NEXT:  %24 = CondBranchInst %23, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = LoadFrameInst [c]
// CHECK-NEXT:  %26 = CallInst %25, undefined : undefined
// CHECK-NEXT:  %27 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = LoadFrameInst [e]
// CHECK-NEXT:  %29 = CallInst %28, undefined : undefined
// CHECK-NEXT:  %30 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function naked_for_loop()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %1 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function test_init_update_exprs(param1)
// CHECK-NEXT:frame = [i, param1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = LoadParamInst %param1
// CHECK-NEXT:  %2 = StoreFrameInst %1, [param1]
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i]
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
// CHECK-NEXT:  %9 = LoadFrameInst [i]
// CHECK-NEXT:  %10 = AsNumericInst %9
// CHECK-NEXT:  %11 = UnaryOperatorInst '++', %10 : number|bigint
// CHECK-NEXT:  %12 = StoreFrameInst %11, [i]
// CHECK-NEXT:  %13 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %15 = LoadFrameInst [param1]
// CHECK-NEXT:  %16 = BranchInst %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = CondBranchInst false : boolean, %BB7, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %18 = CondBranchInst false : boolean, %BB7, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %19 = LoadFrameInst [i]
// CHECK-NEXT:  %20 = UnaryOperatorInst '--', %19
// CHECK-NEXT:  %21 = StoreFrameInst %20, [i]
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
