/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s

// Make sure that Mem2Reg doesn't promote loads across writing
// instructions with AllocStackInst operands.

function foo(x) {
  var [a,b] = x;
  print(a,b)
}

//CHECK-LABEL:function foo(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %1 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %2 = StoreStackInst %x, %1
//CHECK-NEXT:  %3 = IteratorBeginInst %1
//CHECK-NEXT:  %4 = StoreStackInst %3, %0
//CHECK-NEXT:  %5 = IteratorNextInst %0, %1
//CHECK-NEXT:  %6 = LoadStackInst %0
//CHECK-NEXT:  %7 = BinaryOperatorInst '===', %6, undefined : undefined
