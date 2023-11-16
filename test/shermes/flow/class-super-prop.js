/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec -O %s | %FileCheck --match-full-lines %s

'use strict';

class A {
  x: number;

  constructor(x: number) {
    this.x = x;
  }
}

class B extends A {
  y: number;

  constructor(x: number, y: number) {
    super(x);
    this.y = y;
  }

  f(): number {
    return super.x + this.x + this.y;
  }
}

print('super prop');
// CHECK-LABEL: super prop
print(new A(2).x);
// CHECK-NEXT: 2
print(new B(3, 10).f());
// CHECK-NEXT: 16
