/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xflow-parser -dump-ir -O0 %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function foo() {
    let x = 20;
    const y = 30;
    return x + y;
}

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x, y, ?anon_1_tdz$y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [?anon_1_tdz$y]
//CHECK-NEXT:  %4 = StoreFrameInst 20 : number, [x]
//CHECK-NEXT:  %5 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %6 = StoreFrameInst 30 : number, [y]
//CHECK-NEXT:  %7 = StoreFrameInst true : boolean, [?anon_1_tdz$y]
//CHECK-NEXT:  %8 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %9 = ThrowIfUndefinedInst %8
//CHECK-NEXT:  %10 = LoadFrameInst [x]
//CHECK-NEXT:  %11 = LoadFrameInst [?anon_1_tdz$y]
//CHECK-NEXT:  %12 = ThrowIfUndefinedInst %11
//CHECK-NEXT:  %13 = LoadFrameInst [y]
//CHECK-NEXT:  %14 = BinaryOperatorInst '+', %10, %13
//CHECK-NEXT:  %15 = ReturnInst %14
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
