/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

function bug1() {
  var x;
  while (true) { ++x; continue; }
}

function bug2() {
  while (true) { continue; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [bug1, bug2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %bug1#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "bug1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %bug2#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "bug2" : string
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bug1#0#1()#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bug1#0#1()#2}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst undefined : undefined, %BB0, %3 : number|bigint, %BB1
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2 : undefined|number|bigint
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function bug2#0#1()#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bug2#0#1()#3}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:function_end
