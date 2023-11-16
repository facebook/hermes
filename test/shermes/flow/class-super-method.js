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

  f(): number {
    print("A.f");
    return this.x * 100;
  }

  g(): number {
    print("A.g");
    return this.x * 1000;
  }
}

class B extends A {
  constructor(x: number) {
    super(x);
  }

  f(): number {
    print("B.f");
    return super.f() + 23;
  }

  h(): number {
    print("B.h");
    return super.g() + 42;
  }

  h2(): number {
    print("B.h2");
    var fn = () => super.g();
    return fn() + 42;
  }
}

print('super method');
// CHECK-LABEL: super method
print(new A(2).f());
// CHECK-NEXT: A.f
// CHECK-NEXT: 200
print(new B(3).f());
// CHECK-NEXT: B.f
// CHECK-NEXT: A.f
// CHECK-NEXT: 323
print(new B(2).h());
// CHECK-NEXT: B.h
// CHECK-NEXT: A.g
// CHECK-NEXT: 2042
print(new B(2).h2());
// CHECK-NEXT: B.h2
// CHECK-NEXT: A.g
// CHECK-NEXT: 2042
