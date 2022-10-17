/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function simple_loop(obj) {
  var ret = 0;
  for (var x in obj) {
    ret += obj[x];
  }
  return ret;
}

function different_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    var y = x;
    ret += obj[y];
  }
  return ret;
}

function different_obj(obj) {
  var ret = 0;
  var obj1 = obj;
  for (var x in obj) {
    ret += obj1[x];
  }
  return ret;
}

function modify_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    x = 'a';
    ret += obj[x];
  }
  return ret;
}

function modify_value(obj) {
  var ret = 0;
  for (var x in obj) {
    obj[x]++;
    ret += obj[x];
  }
  return ret;
}

function expression_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    ret += obj['a'];
  }
  return ret;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simple_loop, different_prop, different_obj, modify_prop, modify_value, expression_prop]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple_loop#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple_loop" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %different_prop#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "different_prop" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %different_obj#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "different_obj" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %modify_prop#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "modify_prop" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %modify_value#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "modify_value" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %expression_prop#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "expression_prop" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = LoadStackInst %13
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function simple_loop#0#1(obj)#2
// CHECK-NEXT:frame = [obj#2, ret#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_loop#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret#2], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj#2], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret#2], %0
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x#2], %0
// CHECK-NEXT:  %18 = LoadFrameInst [ret#2], %0
// CHECK-NEXT:  %19 = LoadFrameInst [obj#2], %0
// CHECK-NEXT:  %20 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %21 = LoadPropertyInst %19, %20
// CHECK-NEXT:  %22 = BinaryOperatorInst '+', %18, %21
// CHECK-NEXT:  %23 = StoreFrameInst %22, [ret#2], %0
// CHECK-NEXT:  %24 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %25 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function different_prop#0#1(obj)#3
// CHECK-NEXT:frame = [obj#3, ret#3, x#3, y#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{different_prop#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [y#3], %0
// CHECK-NEXT:  %5 = StoreFrameInst 0 : number, [ret#3], %0
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %8 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %9 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %10 = LoadFrameInst [obj#3], %0
// CHECK-NEXT:  %11 = StoreStackInst %10, %7
// CHECK-NEXT:  %12 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %13 = GetPNamesInst %6, %7, %8, %9, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst [ret#3], %0
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = GetNextPNameInst %12, %7, %8, %9, %6, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst %12
// CHECK-NEXT:  %18 = StoreFrameInst %17, [x#3], %0
// CHECK-NEXT:  %19 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %20 = StoreFrameInst %19, [y#3], %0
// CHECK-NEXT:  %21 = LoadFrameInst [ret#3], %0
// CHECK-NEXT:  %22 = LoadFrameInst [obj#3], %0
// CHECK-NEXT:  %23 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %24 = LoadPropertyInst %22, %23
// CHECK-NEXT:  %25 = BinaryOperatorInst '+', %21, %24
// CHECK-NEXT:  %26 = StoreFrameInst %25, [ret#3], %0
// CHECK-NEXT:  %27 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function different_obj#0#1(obj)#4
// CHECK-NEXT:frame = [obj#4, ret#4, obj1#4, x#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{different_obj#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [obj1#4], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [x#4], %0
// CHECK-NEXT:  %5 = StoreFrameInst 0 : number, [ret#4], %0
// CHECK-NEXT:  %6 = LoadFrameInst [obj#4], %0
// CHECK-NEXT:  %7 = StoreFrameInst %6, [obj1#4], %0
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %9 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %10 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %11 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %12 = LoadFrameInst [obj#4], %0
// CHECK-NEXT:  %13 = StoreStackInst %12, %9
// CHECK-NEXT:  %14 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %15 = GetPNamesInst %8, %9, %10, %11, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst [ret#4], %0
// CHECK-NEXT:  %17 = ReturnInst %16
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %18 = GetNextPNameInst %14, %9, %10, %11, %8, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst %14
// CHECK-NEXT:  %20 = StoreFrameInst %19, [x#4], %0
// CHECK-NEXT:  %21 = LoadFrameInst [ret#4], %0
// CHECK-NEXT:  %22 = LoadFrameInst [obj1#4], %0
// CHECK-NEXT:  %23 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %24 = LoadPropertyInst %22, %23
// CHECK-NEXT:  %25 = BinaryOperatorInst '+', %21, %24
// CHECK-NEXT:  %26 = StoreFrameInst %25, [ret#4], %0
// CHECK-NEXT:  %27 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modify_prop#0#1(obj)#5
// CHECK-NEXT:frame = [obj#5, ret#5, x#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{modify_prop#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#5], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret#5], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj#5], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret#5], %0
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x#5], %0
// CHECK-NEXT:  %18 = StoreFrameInst "a" : string, [x#5], %0
// CHECK-NEXT:  %19 = LoadFrameInst [ret#5], %0
// CHECK-NEXT:  %20 = LoadFrameInst [obj#5], %0
// CHECK-NEXT:  %21 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %22 = LoadPropertyInst %20, %21
// CHECK-NEXT:  %23 = BinaryOperatorInst '+', %19, %22
// CHECK-NEXT:  %24 = StoreFrameInst %23, [ret#5], %0
// CHECK-NEXT:  %25 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modify_value#0#1(obj)#6
// CHECK-NEXT:frame = [obj#6, ret#6, x#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{modify_value#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#6], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#6], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret#6], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj#6], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret#6], %0
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x#6], %0
// CHECK-NEXT:  %18 = LoadFrameInst [obj#6], %0
// CHECK-NEXT:  %19 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %20 = LoadPropertyInst %18, %19
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StorePropertyInst %22, %18, %19
// CHECK-NEXT:  %24 = LoadFrameInst [ret#6], %0
// CHECK-NEXT:  %25 = LoadFrameInst [obj#6], %0
// CHECK-NEXT:  %26 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %27 = LoadPropertyInst %25, %26
// CHECK-NEXT:  %28 = BinaryOperatorInst '+', %24, %27
// CHECK-NEXT:  %29 = StoreFrameInst %28, [ret#6], %0
// CHECK-NEXT:  %30 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %31 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function expression_prop#0#1(obj)#7
// CHECK-NEXT:frame = [obj#7, ret#7, x#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{expression_prop#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#7], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret#7], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x#7], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret#7], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj#7], %0
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret#7], %0
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x#7], %0
// CHECK-NEXT:  %18 = LoadFrameInst [ret#7], %0
// CHECK-NEXT:  %19 = LoadFrameInst [obj#7], %0
// CHECK-NEXT:  %20 = LoadPropertyInst %19, "a" : string
// CHECK-NEXT:  %21 = BinaryOperatorInst '+', %18, %20
// CHECK-NEXT:  %22 = StoreFrameInst %21, [ret#7], %0
// CHECK-NEXT:  %23 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
