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

print(Object.keys(o));
// CHECK: x,y
print(Object.values(o));
// CHECK-NEXT: 1,hi
print(Object.entries(o));
// CHECK-NEXT: x,1,y,hi

print(Object.is(1, 1), Object.is(NaN, NaN), Object.is('a', 'b'));
// CHECK-NEXT: true true false

const proto: {[string]: any} = {greet: 'hello'};
const created = Object.create(proto);
print(created.greet, Object.getPrototypeOf(created) === proto);
// CHECK-NEXT: hello true
created.extra = 42;
print(created.extra, Object.keys(created));
// CHECK-NEXT: 42 extra

const nullProto = Object.create(null);
nullProto.x = 1;
print(nullProto.x, Object.getPrototypeOf(nullProto));
// CHECK-NEXT: 1 null

/// Instance methods on a direct Object instance.
const base = new Object();
print(typeof base, base.toString(), base.hasOwnProperty('nope'));
// CHECK-NEXT: object [object Object] false
