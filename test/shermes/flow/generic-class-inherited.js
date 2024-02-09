/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class A<T> {
  x: T;

  constructor(x: T) {
    this.x = x;
  }
}

class B<T, U> extends A<T> {
  y: U;

  constructor(x: T, y: U) {
    super(x);
    this.y = y;
  }

  getX(): T {
    return super.x;
  }

  getY(): U {
    return this.y;
  }
}

class C extends A<number> {
  constructor(x: number) {
    super(x);
  }

  getX(): number {
    return super.x;
  }
}

class NonGeneric {}

class D<T> extends NonGeneric {
  x: T;

  constructor(x: T) {
    super();
    this.x = x;
  }
}

print('generic class');
// CHECK-LABEL: generic class

var b: B<number, string> = new B<number, string>(123, 'abc');
print(b.x, b.y);
// CHECK-NEXT: 123 abc
print(b.getX(), b.getY());
// CHECK-NEXT: 123 abc

var c: C = new C(192);
print(c.getX());
// CHECK-NEXT: 192

var d: D<number> = new D<number>(1282);
print(d.x);
// CHECK-NEXT: 1282
