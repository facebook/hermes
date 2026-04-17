/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test that user-defined types are visible inside callbacks passed to FlowLib
/// generic methods like Array.prototype.map.

'use strict';

(function () {

class Foo {
  x: number;
  constructor(x: number) {
    'inline';
    this.x = x;
  }
}

const nums: number[] = [1, 2, 3];

// User-defined type annotation on local variable inside .map() callback.
const a: Foo[] = nums.map(n => {
  const f: Foo = new Foo(n);
  return f;
});
print(a.length, a[0].x, a[1].x, a[2].x);
// CHECK: 3 1 2 3

// User-defined return type annotation on .map() callback.
const b: Foo[] = nums.map((n: number): Foo => new Foo(n * 10));
print(b.length, b[0].x, b[1].x, b[2].x);
// CHECK-NEXT: 3 10 20 30

// Map with thisArg.
const offset: number[] = [100];
const c: number[] = nums.map(function(this: number[], n: number): number {
  return n + this[0];
}, offset);
print(c.length, c[0], c[1], c[2]);
// CHECK-NEXT: 3 101 102 103

})();
