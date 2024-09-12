/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

print('arguments');
// CHECK-LABEL: arguments

// Function that doesn't reify arguments (test fast paths).
(function noReify(a,b,c) {

print(arguments[-1]);
// CHECK-NEXT: undefined
print(arguments[0]);
// CHECK-NEXT: 10
print(arguments[1]);
// CHECK-NEXT: 20
print(arguments[2]);
// CHECK-NEXT: 30
print(arguments[3]);
// CHECK-NEXT: undefined

print(arguments.length);
// CHECK-NEXT: 3

})(10, 20, 30);

// Mangles arguments.
function evil(a) {
  a.length = 100;
  a[123] = 'wat';
  delete a[1];
}

// Function that leaks arguments and reifies.
(function reify(a,b,c) {

// Reify twice.
evil(arguments);
evil(arguments);

print(arguments[-1]);
// CHECK-NEXT: undefined
print(arguments[0]);
// CHECK-NEXT: 10
print(arguments[1]);
// CHECK-NEXT: undefined
print(arguments[2]);
// CHECK-NEXT: 30
print(arguments[3]);
// CHECK-NEXT: undefined

print(arguments[123]);
// CHECK-NEXT: wat

print(arguments.length);
// CHECK-NEXT: 100

})(10, 20, 30);
