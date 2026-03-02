/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

class A {
  #x: number;
  a: number;
  constructor(xa, a) {
    this.#x = xa;
    this.a = a;
  }
  getXA() {
    return this.#x;
  }
}

class B extends A {
  b: number;
  #x: number;
  constructor(xa, xb, a, b) {
    super(xa, a);
    this.#x = xb;
    this.b = b;
  }
  getXB() {
    return this.#x;
  }
}

var b = new B(1, 2, 3, 4);
print(b.getXA());
// CHECK: 1
print(b.getXB());
// CHECK-NEXT: 2
print(b.a);
// CHECK-NEXT: 3
print(b.b);
// CHECK-NEXT: 4

class C {
  #x: number;
  y: number;
  constructor(x, y) {
    this.#x = x;
    this.y = y;
  }

  method(): number {
    return this.#privateMethod();
  }

  #privateMethod(): number {
    return this.#x;
  }
}

var c = new C(1, 2);
print(c.method());
// CHECK-NEXT: 1
print(c.y);
// CHECK-NEXT: 2

class Outer {
  #x: number;
  #y(): number { return 1; }
  #x2: number;
  constructor(x) {
    this.#x = x;
    this.#x2 = x;
  }
  foo() {
    class Inner {
      #x: string;
      bar(out: Outer) {
        return out.#x + out.#y() + out.#x2;
      }
    }
    return new Inner().bar(this);
  }
}

print(new Outer(10).foo());
// CHECK-NEXT: 21
