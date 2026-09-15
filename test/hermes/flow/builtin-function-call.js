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

/// Test runtime behavior of fn.call(thisArg, args...) on typed functions.

'use strict';

(function () {

class C {
  x: number;
  constructor(x: number) {
    this.x = x;
  }
}

function add(this: C, n: number): number {
  return this.x + n;
}

function mul(a: number, b: number): number {
  return a * b;
}

print(add.call(new C(10), 5));
// CHECK: 15

print(mul.call(undefined, 3, 4));
// CHECK-NEXT: 12

// Result type flows through correctly.
let total: number = add.call(new C(1), 2) + mul.call(undefined, 3, 4);
print(total);
// CHECK-NEXT: 15

})();
