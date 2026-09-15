/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Type-based overload resolution on a private method.
class Foo {
  @Hermes.final @Hermes.overload
  #bar(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  #bar(x: string): string { return x + x; }

  test(): void {
    print(this.#bar(21));
    print(this.#bar("ab"));
  }
}

print("type-based");
// CHECK-LABEL: type-based
let f = new Foo();
f.test();
// CHECK-NEXT: 42
// CHECK-NEXT: abab

// Arity-based overload resolution on a private method.
class Multi {
  @Hermes.final @Hermes.overload
  #calc(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  #calc(x: number, y: number): number { return x + y; }
  @Hermes.final @Hermes.overload
  #calc(x: number, y: number, z: number): number { return x + y + z; }

  run(): void {
    print(this.#calc(1));
    print(this.#calc(1, 2));
    print(this.#calc(1, 2, 3));
  }
}

print("arity");
// CHECK-LABEL: arity
let m = new Multi();
m.run();
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 6

// Mixed generic and non-generic private overloads.
class Converter {
  @Hermes.final @Hermes.overload
  #convert(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  #convert<T>(x: T): T { return x; }

  run(): void {
    print(this.#convert<string>("hello"));
    print(this.#convert<boolean>(true));
  }
}

print("generic mix");
// CHECK-LABEL: generic mix
let c = new Converter();
c.run();
// CHECK-NEXT: hello
// CHECK-NEXT: true
