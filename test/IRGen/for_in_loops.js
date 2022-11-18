/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink(a) { }

function simple_for_in_loop(obj) {
  var prop = 0;
  for (prop in obj) { sink(prop) }
}

function for_in_loop_with_break_continue(obj) {
  var prop = 0;
  for (prop in obj) { sink(prop); break; continue; }
}

function for_in_loop_with_named_break(obj) {
  var prop = 0;
goto1:
  for (prop in obj) { sink(prop); break goto1; }
}

function check_var_decl_for_in_loop(obj) {
  for (var prop in obj) { sink(prop) }
}

function loop_member_expr_lhs() {
  var x = {};

  for (x.y in [1,2,3]) { sink(x.y); }

  sink(x.y);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [sink, simple_for_in_loop, for_in_loop_with_break_continue, for_in_loop_with_named_break, check_var_decl_for_in_loop, loop_member_expr_lhs]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %simple_for_in_loop()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "simple_for_in_loop" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %for_in_loop_with_break_continue()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "for_in_loop_with_break_continue" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %for_in_loop_with_named_break()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "for_in_loop_with_named_break" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %check_var_decl_for_in_loop()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "check_var_decl_for_in_loop" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %loop_member_expr_lhs()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "loop_member_expr_lhs" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function sink(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_in_loop(obj)
// CHECK-NEXT:frame = [prop, obj]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [prop]
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [prop]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = LoadFrameInst [obj]
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst %9
// CHECK-NEXT:  %14 = StoreFrameInst %13, [prop]
// CHECK-NEXT:  %15 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %16 = LoadFrameInst [prop]
// CHECK-NEXT:  %17 = CallInst %15, undefined : undefined, %16
// CHECK-NEXT:  %18 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function for_in_loop_with_break_continue(obj)
// CHECK-NEXT:frame = [prop, obj]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [prop]
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [prop]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = LoadFrameInst [obj]
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst %9
// CHECK-NEXT:  %14 = StoreFrameInst %13, [prop]
// CHECK-NEXT:  %15 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %16 = LoadFrameInst [prop]
// CHECK-NEXT:  %17 = CallInst %15, undefined : undefined, %16
// CHECK-NEXT:  %18 = BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function for_in_loop_with_named_break(obj)
// CHECK-NEXT:frame = [prop, obj]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [prop]
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [prop]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = LoadFrameInst [obj]
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst %9
// CHECK-NEXT:  %15 = StoreFrameInst %14, [prop]
// CHECK-NEXT:  %16 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %17 = LoadFrameInst [prop]
// CHECK-NEXT:  %18 = CallInst %16, undefined : undefined, %17
// CHECK-NEXT:  %19 = BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function check_var_decl_for_in_loop(obj)
// CHECK-NEXT:frame = [prop, obj]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [prop]
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj]
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %3 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %4 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %5 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %6 = LoadFrameInst [obj]
// CHECK-NEXT:  %7 = StoreStackInst %6, %3
// CHECK-NEXT:  %8 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %9 = GetPNamesInst %2, %3, %4, %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = GetNextPNameInst %8, %3, %4, %5, %2, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadStackInst %8
// CHECK-NEXT:  %13 = StoreFrameInst %12, [prop]
// CHECK-NEXT:  %14 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %15 = LoadFrameInst [prop]
// CHECK-NEXT:  %16 = CallInst %14, undefined : undefined, %15
// CHECK-NEXT:  %17 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function loop_member_expr_lhs()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %2 = StoreFrameInst %1 : object, [x]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = AllocArrayInst 3 : number, 1 : number, 2 : number, 3 : number
// CHECK-NEXT:  %8 = StoreStackInst %7 : object, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %12 = LoadFrameInst [x]
// CHECK-NEXT:  %13 = LoadPropertyInst %12, "y" : string
// CHECK-NEXT:  %14 = CallInst %11, undefined : undefined, %13
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst %9
// CHECK-NEXT:  %18 = LoadFrameInst [x]
// CHECK-NEXT:  %19 = StorePropertyLooseInst %17, %18, "y" : string
// CHECK-NEXT:  %20 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %21 = LoadFrameInst [x]
// CHECK-NEXT:  %22 = LoadPropertyInst %21, "y" : string
// CHECK-NEXT:  %23 = CallInst %20, undefined : undefined, %22
// CHECK-NEXT:  %24 = BranchInst %BB2
// CHECK-NEXT:function_end
