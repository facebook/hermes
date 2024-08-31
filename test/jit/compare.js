/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function greater(x, y) {
  return x > y;
}

print('greater');
// CHECK-LABEL: greater
print(greater(1, 2));
// CHECK-NEXT: false
print(greater(3, 2));
// CHECK-NEXT: true
print(greater(2, 2));
// CHECK-NEXT: false
print(greater('1', '2'));
// CHECK-NEXT: false
print(greater('3', '2'));
// CHECK-NEXT: true

function greaterEq(x, y) {
  return x >= y;
}

print('greaterEq');
// CHECK-LABEL: greaterEq
print(greaterEq(1, 2));
// CHECK-NEXT: false
print(greaterEq(3, 2));
// CHECK-NEXT: true
print(greaterEq(2, 2));
// CHECK-NEXT: true
print(greaterEq('1', '2'));
// CHECK-NEXT: false
print(greaterEq('3', '2'));
// CHECK-NEXT: true

function less(x, y) {
  return x < y;
}

print('less');
// CHECK-LABEL: less
print(less(1, 2));
// CHECK-NEXT: true
print(less(3, 2));
// CHECK-NEXT: false
print(less(2, 2));
// CHECK-NEXT: false
print(less('1', '2'));
// CHECK-NEXT: true
print(less('3', '2'));
// CHECK-NEXT: false

function lessEq(x, y) {
  return x <= y;
}

print('lessEq');
// CHECK-LABEL: lessEq
print(lessEq(1, 2));
// CHECK-NEXT: true
print(lessEq(3, 2));
// CHECK-NEXT: false
print(lessEq(2, 2));
// CHECK-NEXT: true
print(lessEq('1', '2'));
// CHECK-NEXT: true
print(lessEq('3', '2'));
// CHECK-NEXT: false

function strictEq(x, y) {
  return x === y;
}

print('strictEq');
// CHECK-LABEL: strictEq
print(strictEq(1, 2));
// CHECK-NEXT: false
print(strictEq(3, 2));
// CHECK-NEXT: false
print(strictEq(2, 2));
// CHECK-NEXT: true
print(strictEq('1', '2'));
// CHECK-NEXT: false
print(strictEq('3', '2'));
// CHECK-NEXT: false

function strictNeq(x, y) {
  return x !== y;
}

print('strictNeq');
// CHECK-LABEL: strictNeq
print(strictNeq(1, 2));
// CHECK-NEXT: true
print(strictNeq(3, 2));
// CHECK-NEXT: true
print(strictNeq(2, 2));
// CHECK-NEXT: false
print(strictNeq('1', '2'));
// CHECK-NEXT: true
print(strictNeq('3', '2'));
// CHECK-NEXT: true

function eq(x, y) {
  return x == y;
}

print('eq');
// CHECK-LABEL: eq
print(eq(1, 2));
// CHECK-NEXT: false
print(eq(3, 2));
// CHECK-NEXT: false
print(eq(2, 2));
// CHECK-NEXT: true
print(eq('1', '2'));
// CHECK-NEXT: false
print(eq('3', '2'));
// CHECK-NEXT: false

function neq(x, y) {
  return x != y;
}

print('neq');
// CHECK-LABEL: neq
print(neq(1, 2));
// CHECK-NEXT: true
print(neq(3, 2));
// CHECK-NEXT: true
print(neq(2, 2));
// CHECK-NEXT: false
print(neq('1', '2'));
// CHECK-NEXT: true
print(neq('3', '2'));
// CHECK-NEXT: true