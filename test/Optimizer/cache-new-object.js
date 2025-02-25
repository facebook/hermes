/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function simple(x, y) {
  this.x = x;
  this.y = y;
}

function simpleWithBranch(x, y) {
  this.x = x;
  this.y = y;
  this.z = x ? 1 : 2;
}

function beforeCond(x, y, z) {
  this.x = x;
  this.y = y;
  if (z) {
    this.z = z;
  }
}

function uniq(x, y, z) {
  this.x = x;
  this.y = y;
  this.z = z;
  this.x = x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleWithBranch": string
// CHECK-NEXT:       DeclareGlobalVarInst "beforeCond": string
// CHECK-NEXT:       DeclareGlobalVarInst "uniq": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "simple": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %simpleWithBranch(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "simpleWithBranch": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %beforeCond(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "beforeCond": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) empty: any, empty: any, %uniq(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "uniq": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple(x: any, y: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %1: object, %2: undefined|object, "x": string, "y": string
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst %5: any, %1: object, "y": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simpleWithBranch(x: any, y: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %1: object, %2: undefined|object, "x": string, "y": string, "z": string
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst %5: any, %1: object, "y": string
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst (:number) 1: number, %BB1, 2: number, %BB0
// CHECK-NEXT:        StorePropertyLooseInst %10: number, %1: object, "z": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function beforeCond(x: any, y: any, z: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %1: object, %2: undefined|object, "x": string, "y": string
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst %5: any, %1: object, "y": string
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        StorePropertyLooseInst %6: any, %1: object, "z": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function uniq(x: any, y: any, z: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %1: object, %2: undefined|object, "x": string, "y": string, "z": string
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:       StorePropertyLooseInst %5: any, %1: object, "y": string
// CHECK-NEXT:       StorePropertyLooseInst %6: any, %1: object, "z": string
// CHECK-NEXT:        StorePropertyLooseInst %4: any, %1: object, "x": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
