/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

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
  withField<T>(val: T): number {
    return this.x;
  }
}

class Child extends Base {
  y: string;

  constructor() {
    super();
    this.y = "child";
  }
}

// Call generic method on subclass instance.
let child: Child = new Child();
print(child.identity<number>(42));
// CHECK: 42
print(child.identity<string>("hello"));
// CHECK-NEXT: hello

// Inferred type arguments on subclass instance.
print(child.identity(100));
// CHECK-NEXT: 100
print(child.identity("world"));
// CHECK-NEXT: world

// Generic method that accesses 'this' fields, called on subclass.
print(child.withField<string>("ignored"));
// CHECK-NEXT: 10

// Multiple type parameters on a subclass instance.
print(child.identity<boolean>(true));
// CHECK-NEXT: true
