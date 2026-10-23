/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// `new` with partial type arguments infers the rest from the constructor.

'use strict';

class C<A, B> {
  a: A;
  b: B;
  constructor(a: A, b: B) {
    this.a = a;
    this.b = b;
  }
}
const c = new C<number>(1, 'x');
print(c.a + 1, c.b.length);
// CHECK: 2 1
const d = new C<number>(2, (n: number): number => n * 3);
print(d.b(4));
// CHECK-NEXT: 12
