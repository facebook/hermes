/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -Xflow-parser -dump-ir -O0 %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function foo() {
    let x = 20;
    const y = 30;
    return x + y;
}

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
//CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [y]
//CHECK-NEXT:  %2 = StoreFrameInst 20 : number, [x]
//CHECK-NEXT:  %3 = StoreFrameInst 30 : number, [y]
//CHECK-NEXT:  %4 = LoadFrameInst [x]
//CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
//CHECK-NEXT:  %6 = LoadFrameInst [y]
//CHECK-NEXT:  %7 = ThrowIfEmptyInst %6
//CHECK-NEXT:  %8 = BinaryOperatorInst '+', %5, %7
//CHECK-NEXT:  %9 = ReturnInst %8
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %10 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
