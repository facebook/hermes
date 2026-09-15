/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Type-based overload resolution on a private static method.
class Foo {
  @Hermes.final @Hermes.overload
  static #bar(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  static #bar(x: string): string { return x + x; }

  // Private statics are only visible inside the class, so we need a
  // public static entry point to call them from the top level.
  static run(): void {
    print(Foo.#bar(21));
    print(Foo.#bar("ab"));
  }
}

print("type-based");
// CHECK-LABEL: type-based
Foo.run();
// CHECK-NEXT: 42
// CHECK-NEXT: abab

// Arity-based overload resolution on a private static method.
class Multi {
  @Hermes.final @Hermes.overload
  static #calc(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  static #calc(x: number, y: number): number { return x + y; }
  @Hermes.final @Hermes.overload
  static #calc(x: number, y: number, z: number): number { return x + y + z; }

  static run(): void {
    print(Multi.#calc(1));
    print(Multi.#calc(1, 2));
    print(Multi.#calc(1, 2, 3));
  }
}

print("arity");
// CHECK-LABEL: arity
Multi.run();
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 6

// Mixed generic and non-generic private static overloads.
class Converter {
  @Hermes.final @Hermes.overload
  static #convert(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  static #convert<T>(x: T): T { return x; }

  static run(): void {
    print(Converter.#convert<string>("hello"));
    print(Converter.#convert<boolean>(true));
  }
}

print("generic mix");
// CHECK-LABEL: generic mix
Converter.run();
// CHECK-NEXT: hello
// CHECK-NEXT: true
