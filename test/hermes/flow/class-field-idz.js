/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class A {}
class B {}

class C {
  p1: number;
  p2: number | A;
  p3: void | A;
  p4: A;
  p5: A | B;

  constructor() {
    this.p1 = 1;
    this.p2 = 2;
    this.p3 = undefined;
  }

  maker(): C {
    return new C();
  }

  method() {
    // These should not have ThrowIf
    print(this.p1)
    print(this.p2)
    print(this.p3)
    // These should have ThrowIf
    try { print(this.p4) } catch (e) { print('caught', e.name); }
    try { print(this.p5) } catch (e) { print('caught', e.name); }
  }
}

new C().method();

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: undefined
// CHECK-NEXT: caught ReferenceError
// CHECK-NEXT: caught ReferenceError
