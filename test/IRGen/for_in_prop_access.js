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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "different_prop": string
// CHECK-NEXT:       DeclareGlobalVarInst "different_obj": string
// CHECK-NEXT:       DeclareGlobalVarInst "modify_prop": string
// CHECK-NEXT:       DeclareGlobalVarInst "modify_value": string
// CHECK-NEXT:       DeclareGlobalVarInst "expression_prop": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %simple_loop(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "simple_loop": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %different_prop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "different_prop": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %different_obj(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "different_obj": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %modify_prop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "modify_prop": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %modify_value(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "modify_value": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %expression_prop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "expression_prop": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function simple_loop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_loop(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %7: any, %8: any, %9: number, %10: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %13: any, %8: any, %9: number, %10: number, %7: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [x]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) %21: any, %22: any
// CHECK-NEXT:  %24 = BinaryAddInst (:any) %20: any, %23: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function different_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %different_prop(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %11 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %12: any, %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %8: any, %9: any, %10: number, %11: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %14: any, %9: any, %10: number, %11: number, %8: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: any, [x]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [y]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %26 = LoadPropertyInst (:any) %24: any, %25: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %23: any, %26: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %27: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function different_obj(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, obj1: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %different_obj(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [obj1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [obj1]: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %12 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %13 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %14: any, %11: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %10: any, %11: any, %12: number, %13: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %16: any, %11: any, %12: number, %13: number, %10: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [x]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [obj1]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %26 = LoadPropertyInst (:any) %24: any, %25: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %23: any, %26: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %27: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function modify_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %modify_prop(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %7: any, %8: any, %9: number, %10: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %13: any, %8: any, %9: number, %10: number, %7: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [x]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, "a": string, [x]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %22: any, %23: any
// CHECK-NEXT:  %25 = BinaryAddInst (:any) %21: any, %24: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function modify_value(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %modify_value(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %7: any, %8: any, %9: number, %10: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %13: any, %8: any, %9: number, %10: number, %7: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [x]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %20: any, %21: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StorePropertyLooseInst %24: number|bigint, %20: any, %21: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %29 = LoadPropertyInst (:any) %27: any, %28: any
// CHECK-NEXT:  %30 = BinaryAddInst (:any) %26: any, %29: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function expression_prop(obj: any): any
// CHECK-NEXT:frame = [obj: any, ret: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %expression_prop(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ret]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [ret]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %10 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %7: any, %8: any, %9: number, %10: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %13: any, %8: any, %9: number, %10: number, %7: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [x]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [ret]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %21: any, "a": string
// CHECK-NEXT:  %23 = BinaryAddInst (:any) %20: any, %22: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: any, [ret]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
