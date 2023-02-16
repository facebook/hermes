/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simple_for_loop": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "simple_for_loop_break": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simple_for_loop_break_label": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "simple_for_loop_continue": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "simple_for_loop_continue_label": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "for_loop_match": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "naked_for_loop": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "test_init_update_exprs": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %sink(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "sink": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %simple_for_loop(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "simple_for_loop": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:closure) %simple_for_loop_break(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: closure, globalObject: object, "simple_for_loop_break": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:closure) %simple_for_loop_break_label(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: closure, globalObject: object, "simple_for_loop_break_label": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:closure) %simple_for_loop_continue(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: closure, globalObject: object, "simple_for_loop_continue": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:closure) %simple_for_loop_continue_label(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: closure, globalObject: object, "simple_for_loop_continue_label": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:closure) %for_loop_match(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: closure, globalObject: object, "for_loop_match": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:closure) %naked_for_loop(): any
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: closure, globalObject: object, "naked_for_loop": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:closure) %test_init_update_exprs(): any
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: closure, globalObject: object, "test_init_update_exprs": string
// CHECK-NEXT:  %27 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %28 = StoreStackInst undefined: undefined, %27: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %27: any
// CHECK-NEXT:  %30 = ReturnInst (:any) %29: any
// CHECK-NEXT:function_end

// CHECK:function sink(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %7 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, %6: any
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:any) %10: any, 10: number
// CHECK-NEXT:  %12 = CondBranchInst %11: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) %13: any, 1: number
// CHECK-NEXT:  %15 = StoreFrameInst %14: any, [i]: any
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %8 = BinaryLessThanInst (:any) %7: any, 10: number
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 1: number
// CHECK-NEXT:  %12 = StoreFrameInst %11: any, [i]: any
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break_label(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryLessThanInst (:any) %8: any, 10: number
// CHECK-NEXT:  %10 = CondBranchInst %9: any, %BB1, %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %12 = BinaryAddInst (:any) %11: any, 1: number
// CHECK-NEXT:  %13 = StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:  %14 = BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %8 = BinaryLessThanInst (:any) %7: any, 10: number
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 1: number
// CHECK-NEXT:  %12 = StoreFrameInst %11: any, [i]: any
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue_label(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryLessThanInst (:any) %8: any, 10: number
// CHECK-NEXT:  %10 = CondBranchInst %9: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %12 = BinaryAddInst (:any) %11: any, 1: number
// CHECK-NEXT:  %13 = StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %15 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function for_loop_match(a: any, b: any, c: any, d: any, e: any, f: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any, d: any, e: any, f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %d: any
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [d]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %9 = StoreFrameInst %8: any, [e]: any
// CHECK-NEXT:  %10 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  %11 = StoreFrameInst %10: any, [f]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %16 = CondBranchInst %15: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [d]: any
// CHECK-NEXT:  %18 = CallInst (:any) %17: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %19 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %22 = CallInst (:any) %21: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %23 = CondBranchInst %22: any, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:  %25 = CallInst (:any) %24: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %26 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [e]: any
// CHECK-NEXT:  %28 = CallInst (:any) %27: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %29 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function naked_for_loop(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %2 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function test_init_update_exprs(param1: any): any
// CHECK-NEXT:frame = [param1: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %param1: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [param1]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %3 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %4 = CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = CondBranchInst false: boolean, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %7 = CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = AsNumericInst (:number|bigint) %8: any
// CHECK-NEXT:  %10 = UnaryIncInst (:any) %9: number|bigint
// CHECK-NEXT:  %11 = StoreFrameInst %10: any, [i]: any
// CHECK-NEXT:  %12 = BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [param1]: any
// CHECK-NEXT:  %15 = CondBranchInst false: boolean, %BB8, %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %16 = CondBranchInst false: boolean, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %18 = UnaryDecInst (:any) %17: any
// CHECK-NEXT:  %19 = StoreFrameInst %18: any, [i]: any
// CHECK-NEXT:  %20 = BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %21 = BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %22 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %23 = CondBranchInst false: boolean, %BB8, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %24 = BranchInst %BB12
// CHECK-NEXT:function_end
