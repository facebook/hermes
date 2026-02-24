/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit


function mod(a, b) {
    return a % b;
}

print("mod", mod(10, 3));
// CHECK: mod 1

function addS(b) {
    return "a" + b + "c";
}

print("addS", addS("B"));
// CHECK-NEXT: addS aBc

function ushr0(a) {
  return a >>> 0;
}
print(ushr0(-2000000000));
// CHECK-NEXT: 2294967296
print(ushr0(3000000000));
// CHECK-NEXT: 3000000000
