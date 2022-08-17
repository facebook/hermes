/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra -dump-operand-registers %s | %FileCheck --match-full-lines %s
// Ensure that the register allocator correctly handles cycles between Phi-nodes.

function foo (a, b) {
    var t;
    for(;;) {
        t = a;
        a = b;
        b = t;
    }
}

//CHECK-LABEL:function foo(a, b) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  $Reg1 @0 [1...3) 	%0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  $Reg0 @1 [2...4) 	%1 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  $Reg1 @2 [3...6) 	%2 = MovInst %0 @ $Reg1
//CHECK-NEXT:  $Reg0 @3 [4...7) 	%3 = MovInst %1 @ $Reg0
//CHECK-NEXT:  $Reg2 @4 [empty]	%4 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  $Reg1 @5 [1...8) [10...12) 	%5 = PhiInst %2 @ $Reg1, %BB0, %9 @ $Reg1, %BB1
//CHECK-NEXT:  $Reg0 @6 [2...9) [11...12) 	%6 = PhiInst %3 @ $Reg0, %BB0, %10 @ $Reg0, %BB1
//CHECK-NEXT:  $Reg2 @7 [8...11) 	%7 = MovInst %5 @ $Reg1
//CHECK-NEXT:  $Reg0 @8 [2...10) [11...12) 	%8 = MovInst %6 @ $Reg0
//CHECK-NEXT:  $Reg1 @9 [10...11) 	%9 = MovInst %8 @ $Reg0
//CHECK-NEXT:  $Reg0 @10 [empty]	%10 = MovInst %7 @ $Reg2
//CHECK-NEXT:  $Reg0 @11 [empty]	%11 = BranchInst %BB1
//CHECK-NEXT:function_end
