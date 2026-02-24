/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function check(x) {
  return [x === null, x !== null];
}

print(check(null));
// CHECK: true,false
print(check(1));
// CHECK-NEXT: false,true
print(check('a'));
// CHECK-NEXT: false,true
print(check(1n));
// CHECK-NEXT: false,true
print(check({}));
// CHECK-NEXT: false,true
print(check(NaN));
// CHECK-NEXT: false,true
print(check(1.12984675));
// CHECK-NEXT: false,true
print(check(undefined));
// CHECK-NEXT: false,true
print(check(true));
// CHECK-NEXT: false,true
