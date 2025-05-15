/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

"use strict";

print('pop');
// CHECK-LABEL: pop
var a = Array(1,2,3);
print(a.pop(), a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 2 1 2 undefined
print(a.pop(), a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 2 1 1 undefined undefined
print(a.pop(), a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 1 0 undefined undefined undefined
print(a.pop(), a.length, a[0], a[1], a[2]);
// CHECK-NEXT: undefined 0 undefined undefined undefined
print(a.pop(), a.length, a[0], a[1], a[2]);
// CHECK-NEXT: undefined 0 undefined undefined undefined

// Pop when length is readonly.
var a = [123];
Object.defineProperty(a, 'length', {writable: false, value: 1});
try { a.pop(); } catch (e) { print(e.name); }
// CHECK-NEXT: TypeError

// Pop from sparse array.
var a = [,,,,10,,,,,];
print(a.pop(), a.length);
// CHECK-NEXT: undefined 8

// Test recursion of pop re-entering itself.
var a = [];
Object.defineProperty(a, 9, {
  get: Array.prototype.pop,
});
try {
  print(a.pop());
} catch (e) {
  // Infinite recursion, should throw call stack exceeded.
  print(e.name);
}
// CHECK-NEXT: RangeError
var a = [];
a[0xFFFFFFFE] = 1;
print(a.length);
// CHECK-NEXT: 4294967295
print(a.pop());
// CHECK-NEXT: 1
print(a.length);
// CHECK-NEXT: 4294967294

var a = {
  0: 12,
  1: 13,
  length: 2,
};
print(Array.prototype.pop.call(a), a.length);
// CHECK-NEXT: 13 1
print(Array.prototype.pop.call(a), a.length);
// CHECK-NEXT: 12 0
