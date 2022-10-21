/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fcache-new-object -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [simple, beforeCond, uniq]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %beforeCond#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "beforeCond" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %uniq#0#1()#4 : undefined, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "uniq" : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple#0#1(x, y)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CacheNewObjectInst %this, "x" : string, "y" : string
// CHECK-NEXT:  %1 = CreateScopeInst %S{simple#0#1()#2}
// CHECK-NEXT:  %2 = StorePropertyInst %x, %0 : object, "x" : string
// CHECK-NEXT:  %3 = StorePropertyInst %y, %0 : object, "y" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function beforeCond#0#1(x, y, z)#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CacheNewObjectInst %this, "x" : string, "y" : string
// CHECK-NEXT:  %1 = CreateScopeInst %S{beforeCond#0#1()#3}
// CHECK-NEXT:  %2 = StorePropertyInst %x, %0 : object, "x" : string
// CHECK-NEXT:  %3 = StorePropertyInst %y, %0 : object, "y" : string
// CHECK-NEXT:  %4 = CondBranchInst %z, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = StorePropertyInst %z, %0 : object, "z" : string
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function uniq#0#1(x, y, z)#4 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CacheNewObjectInst %this, "x" : string, "y" : string, "z" : string
// CHECK-NEXT:  %1 = CreateScopeInst %S{uniq#0#1()#4}
// CHECK-NEXT:  %2 = StorePropertyInst %x, %0 : object, "x" : string
// CHECK-NEXT:  %3 = StorePropertyInst %y, %0 : object, "y" : string
// CHECK-NEXT:  %4 = StorePropertyInst %z, %0 : object, "z" : string
// CHECK-NEXT:  %5 = StorePropertyInst %x, %0 : object, "x" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
