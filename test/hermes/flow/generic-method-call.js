/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class Box {
  value: number;

  constructor() {
    this.value = 0;
  }

  @Hermes.final
  identity<T>(x: T): T {
    return x;
  }

  @Hermes.final
  pair<T, U>(x: T, y: U): T {
    return x;
  }
}

let box: Box = new Box();

// Test explicit type arguments.
print(box.identity<number>(42));
// CHECK: 42
print(box.identity<string>("hello"));
// CHECK-NEXT: hello

// Test type inference.
print(box.identity(42));
// CHECK-NEXT: 42
print(box.identity("hello"));
// CHECK-NEXT: hello

// Test multiple type parameters with explicit args.
print(box.pair<number, string>(10, "test"));
// CHECK-NEXT: 10
