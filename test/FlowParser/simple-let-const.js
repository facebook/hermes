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

//CHECK-LABEL:function foo#0#1()#2
//CHECK-NEXT:frame = [x#2, y#2]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
//CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#2], %0
//CHECK-NEXT:  %2 = StoreFrameInst empty : empty, [y#2], %0
//CHECK-NEXT:  %3 = StoreFrameInst 20 : number, [x#2], %0
//CHECK-NEXT:  %4 = StoreFrameInst 30 : number, [y#2], %0
//CHECK-NEXT:  %5 = LoadFrameInst [x#2], %0
//CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
//CHECK-NEXT:  %7 = LoadFrameInst [y#2], %0
//CHECK-NEXT:  %8 = ThrowIfEmptyInst %7
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %6, %8
//CHECK-NEXT:  %10 = ReturnInst %9
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
