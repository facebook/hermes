/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Test that a generic method can use 'super' to call parent methods.

class Parent {
  x: number;

  constructor() {
    this.x = 100;
  }

  getX(): number {
    return this.x;
  }
}

class Child extends Parent {
  constructor() {
    super();
  }

  @Hermes.final
  superGetX<T>(val: T): number {
    return super.getX();
  }
}

let child: Child = new Child();
print(child.superGetX<string>("ignored"));
// CHECK: 100
print(child.superGetX<number>(0));
// CHECK-NEXT: 100
