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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simple_for_loop" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "simple_for_loop_break" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simple_for_loop_break_label" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "simple_for_loop_continue" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "simple_for_loop_continue_label" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "for_loop_match" : string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "naked_for_loop" : string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "test_init_update_exprs" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %sink()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %simple_for_loop()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "simple_for_loop" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %simple_for_loop_break()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "simple_for_loop_break" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %simple_for_loop_break_label()
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15 : closure, globalObject : object, "simple_for_loop_break_label" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %simple_for_loop_continue()
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17 : closure, globalObject : object, "simple_for_loop_continue" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %simple_for_loop_continue_label()
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19 : closure, globalObject : object, "simple_for_loop_continue_label" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %for_loop_match()
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21 : closure, globalObject : object, "for_loop_match" : string
// CHECK-NEXT:  %23 = CreateFunctionInst %naked_for_loop()
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23 : closure, globalObject : object, "naked_for_loop" : string
// CHECK-NEXT:  %25 = CreateFunctionInst %test_init_update_exprs()
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25 : closure, globalObject : object, "test_init_update_exprs" : string
// CHECK-NEXT:  %27 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %28 = StoreStackInst undefined : undefined, %27
// CHECK-NEXT:  %29 = LoadStackInst %27
// CHECK-NEXT:  %30 = ReturnInst %29
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
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %6 = LoadFrameInst [i]
// CHECK-NEXT:  %7 = CallInst %5, undefined : undefined, %6
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst [i]
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst [i]
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %13, 1 : number
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i]
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadFrameInst [i]
// CHECK-NEXT:  %8 = BinaryOperatorInst '<', %7, 10 : number
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst [i]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %10, 1 : number
// CHECK-NEXT:  %12 = StoreFrameInst %11, [i]
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break_label()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB1, %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadFrameInst [i]
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 1 : number
// CHECK-NEXT:  %13 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst [i]
// CHECK-NEXT:  %8 = BinaryOperatorInst '<', %7, 10 : number
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = LoadFrameInst [i]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %10, 1 : number
// CHECK-NEXT:  %12 = StoreFrameInst %11, [i]
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue_label()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadFrameInst [i]
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %11, 1 : number
// CHECK-NEXT:  %13 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB4
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
// CHECK-NEXT:  %14 = LoadFrameInst [b]
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = CondBranchInst %15, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadFrameInst [d]
// CHECK-NEXT:  %18 = CallInst %17, undefined : undefined
// CHECK-NEXT:  %19 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = LoadFrameInst [b]
// CHECK-NEXT:  %22 = CallInst %21, undefined : undefined
// CHECK-NEXT:  %23 = CondBranchInst %22, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = LoadFrameInst [c]
// CHECK-NEXT:  %25 = CallInst %24, undefined : undefined
// CHECK-NEXT:  %26 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %27 = LoadFrameInst [e]
// CHECK-NEXT:  %28 = CallInst %27, undefined : undefined
// CHECK-NEXT:  %29 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function naked_for_loop()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test_init_update_exprs(param1)
// CHECK-NEXT:frame = [param1, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %param1
// CHECK-NEXT:  %1 = StoreFrameInst %0, [param1]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %4 = CondBranchInst false : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = CondBranchInst false : boolean, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %7 = CondBranchInst false : boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = AsNumericInst %8
// CHECK-NEXT:  %10 = UnaryOperatorInst '++', %9 : number|bigint
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i]
// CHECK-NEXT:  %12 = BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = LoadFrameInst [param1]
// CHECK-NEXT:  %15 = CondBranchInst false : boolean, %BB8, %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %16 = CondBranchInst false : boolean, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = LoadFrameInst [i]
// CHECK-NEXT:  %18 = UnaryOperatorInst '--', %17
// CHECK-NEXT:  %19 = StoreFrameInst %18, [i]
// CHECK-NEXT:  %20 = BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %21 = BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %23 = CondBranchInst false : boolean, %BB8, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %24 = BranchInst %BB12
// CHECK-NEXT:function_end
