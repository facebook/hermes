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
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_break": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_break_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_continue": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_continue_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_loop_match": string
// CHECK-NEXT:       DeclareGlobalVarInst "naked_for_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_init_update_exprs": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %sink(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "sink": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %simple_for_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "simple_for_loop": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %simple_for_loop_break(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "simple_for_loop_break": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %simple_for_loop_break_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "simple_for_loop_break_label": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %simple_for_loop_continue(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "simple_for_loop_continue": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %simple_for_loop_continue_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "simple_for_loop_continue_label": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %for_loop_match(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "for_loop_match": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %naked_for_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "naked_for_loop": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %test_init_update_exprs(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %25: object, globalObject: object, "test_init_update_exprs": string
// CHECK-NEXT:  %27 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %27: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %27: any
// CHECK-NEXT:        ReturnInst %29: any
// CHECK-NEXT:function_end

// CHECK:function sink(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %7 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: any
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) %13: any, 1: number
// CHECK-NEXT:        StoreFrameInst %14: any, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_break_label(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %8 = BinaryLessThanInst (:boolean) %7: any, 10: number
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 1: number
// CHECK-NEXT:        StoreFrameInst %11: any, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function simple_for_loop_continue_label(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryLessThanInst (:boolean) %8: any, 10: number
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %12 = BinaryAddInst (:any) %11: any, 1: number
// CHECK-NEXT:        StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function for_loop_match(a: any, b: any, c: any, d: any, e: any, f: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any, d: any, e: any, f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %d: any
// CHECK-NEXT:       StoreFrameInst %6: any, [d]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %e: any
// CHECK-NEXT:       StoreFrameInst %8: any, [e]: any
// CHECK-NEXT:  %10 = LoadParamInst (:any) %f: any
// CHECK-NEXT:        StoreFrameInst %10: any, [f]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        CondBranchInst %15: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [d]: any
// CHECK-NEXT:  %18 = CallInst (:any) %17: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function naked_for_loop(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test_init_update_exprs(param1: any): any
// CHECK-NEXT:frame = [param1: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %param1: any
// CHECK-NEXT:       StoreFrameInst %0: any, [param1]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       CondBranchInst false: boolean, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:       CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = AsNumericInst (:number|bigint) %8: any
// CHECK-NEXT:  %10 = UnaryIncInst (:number|bigint) %9: number|bigint
// CHECK-NEXT:        StoreFrameInst %10: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [param1]: any
// CHECK-NEXT:        CondBranchInst false: boolean, %BB8, %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        CondBranchInst false: boolean, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %18 = UnaryDecInst (:number|bigint) %17: any
// CHECK-NEXT:        StoreFrameInst %18: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        CondBranchInst false: boolean, %BB8, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:function_end
