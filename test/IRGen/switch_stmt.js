/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function days_of_the_week(day, x) {
  switch (day) {
  default:
    day = "?";
  case 0:
    day = "Sunday";
    break;
  case 1:
    day = "Monday";
    break;
  case x:
    day = "Tuesday";
    break;
  case 3:
    day = "Wednesday";
    break;
  case 4:
    day = "Thursday";
    break;
  case 5:
    day = "Friday";
    break;
  case 6:
    day = "Saturday";
  }
  return day;
}

function simple_xor(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  }
  return "invalid";
}

function simple_xor2(b) {
  switch (b) {
  case 1: return 0;
  case 0: return 1;
  default: return "invalid"
  }
}

function simple_test0(b, c) {
  switch (b) {
  case 1+c: return 4+5;
  case 2+c: return 6+7;
  default: return 8+9;
  }
}

function simple_test1(b, c) {
  switch (b) {
  case 1+c: return 4+5;
  case 2+c: break;
  default: return 8+9;
  }
}

function fallthrough(b) {
  switch (b) {
  case 0: null
  case 1: null
  default: null
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "days_of_the_week": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_xor": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_xor2": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "fallthrough": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %days_of_the_week(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "days_of_the_week": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %simple_xor(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "simple_xor": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %simple_xor2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "simple_xor2": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %simple_test0(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "simple_test0": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %simple_test1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "simple_test1": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %fallthrough(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "fallthrough": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function days_of_the_week(day: any, x: any): any
// CHECK-NEXT:frame = [day: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %days_of_the_week(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %day: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [day]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [day]: any
// CHECK-NEXT:  %7 = BinaryStrictlyEqualInst (:any) 0: number, %6: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [day]: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        StoreFrameInst %1: environment, "?": string, [day]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Sunday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:any) 1: number, %6: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Monday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %20 = BinaryStrictlyEqualInst (:any) %19: any, %6: any
// CHECK-NEXT:        CondBranchInst %20: any, %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Tuesday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = BinaryStrictlyEqualInst (:any) 3: number, %6: any
// CHECK-NEXT:        CondBranchInst %24: any, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Wednesday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %28 = BinaryStrictlyEqualInst (:any) 4: number, %6: any
// CHECK-NEXT:        CondBranchInst %28: any, %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Thursday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %32 = BinaryStrictlyEqualInst (:any) 5: number, %6: any
// CHECK-NEXT:        CondBranchInst %32: any, %BB13, %BB14
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Friday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %36 = BinaryStrictlyEqualInst (:any) 6: number, %6: any
// CHECK-NEXT:        CondBranchInst %36: any, %BB15, %BB16
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        StoreFrameInst %1: environment, "Saturday": string, [day]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function simple_xor(b: any): any
// CHECK-NEXT:frame = [b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_xor(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:       SwitchInst %4: any, %BB1, 1: number, %BB2, 0: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst "invalid": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 0: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function simple_xor2(b: any): any
// CHECK-NEXT:frame = [b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_xor2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:       SwitchInst %4: any, %BB3, 1: number, %BB1, 0: number, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 0: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst "invalid": string
// CHECK-NEXT:function_end

// CHECK:function simple_test0(b: any, c: any): any
// CHECK-NEXT:frame = [b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_test0(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) 1: number, %7: any
// CHECK-NEXT:  %9 = BinaryStrictlyEqualInst (:any) %8: any, %6: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst 9: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) 2: number, %12: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:any) %13: any, %6: any
// CHECK-NEXT:        CondBranchInst %14: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst 13: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst 17: number
// CHECK-NEXT:function_end

// CHECK:function simple_test1(b: any, c: any): any
// CHECK-NEXT:frame = [b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_test1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) 1: number, %7: any
// CHECK-NEXT:  %9 = BinaryStrictlyEqualInst (:any) %8: any, %6: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst 9: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) 2: number, %13: any
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:any) %14: any, %6: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst 17: number
// CHECK-NEXT:function_end

// CHECK:function fallthrough(b: any): any
// CHECK-NEXT:frame = [b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %fallthrough(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:       SwitchInst %4: any, %BB4, 0: number, %BB2, 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
