/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo() {
  return {
    get x() {
      return 1;
    },
    set x(x) {
      print('set', x);
    },
  };
}

var obj = foo();
print(obj.x);
// CHECK: 1
obj.x = 2;
// CHECK-NEXT: set 2
