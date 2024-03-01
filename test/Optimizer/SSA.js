/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

function simple(x, y) {
  var t = 9;
  if (x) { t = 19; }
  return t;
}

function control_flow(x, y) {
  var t = 9;

  if (x) {
    t = 19;
  } else {
    t = 12;
    if (y) {
      t = 15;
    } else {
      t = 4;
    }
  }
  t = t + 1;
  if (x - 1) {
    t = y;
  } else {
    if (y) { t = 15; }
  }

  return t;
}

function control_catch(x, y) {
  var t = 9;

  if (x) {
    try {
      if (j) {
        throw 3;
      }
      t = 19;
    } catch (e) {
      return t;
    }
  }

  return t;
}

function multi(x, y) {
  var t0 = 9;
  var t1 = 9;
  if (x) { t0 = 19; t1 = t0; }
  return t0 + t1;
}

// Make sure that we are not promoting allocas inside try-catch sections.
function badThrow() {
  var result = -1;
  try {
      result = 100;
      throw "hello";
  } catch (e) {}
  return result;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:       DeclareGlobalVarInst "control_flow": string
// CHECK-NEXT:       DeclareGlobalVarInst "control_catch": string
// CHECK-NEXT:       DeclareGlobalVarInst "multi": string
// CHECK-NEXT:       DeclareGlobalVarInst "badThrow": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "simple": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %control_flow(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "control_flow": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %control_catch(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "control_catch": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %multi(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "multi": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %badThrow(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "badThrow": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple(x: any, y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = PhiInst (:number) 19: number, %BB1, 9: number, %BB0
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function control_flow(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       CondBranchInst %1: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = PhiInst (:number) 4: number, %BB3, 19: number, %BB0, 15: number, %BB1
// CHECK-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %6 = BinarySubtractInst (:number) %0: any, 1: number
// CHECK-NEXT:       CondBranchInst %6: number, %BB5, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       CondBranchInst %1: any, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = PhiInst (:any) 15: number, %BB6, %5: number, %BB4, %1: any, %BB2
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:function_end

// CHECK:function control_catch(x: any, y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:number) $t: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreStackInst 9: number, %0: number
// CHECK-NEXT:       CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadStackInst (:number) %0: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = CatchInst (:any)
// CHECK-NEXT:  %8 = LoadStackInst (:number) %0: number
// CHECK-NEXT:       ReturnInst %8: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "j": string
// CHECK-NEXT:        CondBranchInst %10: any, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ThrowInst 3: number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreStackInst 19: number, %0: number
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function multi(x: any, y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = PhiInst (:number) 19: number, %BB1, 9: number, %BB0
// CHECK-NEXT:  %4 = PhiInst (:number) 19: number, %BB1, 9: number, %BB0
// CHECK-NEXT:  %5 = FAddInst (:number) %3: number, %4: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function badThrow(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:number) $result: any
// CHECK-NEXT:       StoreStackInst -1: number, %0: number
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadStackInst (:number) %0: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreStackInst 100: number, %0: number
// CHECK-NEXT:       ThrowInst "hello": string
// CHECK-NEXT:function_end
