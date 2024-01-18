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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "simple_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "different_prop": string
// CHECK-NEXT:       DeclareGlobalVarInst "different_obj": string
// CHECK-NEXT:       DeclareGlobalVarInst "modify_prop": string
// CHECK-NEXT:       DeclareGlobalVarInst "modify_value": string
// CHECK-NEXT:       DeclareGlobalVarInst "expression_prop": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %simple_loop(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "simple_loop": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %different_prop(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "different_prop": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %different_obj(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "different_obj": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %modify_prop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "modify_prop": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %modify_value(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "modify_value": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %expression_prop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "expression_prop": string
// CHECK-NEXT:  %18 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %18: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:function_end

// CHECK:function simple_loop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %19: any, %20: any
// CHECK-NEXT:  %22 = BinaryAddInst (:any) %18: any, %21: any
// CHECK-NEXT:        StoreFrameInst %22: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function different_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %6: any, %7: any, %8: number, %9: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %12: any, %7: any, %8: number, %9: number, %6: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %17: any, [x]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        StoreFrameInst %19: any, [y]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %22: any, %23: any
// CHECK-NEXT:  %25 = BinaryAddInst (:any) %21: any, %24: any
// CHECK-NEXT:        StoreFrameInst %25: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function different_obj(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, obj1: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [obj1]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:       StoreFrameInst %6: any, [obj1]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %11 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %12: any, %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %8: any, %9: any, %10: number, %11: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %14: any, %9: any, %10: number, %11: number, %8: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StoreFrameInst %19: any, [x]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [obj1]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %22: any, %23: any
// CHECK-NEXT:  %25 = BinaryAddInst (:any) %21: any, %24: any
// CHECK-NEXT:        StoreFrameInst %25: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function modify_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:        StoreFrameInst "a": string, [x]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %20: any, %21: any
// CHECK-NEXT:  %23 = BinaryAddInst (:any) %19: any, %22: any
// CHECK-NEXT:        StoreFrameInst %23: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function modify_value(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %18: any, %19: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:        StorePropertyLooseInst %22: number|bigint, %18: any, %19: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %27 = LoadPropertyInst (:any) %25: any, %26: any
// CHECK-NEXT:  %28 = BinaryAddInst (:any) %24: any, %27: any
// CHECK-NEXT:        StoreFrameInst %28: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function expression_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [ret]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [ret]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "a": string
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %18: any, %20: any
// CHECK-NEXT:        StoreFrameInst %21: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
