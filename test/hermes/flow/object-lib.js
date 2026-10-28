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

/// Test the Object static and instance methods provided by the typed lib.

'use strict';

const o: {x: number, y: string} = {x: 1, y: 'hi'};

print(Object.keys(o).join(','));
// CHECK: x,y
print(Object.values(o).join(','));
// CHECK-NEXT: 1,hi
for (const e of Object.entries(o)) print(e[0], e[1]);
// CHECK-NEXT: x 1
// CHECK-NEXT: y hi

/// Index-like keys come first, in ascending numeric order; the rest keep
/// insertion order.
const indexed: {[string]: mixed} = {b: 1, '2': 'two', a: 3, '0': 'zero'};
print(Object.keys(indexed).join(','));
// CHECK-NEXT: 0,2,b,a
print(Object.values(indexed).join(','));
// CHECK-NEXT: zero,two,1,3

/// More than 8 properties, to exercise growth past the inline scan buffer.
const many: {[string]: mixed} = {
  p0: 0,
  p1: 1,
  p2: 2,
  p3: 3,
  p4: 4,
  p5: 5,
  p6: 6,
  p7: 7,
  p8: 8,
  p9: 9,
};
print(Object.keys(many).join(','));
// CHECK-NEXT: p0,p1,p2,p3,p4,p5,p6,p7,p8,p9
print(Object.values(many).join(','));
// CHECK-NEXT: 0,1,2,3,4,5,6,7,8,9
print(Object.entries(many).length);
// CHECK-NEXT: 10

print(Object.is(1, 1), Object.is(NaN, NaN), Object.is('a', 'b'));
// CHECK-NEXT: true true false

const proto: {[string]: any} = {greet: 'hello'};
const created = Object.create(proto);
print(created.greet, Object.getPrototypeOf(created) === proto);
// CHECK-NEXT: hello true
created.extra = 42;
print(created.extra, Object.keys(created).join(','));
// CHECK-NEXT: 42 extra

const nullProto = Object.create(null);
nullProto.x = 1;
print(nullProto.x, Object.getPrototypeOf(nullProto));
// CHECK-NEXT: 1 null

/// Instance methods on a direct Object instance.
const base = new Object();
print(typeof base, base.toString(), base.hasOwnProperty('nope'));
// CHECK-NEXT: object [object Object] false
