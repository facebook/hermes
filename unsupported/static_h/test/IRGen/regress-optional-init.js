/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s
//
// Make sure that the optional initialization generates a correct Phi node, when the initialization
// is multi-block.

var a, b;
function foo(param = a || b) {}

//CHECK-LABEL:function foo(param)
//CHECK-NEXT:frame = [param]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '!==', %param, undefined : undefined
//CHECK-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %2 = AllocStackInst $?anon_0_logical
//CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %4 = StoreStackInst %3, %2
//CHECK-NEXT:  %5 = CondBranchInst %3, %BB3, %BB4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %6 = PhiInst %param, %BB0, %12, %BB3
//CHECK-NEXT:  %7 = StoreFrameInst %6, [param]
//CHECK-NEXT:  %8 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %9 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %10 = StoreStackInst %9, %2
//CHECK-NEXT:  %11 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %12 = LoadStackInst %2
//CHECK-NEXT:  %13 = BranchInst %BB1
//CHECK-NEXT:function_end
