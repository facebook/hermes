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

'use strict';

class A {
  x: number;
  static y: number = 10;
  static #secret: number = 99;
  static foo(): number { return 42; }
  static getSecret(): number { return A.#secret; }

  constructor(x: number) {
    this.x = x;
  }

  bar(): number { return this.x; }
}

class B extends A {
  static z: string = "hello";
  static foo(): number { return 99; }

  constructor(x: number) {
    super(x);
  }
}

// Test static fields.
print("static fields");
// CHECK-LABEL: static fields
print(A.y);
// CHECK-NEXT: 10
print(B.z);
// CHECK-NEXT: hello

// Test static methods.
print("static methods");
// CHECK-LABEL: static methods
print(A.foo());
// CHECK-NEXT: 42
print(B.foo());
// CHECK-NEXT: 99

// Test private static fields.
print("private static");
// CHECK-LABEL: private static
print(A.getSecret());
// CHECK-NEXT: 99

// Test instance functionality still works with static members present.
print("instance");
// CHECK-LABEL: instance
print(new A(5).bar());
// CHECK-NEXT: 5
print(new B(7).bar());
// CHECK-NEXT: 7

// Test static private methods.
class C {
  static #secret: number = 42;
  static #getSecret(): number { return C.#secret; }
  static #double(x: number): number { return x * 2; }
  static callGetSecret(): number { return C.#getSecret(); }
  static callDouble(x: number): number { return C.#double(x); }
}

print("static private methods");
// CHECK-LABEL: static private methods
print(C.callGetSecret());
// CHECK-NEXT: 42
print(C.callDouble(21));
// CHECK-NEXT: 42

// Test writing to static fields.
print("static field write");
// CHECK-LABEL: static field write
A.y = 42;
print(A.y);
// CHECK-NEXT: 42

// Test increment/decrement of static fields.
print("static field increment");
// CHECK-LABEL: static field increment
A.y = 0;
A.y++;
A.y++;
A.y++;
print(A.y);
// CHECK-NEXT: 3

// Test static field as mutable counter (the widgets.js pattern).
class Counter {
  static id: number = 0;
  static next(): number {
    return Counter.id++;
  }
}
print("static counter");
// CHECK-LABEL: static counter
print(Counter.next());
// CHECK-NEXT: 0
print(Counter.next());
// CHECK-NEXT: 1
print(Counter.next());
// CHECK-NEXT: 2
