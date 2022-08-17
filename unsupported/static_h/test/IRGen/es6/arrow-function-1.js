/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

var func1 = () => 10;
//CHECK-LABEL:arrow func1()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 10 : number
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

var func2 = () => { return 11; }
//CHECK-LABEL:arrow func2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 11 : number
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
