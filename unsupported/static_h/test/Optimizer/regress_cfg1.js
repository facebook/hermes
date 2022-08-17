/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s

function f() {
  for (var j = 1; j < 1; j *= -8) {
  }
  for (var i = 1; i < 1; j += 2) {
    j * -1;
  }
}

//CHECK-LABEL:function f() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

f();
