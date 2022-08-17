/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines


// Make sure we are not removing the binary operator. "In" may throw.
//CHECK-LABEL:function test0() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = BinaryOperatorInst 'in', "foo" : string, true : boolean
//CHECK-NEXT:    %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test0() {
  "foo" in true;
}
