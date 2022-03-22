/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheck %s

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 10 : number
//CHECK-NEXT:function_end
function foo() {
  var k = 4;
  k++
  k++
  k++
  k++
  k++
  k++
  return k
}

foo()
