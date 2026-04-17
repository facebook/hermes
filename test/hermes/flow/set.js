/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// Basic add/has.
let s = new Set<string>();
s.add('a');
s.add('b');
print(s.has('a'));
// CHECK: true
print(s.has('b'));
// CHECK-NEXT: true
print(s.has('c'));
// CHECK-NEXT: false

// Chained add.
let s2 = new Set<number>();
s2.add(1).add(2).add(3);
print(s2.has(1));
// CHECK-NEXT: true
print(s2.has(3));
// CHECK-NEXT: true

// delete().
print(s.delete('a'));
// CHECK-NEXT: true
print(s.has('a'));
// CHECK-NEXT: false
print(s.delete('a'));
// CHECK-NEXT: false

// clear().
s.add('x');
s.add('y');
print(s.has('x'));
// CHECK-NEXT: true
s.clear();
print(s.has('x'));
// CHECK-NEXT: false
print(s.has('b'));
// CHECK-NEXT: false

// forEach().
let s3 = new Set<number>();
s3.add(10).add(20).add(30);
s3.forEach(v => { print(v); });
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 30
