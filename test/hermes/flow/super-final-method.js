/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Test super.finalMethod() calls for both generic and non-generic final
// methods.

class Base {
  x: number;

  constructor() {
    this.x = 10;
  }

  @Hermes.final
  identity<T>(val: T): T {
    return val;
  }

  @Hermes.final
  getX(): number {
    return this.x;
  }
}

class Child extends Base {
  y: number;

  constructor() {
    super();
    this.y = 20;
  }

  callSuperGeneric(): number {
    return super.identity<number>(42);
  }

  callSuperNonGeneric(): number {
    return super.getX();
  }
}

let child: Child = new Child();
print(child.callSuperGeneric());
// CHECK: 42
print(child.callSuperNonGeneric());
// CHECK-NEXT: 10
