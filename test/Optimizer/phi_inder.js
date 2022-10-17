/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

function recursive_phi(x) {
  var k = 1;
  var j = "hi";
  var t;

  for (var i = 0; i < 10; i++) {
    if (x > 3) {
      t = k;
      k = j;
      j = t;
    }
  }

  return k + j;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [sink, recursive_phi]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %recursive_phi#0#1()#3 : string|number, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "recursive_phi" : string
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink#0#1()#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function recursive_phi#0#1(x)#3 : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{recursive_phi#0#1()#3}
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst 1 : number, %BB0, %9 : string|number, %BB2
// CHECK-NEXT:  %3 = PhiInst "hi" : string, %BB0, %10 : string|number, %BB2
// CHECK-NEXT:  %4 = PhiInst 0 : number, %BB0, %11 : number|bigint, %BB2
// CHECK-NEXT:  %5 = BinaryOperatorInst '>', %x, 3 : number
// CHECK-NEXT:  %6 = CondBranchInst %5 : boolean, %BB3, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %9 : string|number, %10 : string|number
// CHECK-NEXT:  %8 = ReturnInst %7 : string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = PhiInst %3 : string|number, %BB3, %2 : string|number, %BB1
// CHECK-NEXT:  %10 = PhiInst %2 : string|number, %BB3, %3 : string|number, %BB1
// CHECK-NEXT:  %11 = UnaryOperatorInst '++', %4 : number|bigint
// CHECK-NEXT:  %12 = BinaryOperatorInst '<', %11 : number|bigint, 10 : number
// CHECK-NEXT:  %13 = CondBranchInst %12 : boolean, %BB1, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = BranchInst %BB2
// CHECK-NEXT:function_end
