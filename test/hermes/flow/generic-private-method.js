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
  constructor() { this.value = 0; }

  // Instance private generic method.
  @Hermes.final
  #identity<T>(x: T): T { return x; }

  // Static private generic method.
  @Hermes.final
  static #wrap<T>(x: T): T { return x; }

  test(): void {
    // Explicit type args.
    print(this.#identity<number>(42));
    print(this.#identity<string>("hello"));
    // Type inference.
    print(this.#identity(99));
    print(this.#identity("world"));
  }

  static testStatic(): void {
    print(Box.#wrap<number>(10));
    print(Box.#wrap("static"));
  }
}

let b: Box = new Box();
b.test();
Box.testStatic();
// CHECK: 42
// CHECK-NEXT: hello
// CHECK-NEXT: 99
// CHECK-NEXT: world
// CHECK-NEXT: 10
// CHECK-NEXT: static
