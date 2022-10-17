/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines

function g12(z) {
    var w = function () { return 1; }
    w();  // site 1
    if (z > 0) {
        w(); // site 2
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [g12]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %g12#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "g12" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function g12#0#1(z)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{g12#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %w#1#2()#3 : number, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = BinaryOperatorInst '>', %z, 0 : number
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function w#1#2()#3 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{w#1#2()#3}
// CHECK-NEXT:  %1 = ReturnInst 1 : number
// CHECK-NEXT:function_end
