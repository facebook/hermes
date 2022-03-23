/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ir %s -dump-instr-uselist  | %FileCheck %s --match-full-lines

//CHECK-LABEL:function foo(a, b)
//CHECK-NEXT:frame = [c, a, b]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [c]
//CHECK-NEXT:  %1 = StoreFrameInst %a, [a]
//CHECK-NEXT:  %2 = StoreFrameInst %b, [b]
//CHECK-NEXT:  %3 = LoadFrameInst [a] // users: %5
//CHECK-NEXT:  %4 = LoadFrameInst [b] // users: %5
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3, %4 // users: %6
//CHECK-NEXT:  %6 = StoreFrameInst %5, [c]
//CHECK-NEXT:  %7 = LoadFrameInst [c] // users: %9
//CHECK-NEXT:  %8 = LoadFrameInst [c] // users: %9
//CHECK-NEXT:  %9 = BinaryOperatorInst '*', %7, %8 // users: %10
//CHECK-NEXT:  %10 = ReturnInst %9
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function foo(a, b) {
  var c = a + b;
  return c * c;
}
