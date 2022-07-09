/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL:function simple_loop(obj)
//CHECK-NEXT:frame = [ret, x, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %2 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %8 = LoadFrameInst [obj]
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = LoadFrameInst [ret]
//CHECK-NEXT:  %13 = ReturnInst %12
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %14 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
//CHECK-NEXT:  %17 = LoadFrameInst [ret]
//CHECK-NEXT:  %18 = LoadFrameInst [obj]
//CHECK-NEXT:  %19 = LoadFrameInst [x]
//CHECK-NEXT:  %20 = LoadPropertyInst %18, %19
//CHECK-NEXT:  %21 = BinaryOperatorInst '+', %17, %20
//CHECK-NEXT:  %22 = StoreFrameInst %21, [ret]
//CHECK-NEXT:  %23 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %24 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function simple_loop(obj) {
  var ret = 0;
  for (var x in obj) {
    ret += obj[x];
  }
  return ret;
}


//CHECK-LABEL:function different_prop(obj)
//CHECK-NEXT:frame = [ret, x, y, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:  %3 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %9 = LoadFrameInst [obj]
//CHECK-NEXT:  %10 = StoreStackInst %9, %6
//CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = LoadFrameInst [ret]
//CHECK-NEXT:  %14 = ReturnInst %13
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %16 = LoadStackInst %11
//CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
//CHECK-NEXT:  %18 = LoadFrameInst [x]
//CHECK-NEXT:  %19 = StoreFrameInst %18, [y]
//CHECK-NEXT:  %20 = LoadFrameInst [ret]
//CHECK-NEXT:  %21 = LoadFrameInst [obj]
//CHECK-NEXT:  %22 = LoadFrameInst [y]
//CHECK-NEXT:  %23 = LoadPropertyInst %21, %22
//CHECK-NEXT:  %24 = BinaryOperatorInst '+', %20, %23
//CHECK-NEXT:  %25 = StoreFrameInst %24, [ret]
//CHECK-NEXT:  %26 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %27 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function different_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    var y = x;
    ret += obj[y];
  }
  return ret;
}

//CHECK-LABEL:function different_obj(obj)
//CHECK-NEXT:frame = [ret, obj1, x, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [obj1]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %3 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %5 = LoadFrameInst [obj]
//CHECK-NEXT:  %6 = StoreFrameInst %5, [obj1]
//CHECK-NEXT:  %7 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %8 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %9 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %10 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %11 = LoadFrameInst [obj]
//CHECK-NEXT:  %12 = StoreStackInst %11, %8
//CHECK-NEXT:  %13 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %14 = GetPNamesInst %7, %8, %9, %10, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %15 = LoadFrameInst [ret]
//CHECK-NEXT:  %16 = ReturnInst %15
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %17 = GetNextPNameInst %13, %8, %9, %10, %7, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %18 = LoadStackInst %13
//CHECK-NEXT:  %19 = StoreFrameInst %18, [x]
//CHECK-NEXT:  %20 = LoadFrameInst [ret]
//CHECK-NEXT:  %21 = LoadFrameInst [obj1]
//CHECK-NEXT:  %22 = LoadFrameInst [x]
//CHECK-NEXT:  %23 = LoadPropertyInst %21, %22
//CHECK-NEXT:  %24 = BinaryOperatorInst '+', %20, %23
//CHECK-NEXT:  %25 = StoreFrameInst %24, [ret]
//CHECK-NEXT:  %26 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %27 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function different_obj(obj) {
  var ret = 0;
  var obj1 = obj;
  for (var x in obj) {
    ret += obj1[x];
  }
  return ret;
}


//CHECK-LABEL:function modify_prop(obj)
//CHECK-NEXT:frame = [ret, x, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %2 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %8 = LoadFrameInst [obj]
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = LoadFrameInst [ret]
//CHECK-NEXT:  %13 = ReturnInst %12
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %14 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
//CHECK-NEXT:  %17 = StoreFrameInst "a" : string, [x]
//CHECK-NEXT:  %18 = LoadFrameInst [ret]
//CHECK-NEXT:  %19 = LoadFrameInst [obj]
//CHECK-NEXT:  %20 = LoadFrameInst [x]
//CHECK-NEXT:  %21 = LoadPropertyInst %19, %20
//CHECK-NEXT:  %22 = BinaryOperatorInst '+', %18, %21
//CHECK-NEXT:  %23 = StoreFrameInst %22, [ret]
//CHECK-NEXT:  %24 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %25 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function modify_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    x = 'a';
    ret += obj[x];
  }
  return ret;
}

//CHECK-LABEL:function modify_value(obj)
//CHECK-NEXT:frame = [ret, x, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %2 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %8 = LoadFrameInst [obj]
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = LoadFrameInst [ret]
//CHECK-NEXT:  %13 = ReturnInst %12
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %14 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
//CHECK-NEXT:  %17 = LoadFrameInst [obj]
//CHECK-NEXT:  %18 = LoadFrameInst [x]
//CHECK-NEXT:  %19 = LoadPropertyInst %17, %18
//CHECK-NEXT:  %20 = AsNumericInst %19
//CHECK-NEXT:  %21 = UnaryOperatorInst '++', %20 : number|bigint
//CHECK-NEXT:  %22 = StorePropertyInst %21, %17, %18
//CHECK-NEXT:  %23 = LoadFrameInst [ret]
//CHECK-NEXT:  %24 = LoadFrameInst [obj]
//CHECK-NEXT:  %25 = LoadFrameInst [x]
//CHECK-NEXT:  %26 = LoadPropertyInst %24, %25
//CHECK-NEXT:  %27 = BinaryOperatorInst '+', %23, %26
//CHECK-NEXT:  %28 = StoreFrameInst %27, [ret]
//CHECK-NEXT:  %29 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %30 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function modify_value(obj) {
  var ret = 0;
  for (var x in obj) {
    obj[x]++;
    ret += obj[x];
  }
  return ret;
}

//CHECK-LABEL:function expression_prop(obj)
//CHECK-NEXT:frame = [ret, x, obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [ret]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %2 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [ret]
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %6 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %7 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %8 = LoadFrameInst [obj]
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %11 = GetPNamesInst %4, %5, %6, %7, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = LoadFrameInst [ret]
//CHECK-NEXT:  %13 = ReturnInst %12
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %14 = GetNextPNameInst %10, %5, %6, %7, %4, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
//CHECK-NEXT:  %17 = LoadFrameInst [ret]
//CHECK-NEXT:  %18 = LoadFrameInst [obj]
//CHECK-NEXT:  %19 = LoadPropertyInst %18, "a" : string
//CHECK-NEXT:  %20 = BinaryOperatorInst '+', %17, %19
//CHECK-NEXT:  %21 = StoreFrameInst %20, [ret]
//CHECK-NEXT:  %22 = BranchInst %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %23 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function expression_prop(obj) {
  var ret = 0;
  for (var x in obj) {
    ret += obj['a'];
  }
  return ret;
}
