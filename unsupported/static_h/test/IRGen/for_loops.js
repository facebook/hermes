/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink(a) { }

//CHECK-LABEL:function simple_for_loop()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = LoadPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:    %4 = LoadFrameInst [i]
//CHECK-NEXT:    %5 = CallInst %3, undefined : undefined, %4
//CHECK-NEXT:    %6 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %7 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '<', %8, 10 : number
//CHECK-NEXT:    %10 = CondBranchInst %9, %BB2, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '<', %11, 10 : number
//CHECK-NEXT:    %13 = CondBranchInst %12, %BB2, %BB4
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %14 = LoadFrameInst [i]
//CHECK-NEXT:    %15 = BinaryOperatorInst '+', %14, 1 : number
//CHECK-NEXT:    %16 = StoreFrameInst %15, [i]
//CHECK-NEXT:    %17 = BranchInst %BB5
//CHECK-NEXT:function_end
function simple_for_loop() {
  for (var i = 0; i < 10; i = i + 1) { sink(i) }
}

//CHECK-LABEL:function simple_for_loop_break()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = BranchInst %BB3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %4 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %5 = LoadFrameInst [i]
//CHECK-NEXT:    %6 = BinaryOperatorInst '<', %5, 10 : number
//CHECK-NEXT:    %7 = CondBranchInst %6, %BB2, %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '<', %8, 10 : number
//CHECK-NEXT:    %10 = CondBranchInst %9, %BB2, %BB3
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 1 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = BranchInst %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %15 = BranchInst %BB5
//CHECK-NEXT:function_end
function simple_for_loop_break() {
  for (var i = 0; i < 10; i = i + 1) { break; }
}

//CHECK-LABEL:function simple_for_loop_break_label()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %4 = BranchInst %BB4
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %5 = BranchInst %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '<', %6, 10 : number
//CHECK-NEXT:    %8 = CondBranchInst %7, %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %9 = LoadFrameInst [i]
//CHECK-NEXT:    %10 = BinaryOperatorInst '<', %9, 10 : number
//CHECK-NEXT:    %11 = CondBranchInst %10, %BB3, %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %12 = LoadFrameInst [i]
//CHECK-NEXT:    %13 = BinaryOperatorInst '+', %12, 1 : number
//CHECK-NEXT:    %14 = StoreFrameInst %13, [i]
//CHECK-NEXT:    %15 = BranchInst %BB5
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %16 = BranchInst %BB6
//CHECK-NEXT:function_end
function simple_for_loop_break_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { break fail; }
}


//CHECK-LABEL:function simple_for_loop_continue()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %4 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %5 = LoadFrameInst [i]
//CHECK-NEXT:    %6 = BinaryOperatorInst '<', %5, 10 : number
//CHECK-NEXT:    %7 = CondBranchInst %6, %BB2, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '<', %8, 10 : number
//CHECK-NEXT:    %10 = CondBranchInst %9, %BB2, %BB4
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 1 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = BranchInst %BB5
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %15 = BranchInst %BB3
//CHECK-NEXT:function_end
function simple_for_loop_continue() {
  for (var i = 0; i < 10; i = i + 1) { continue; }
}

//CHECK-LABEL:function simple_for_loop_continue_label()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %4 = BranchInst %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %5 = BranchInst %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '<', %6, 10 : number
//CHECK-NEXT:    %8 = CondBranchInst %7, %BB3, %BB5
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %9 = LoadFrameInst [i]
//CHECK-NEXT:    %10 = BinaryOperatorInst '<', %9, 10 : number
//CHECK-NEXT:    %11 = CondBranchInst %10, %BB3, %BB5
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %12 = LoadFrameInst [i]
//CHECK-NEXT:    %13 = BinaryOperatorInst '+', %12, 1 : number
//CHECK-NEXT:    %14 = StoreFrameInst %13, [i]
//CHECK-NEXT:    %15 = BranchInst %BB6
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %16 = BranchInst %BB4
//CHECK-NEXT:function_end
function simple_for_loop_continue_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { continue fail; }
}

