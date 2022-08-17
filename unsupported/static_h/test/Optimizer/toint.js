/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s

function foo (x) {
    return 1e20 | 0;
}

//CHECK-LABEL: function foo(x) : number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = ReturnInst 1661992960 : number
//CHECK-NEXT: function_end
