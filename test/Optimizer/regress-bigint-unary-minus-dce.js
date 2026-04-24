/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheck --match-full-lines %s

function f() {
  let a = BigInt.asUintN(65472, -1n);
  let b = -a;
  -b;
}

// CHECK-LABEL:function f(): undefined
// CHECK:  %{{[0-9]+}} = UnaryMinusInst {{.*}}
// CHECK:  %{{[0-9]+}} = UnaryMinusInst {{.*}}
// CHECK:       ReturnInst undefined: undefined
