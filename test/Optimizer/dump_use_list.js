/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ir %s -dump-instr-uselist  | %FileCheckOrRegen %s --match-full-lines

function foo(a, b) {
  var c = a + b;
  return c * c;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1} // users: %1
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0 // users: %2
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret // users: %4 %5
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3 // users: %6
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b)#2
// CHECK-NEXT:frame = [a#2, b#2, c#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2} // users: %1 %2 %3 %4 %5 %7 %8 %9
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [c#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [a#2], %0 // users: %6
// CHECK-NEXT:  %5 = LoadFrameInst [b#2], %0 // users: %6
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5 // users: %7
// CHECK-NEXT:  %7 = StoreFrameInst %6, [c#2], %0
// CHECK-NEXT:  %8 = LoadFrameInst [c#2], %0 // users: %10
// CHECK-NEXT:  %9 = LoadFrameInst [c#2], %0 // users: %10
// CHECK-NEXT:  %10 = BinaryOperatorInst '*', %8, %9 // users: %11
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
