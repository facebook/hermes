/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines


//CHECK-LABEL:function bug1() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %1 = PhiInst undefined : undefined, %BB0, %3 : number, %BB1
//CHECK-NEXT:  %2 = AsNumberInst %1 : undefined|number
//CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2 : number
//CHECK-NEXT:  %4 = BranchInst %BB1
//CHECK-NEXT:function_end
function bug1() {
  var x;
  while (true) { ++x; continue; }
}

//CHECK-LABEL:function bug2() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = BranchInst %BB1
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %1 = BranchInst %BB1
//CHECK-NEXT:function_end
function bug2() {
  while (true) { continue; }
}
