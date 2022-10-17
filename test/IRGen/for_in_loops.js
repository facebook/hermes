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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [sink, simple_for_in_loop, for_in_loop_with_break_continue, for_in_loop_with_named_break, check_var_decl_for_in_loop, loop_member_expr_lhs]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simple_for_in_loop#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simple_for_in_loop" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %for_in_loop_with_break_continue#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "for_in_loop_with_break_continue" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %for_in_loop_with_named_break#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "for_in_loop_with_named_break" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %check_var_decl_for_in_loop#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "check_var_decl_for_in_loop" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %loop_member_expr_lhs#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "loop_member_expr_lhs" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function sink#0#1(a)#2
// CHECK-NEXT:frame = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_for_in_loop#0#1(obj)#3
// CHECK-NEXT:frame = [obj#3, prop#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_for_in_loop#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [prop#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [prop#3], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %8 = LoadFrameInst [obj#3], %0
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadStackInst %10
// CHECK-NEXT:  %15 = StoreFrameInst %14, [prop#3], %0
// CHECK-NEXT:  %16 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %17 = LoadFrameInst [prop#3], %0
// CHECK-NEXT:  %18 = CallInst %16, undefined : undefined, %17
// CHECK-NEXT:  %19 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function for_in_loop_with_break_continue#0#1(obj)#4
// CHECK-NEXT:frame = [obj#4, prop#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{for_in_loop_with_break_continue#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [prop#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [prop#4], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %8 = LoadFrameInst [obj#4], %0
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadStackInst %10
// CHECK-NEXT:  %15 = StoreFrameInst %14, [prop#4], %0
// CHECK-NEXT:  %16 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %17 = LoadFrameInst [prop#4], %0
// CHECK-NEXT:  %18 = CallInst %16, undefined : undefined, %17
// CHECK-NEXT:  %19 = BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function for_in_loop_with_named_break#0#1(obj)#5
// CHECK-NEXT:frame = [obj#5, prop#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{for_in_loop_with_named_break#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [prop#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [prop#5], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %8 = LoadFrameInst [obj#5], %0
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadStackInst %10
// CHECK-NEXT:  %16 = StoreFrameInst %15, [prop#5], %0
// CHECK-NEXT:  %17 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %18 = LoadFrameInst [prop#5], %0
// CHECK-NEXT:  %19 = CallInst %17, undefined : undefined, %18
// CHECK-NEXT:  %20 = BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function check_var_decl_for_in_loop#0#1(obj)#6
// CHECK-NEXT:frame = [obj#6, prop#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check_var_decl_for_in_loop#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [prop#6], %0
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = LoadFrameInst [obj#6], %0
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst %9
// CHECK-NEXT:  %14 = StoreFrameInst %13, [prop#6], %0
// CHECK-NEXT:  %15 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %16 = LoadFrameInst [prop#6], %0
// CHECK-NEXT:  %17 = CallInst %15, undefined : undefined, %16
// CHECK-NEXT:  %18 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function loop_member_expr_lhs#0#1()#7
// CHECK-NEXT:frame = [x#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{loop_member_expr_lhs#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#7], %0
// CHECK-NEXT:  %2 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %3 = StoreFrameInst %2 : object, [x#7], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %8 = AllocArrayInst 3 : number, 1 : number, 2 : number, 3 : number
// CHECK-NEXT:  %9 = StoreStackInst %8 : object, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %13 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %14 = LoadPropertyInst %13, "y" : string
// CHECK-NEXT:  %15 = CallInst %12, undefined : undefined, %14
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst %10
// CHECK-NEXT:  %19 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %20 = StorePropertyInst %18, %19, "y" : string
// CHECK-NEXT:  %21 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %22 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %23 = LoadPropertyInst %22, "y" : string
// CHECK-NEXT:  %24 = CallInst %21, undefined : undefined, %23
// CHECK-NEXT:  %25 = BranchInst %BB2
// CHECK-NEXT:function_end