//CHECK-LABEL:function for_loop_match(a, b, c, d, e, f)
//CHECK-NEXT:frame = [a, b, c, d, e, f]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %a, [a]
//CHECK-NEXT:    %1 = StoreFrameInst %b, [b]
//CHECK-NEXT:    %2 = StoreFrameInst %c, [c]
//CHECK-NEXT:    %3 = StoreFrameInst %d, [d]
//CHECK-NEXT:    %4 = StoreFrameInst %e, [e]
//CHECK-NEXT:    %5 = StoreFrameInst %f, [f]
//CHECK-NEXT:    %6 = LoadFrameInst [a]
//CHECK-NEXT:    %7 = CallInst %6, undefined : undefined
//CHECK-NEXT:    %8 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %9 = LoadFrameInst [d]
//CHECK-NEXT:    %10 = CallInst %9, undefined : undefined
//CHECK-NEXT:    %11 = BranchInst %BB3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %12 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %13 = LoadFrameInst [b]
//CHECK-NEXT:    %14 = CallInst %13, undefined : undefined
//CHECK-NEXT:    %15 = CondBranchInst %14, %BB2, %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %16 = LoadFrameInst [b]
//CHECK-NEXT:    %17 = CallInst %16, undefined : undefined
//CHECK-NEXT:    %18 = CondBranchInst %17, %BB2, %BB3
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %19 = LoadFrameInst [c]
//CHECK-NEXT:    %20 = CallInst %19, undefined : undefined
//CHECK-NEXT:    %21 = BranchInst %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %22 = LoadFrameInst [e]
//CHECK-NEXT:    %23 = CallInst %22, undefined : undefined
//CHECK-NEXT:    %24 = BranchInst %BB5
//CHECK-NEXT:function_end
function for_loop_match(a,b,c,d,e,f) {
  for (a(); b(); c()) { d(); break; e(); }
}

//CHECK-LABEL:function naked_for_loop()
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %1 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %2 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = BranchInst %BB2
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %4 = BranchInst %BB2
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %5 = BranchInst %BB5
//CHECK-NEXT:function_end
function naked_for_loop() {
  for (;;) { }
}

// Make sure we are not crashing on expressions in the update and init field.
//CHECK-LABEL:function test_init_update_exprs(param1)
//CHECK-NEXT:frame = [i, param1]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst %param1, [param1]
//CHECK-NEXT:    %2 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %3 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %4 = BranchInst %BB3
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %5 = BranchInst %BB5
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = CondBranchInst false : boolean, %BB2, %BB4
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %7 = CondBranchInst false : boolean, %BB2, %BB4
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = AsNumericInst %8
//CHECK-NEXT:    %10 = UnaryOperatorInst '++', %9 : number|bigint
//CHECK-NEXT:    %11 = StoreFrameInst %10, [i]
//CHECK-NEXT:    %12 = BranchInst %BB6
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %13 = BranchInst %BB8
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %14 = LoadFrameInst [param1]
//CHECK-NEXT:    %15 = BranchInst %BB10
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %16 = CondBranchInst false : boolean, %BB7, %BB9
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %17 = CondBranchInst false : boolean, %BB7, %BB9
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %18 = LoadFrameInst [i]
//CHECK-NEXT:    %19 = UnaryOperatorInst '--', %18
//CHECK-NEXT:    %20 = StoreFrameInst %19, [i]
//CHECK-NEXT:    %21 = BranchInst %BB11
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %22 = BranchInst %BB13
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %23 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %24 = CondBranchInst false : boolean, %BB12, %BB14
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %25 = CondBranchInst false : boolean, %BB12, %BB14
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %26 = BranchInst %BB15
//CHECK-NEXT:function_end
function test_init_update_exprs(param1) {
  for (var i = 0; false ; i++) { }
  for (4        ; false ; --i) { }
  for (param1   ; false ; 2)   { }
}
