/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function test_assignment_expr() {
  var y = 0;
  var x = y = 4;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [test_assignment_expr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test_assignment_expr#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test_assignment_expr" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function test_assignment_expr#0#1()#2
// CHECK-NEXT:frame = [y#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_assignment_expr#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [y#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [y#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst 4 : number, [y#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst 4 : number, [x#2], %0
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
