/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo(obj, x) {
  print(delete obj.a);
  print(delete obj[x]);
}

var obj = {a: 1, b: 2};
foo(obj, 'b');
print(obj.a, obj.b);
// CHECK: true
// CHECK-NEXT: true
// CHECK-NEXT: undefined undefined
