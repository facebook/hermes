/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheck %s

// Make sure that we are promoting t to a constant value in bar()
function foo(p1) {
  var t = 123;

//CHECK-LABEL:function bar()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 123 : number
//CHECK-NEXT:function_end

  function bar() {
    return t;
  }

  return bar;
}

foo()()

