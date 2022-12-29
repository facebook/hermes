/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fcache-new-object -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function simple(x, y) {
  this.x = x;
  this.y = y;
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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [simple, beforeCond, uniq]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %simple() : undefined
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "simple" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %beforeCond() : undefined
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "beforeCond" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %uniq() : undefined
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "uniq" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = CacheNewObjectInst %1 : object, %2, "x" : string, "y" : string
// CHECK-NEXT:  %4 = LoadParamInst %x
// CHECK-NEXT:  %5 = LoadParamInst %y
// CHECK-NEXT:  %6 = StorePropertyLooseInst %4, %1 : object, "x" : string
// CHECK-NEXT:  %7 = StorePropertyLooseInst %5, %1 : object, "y" : string
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function beforeCond(x, y, z) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = CacheNewObjectInst %1 : object, %2, "x" : string, "y" : string
// CHECK-NEXT:  %4 = LoadParamInst %x
// CHECK-NEXT:  %5 = LoadParamInst %y
// CHECK-NEXT:  %6 = LoadParamInst %z
// CHECK-NEXT:  %7 = StorePropertyLooseInst %4, %1 : object, "x" : string
// CHECK-NEXT:  %8 = StorePropertyLooseInst %5, %1 : object, "y" : string
// CHECK-NEXT:  %9 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = StorePropertyLooseInst %6, %1 : object, "z" : string
// CHECK-NEXT:  %11 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function uniq(x, y, z) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = CacheNewObjectInst %1 : object, %2, "x" : string, "y" : string, "z" : string
// CHECK-NEXT:  %4 = LoadParamInst %x
// CHECK-NEXT:  %5 = LoadParamInst %y
// CHECK-NEXT:  %6 = LoadParamInst %z
// CHECK-NEXT:  %7 = StorePropertyLooseInst %4, %1 : object, "x" : string
// CHECK-NEXT:  %8 = StorePropertyLooseInst %5, %1 : object, "y" : string
// CHECK-NEXT:  %9 = StorePropertyLooseInst %6, %1 : object, "z" : string
// CHECK-NEXT:  %10 = StorePropertyLooseInst %4, %1 : object, "x" : string
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
