/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple_loop" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "different_prop" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "different_obj" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "modify_prop" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "modify_value" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "expression_prop" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %simple_loop()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "simple_loop" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %different_prop()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "different_prop" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %different_obj()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "different_obj" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %modify_prop()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "modify_prop" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %modify_value()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "modify_value" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %expression_prop()
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "expression_prop" : string
// CHECK-NEXT:  %18 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %18
// CHECK-NEXT:  %20 = LoadStackInst %18
// CHECK-NEXT:  %21 = ReturnInst %20
// CHECK-NEXT:function_end

// CHECK:function simple_loop(obj)
// CHECK-NEXT:frame = [obj, ret, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj]
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = LoadFrameInst [ret]
// CHECK-NEXT:  %19 = LoadFrameInst [obj]
// CHECK-NEXT:  %20 = LoadFrameInst [x]
// CHECK-NEXT:  %21 = LoadPropertyInst %19, %20
// CHECK-NEXT:  %22 = BinaryOperatorInst '+', %18, %21
// CHECK-NEXT:  %23 = StoreFrameInst %22, [ret]
// CHECK-NEXT:  %24 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %25 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function different_prop(obj)
// CHECK-NEXT:frame = [obj, ret, x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:  %5 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %8 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %9 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %10 = LoadFrameInst [obj]
// CHECK-NEXT:  %11 = StoreStackInst %10, %7
// CHECK-NEXT:  %12 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %13 = GetPNamesInst %6, %7, %8, %9, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst [ret]
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = GetNextPNameInst %12, %7, %8, %9, %6, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst %12
// CHECK-NEXT:  %18 = StoreFrameInst %17, [x]
// CHECK-NEXT:  %19 = LoadFrameInst [x]
// CHECK-NEXT:  %20 = StoreFrameInst %19, [y]
// CHECK-NEXT:  %21 = LoadFrameInst [ret]
// CHECK-NEXT:  %22 = LoadFrameInst [obj]
// CHECK-NEXT:  %23 = LoadFrameInst [y]
// CHECK-NEXT:  %24 = LoadPropertyInst %22, %23
// CHECK-NEXT:  %25 = BinaryOperatorInst '+', %21, %24
// CHECK-NEXT:  %26 = StoreFrameInst %25, [ret]
// CHECK-NEXT:  %27 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function different_obj(obj)
// CHECK-NEXT:frame = [obj, ret, obj1, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [obj1]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %5 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %6 = LoadFrameInst [obj]
// CHECK-NEXT:  %7 = StoreFrameInst %6, [obj1]
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %9 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %10 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %11 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %12 = LoadFrameInst [obj]
// CHECK-NEXT:  %13 = StoreStackInst %12, %9
// CHECK-NEXT:  %14 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %15 = GetPNamesInst %8, %9, %10, %11, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst [ret]
// CHECK-NEXT:  %17 = ReturnInst %16
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %18 = GetNextPNameInst %14, %9, %10, %11, %8, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst %14
// CHECK-NEXT:  %20 = StoreFrameInst %19, [x]
// CHECK-NEXT:  %21 = LoadFrameInst [ret]
// CHECK-NEXT:  %22 = LoadFrameInst [obj1]
// CHECK-NEXT:  %23 = LoadFrameInst [x]
// CHECK-NEXT:  %24 = LoadPropertyInst %22, %23
// CHECK-NEXT:  %25 = BinaryOperatorInst '+', %21, %24
// CHECK-NEXT:  %26 = StoreFrameInst %25, [ret]
// CHECK-NEXT:  %27 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modify_prop(obj)
// CHECK-NEXT:frame = [obj, ret, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj]
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = StoreFrameInst "a" : string, [x]
// CHECK-NEXT:  %19 = LoadFrameInst [ret]
// CHECK-NEXT:  %20 = LoadFrameInst [obj]
// CHECK-NEXT:  %21 = LoadFrameInst [x]
// CHECK-NEXT:  %22 = LoadPropertyInst %20, %21
// CHECK-NEXT:  %23 = BinaryOperatorInst '+', %19, %22
// CHECK-NEXT:  %24 = StoreFrameInst %23, [ret]
// CHECK-NEXT:  %25 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modify_value(obj)
// CHECK-NEXT:frame = [obj, ret, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj]
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = LoadFrameInst [obj]
// CHECK-NEXT:  %19 = LoadFrameInst [x]
// CHECK-NEXT:  %20 = LoadPropertyInst %18, %19
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StorePropertyLooseInst %22, %18, %19
// CHECK-NEXT:  %24 = LoadFrameInst [ret]
// CHECK-NEXT:  %25 = LoadFrameInst [obj]
// CHECK-NEXT:  %26 = LoadFrameInst [x]
// CHECK-NEXT:  %27 = LoadPropertyInst %25, %26
// CHECK-NEXT:  %28 = BinaryOperatorInst '+', %24, %27
// CHECK-NEXT:  %29 = StoreFrameInst %28, [ret]
// CHECK-NEXT:  %30 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %31 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function expression_prop(obj)
// CHECK-NEXT:frame = [obj, ret, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [ret]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [ret]
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %9 = LoadFrameInst [obj]
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %12 = GetPNamesInst %5, %6, %7, %8, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst [ret]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = GetNextPNameInst %11, %6, %7, %8, %5, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = LoadFrameInst [ret]
// CHECK-NEXT:  %19 = LoadFrameInst [obj]
// CHECK-NEXT:  %20 = LoadPropertyInst %19, "a" : string
// CHECK-NEXT:  %21 = BinaryOperatorInst '+', %18, %20
// CHECK-NEXT:  %22 = StoreFrameInst %21, [ret]
// CHECK-NEXT:  %23 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
