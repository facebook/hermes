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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() // users: %1
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret // users: %3 %4
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2 // users: %5
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(a, b)
// CHECK-NEXT:frame = [c, a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [c]
// CHECK-NEXT:  %1 = LoadParamInst %a // users: %2
// CHECK-NEXT:  %2 = StoreFrameInst %1, [a]
// CHECK-NEXT:  %3 = LoadParamInst %b // users: %4
// CHECK-NEXT:  %4 = StoreFrameInst %3, [b]
// CHECK-NEXT:  %5 = LoadFrameInst [a] // users: %7
// CHECK-NEXT:  %6 = LoadFrameInst [b] // users: %7
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6 // users: %8
// CHECK-NEXT:  %8 = StoreFrameInst %7, [c]
// CHECK-NEXT:  %9 = LoadFrameInst [c] // users: %11
// CHECK-NEXT:  %10 = LoadFrameInst [c] // users: %11
// CHECK-NEXT:  %11 = BinaryOperatorInst '*', %9, %10 // users: %12
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
