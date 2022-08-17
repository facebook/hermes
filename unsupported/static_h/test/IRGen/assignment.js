/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O



//CHECK-LABEL:function test_assignment_expr()
//CHECK-NEXT:frame = [y, x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:    %1 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:    %2 = StoreFrameInst 0 : number, [y]
//CHECK-NEXT:    %3 = StoreFrameInst 4 : number, [y]
//CHECK-NEXT:    %4 = StoreFrameInst 4 : number, [x]
//CHECK-NEXT:    %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_assignment_expr() {
  var y = 0;
  var x = y = 4;
}




