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

// Basic get/set with string keys and number values.
let m = new Map<string, number>();
m.set('a', 1);
m.set('b', 2);
print(m.get('a'));
// CHECK: 1
print(m.get('b'));
// CHECK-NEXT: 2
print(m.get('c'));
// CHECK-NEXT: undefined

// Overwrite existing key.
m.set('a', 42);
print(m.get('a'));
// CHECK-NEXT: 42

// Number keys with string values.
let m2 = new Map<number, string>();
m2.set(1, 'hello');
m2.set(2, 'world');
print(m2.get(1));
// CHECK-NEXT: hello
print(m2.get(2));
// CHECK-NEXT: world
print(m2.get(3));
// CHECK-NEXT: undefined

// Chained set calls.
let m3 = new Map<string, number>();
m3.set('x', 10).set('y', 20).set('z', 30);
print(m3.get('x'));
// CHECK-NEXT: 10
print(m3.get('y'));
// CHECK-NEXT: 20
print(m3.get('z'));
// CHECK-NEXT: 30

// has().
print(m.has('a'));
// CHECK-NEXT: true
print(m.has('nonexistent'));
// CHECK-NEXT: false

// delete().
print(m.delete('a'));
// CHECK-NEXT: true
print(m.has('a'));
// CHECK-NEXT: false
print(m.get('a'));
// CHECK-NEXT: undefined
print(m.delete('a'));
// CHECK-NEXT: false

// clear().
m.set('x', 1);
m.set('y', 2);
print(m.has('x'));
// CHECK-NEXT: true
m.clear();
print(m.has('x'));
// CHECK-NEXT: false
print(m.has('b'));
// CHECK-NEXT: false
