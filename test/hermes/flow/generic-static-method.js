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

class Utils {
  constructor() {}

  @Hermes.final
  static identity<T>(x: T): T {
    return x;
  }

  @Hermes.final
  static pair<T, U>(x: T, y: U): T {
    return x;
  }
}

// Test explicit type arguments.
print(Utils.identity<number>(42));
// CHECK: 42
print(Utils.identity<string>("hello"));
// CHECK-NEXT: hello

// Test type inference.
print(Utils.identity(42));
// CHECK-NEXT: 42
print(Utils.identity("hello"));
// CHECK-NEXT: hello

// Test multiple type parameters with explicit args.
print(Utils.pair<number, string>(10, "test"));
// CHECK-NEXT: 10

// A generic static method has the same receiver as any other static method, so
// 'this' reaches the class's statics and 'super' the superclass's.
class Base {
  static tag: number = 5;
  static who(): string {
    return "Base";
  }
  m(): string {
    return "Base#m";
  }
}

class Derived extends Base {
  m(): string {
    return "Derived#m";
  }
  @Hermes.final
  static viaThis<U>(x: U): number {
    return this.tag;
  }

  @Hermes.final
  static viaSuper<U>(x: U): string {
    return super.who();
  }
  @Hermes.final
  instanceGeneric<U>(x: U): string {
    return super.m();
  }
}
class Sub extends Derived {}

print(Derived.viaThis<number>(1));
// CHECK-NEXT: 5

// Statics on a *generic* class. Each specialization is compiled from a cloned
// AST, so it gets its own copy of the static storage, initialized separately.
// Inside the class, a member access on the class name refers to the current
// specialization.
let nextBoxId: number = 0;
function newBoxId(): number {
  return ++nextBoxId;
}

class Box<T> {
  static id: number = newBoxId();
  value: T;
  constructor(v: T) {
    this.value = v;
  }
  static read(): number {
    return this.id;
  }
}

class NumBox extends Box<number> {
  static viaSuper(): number {
    return super.id;
  }
}
class StrBox extends Box<string> {}

print(NumBox.read(), StrBox.read(), NumBox.viaSuper(), StrBox.id);
// CHECK-NEXT: 1 2 1 2

// 'this' and 'super' in a generic method come from the class that declares it,
// not from the receiver at the call site. Specialize from the subclass first
// to ensure the cached specialization does not inherit that receiver's class
// context.
print(Sub.viaSuper<number>(1), Derived.viaSuper<number>(1));
// CHECK-NEXT: Base Base
print(
  new Sub().instanceGeneric<number>(1),
  new Derived().instanceGeneric<number>(1),
);
// CHECK-NEXT: Base#m Base#m

// Statics, including private ones, can be read and written through the class
// name inside a generic class, from static and instance methods, arrows and
// generic methods. Each specialization has its own storage.
class Counter<T> {
  static n: number = 1;
  static #p: number = 10;
  v: T;
  constructor(v: T) {
    this.v = v;
  }
  static bump(): number {
    Counter.n++;
    Counter.#p += 5;
    return Counter.n + Counter.#p;
  }
  inst(): number {
    return Counter.n + Counter.#p;
  }
  @Hermes.final
  static gen<U>(u: U): number {
    return Counter.#p;
  }
  static arrow(): number {
    const f = (): number => Counter.#p;
    return f();
  }
}
class NumCounter extends Counter<number> {}
class StrCounter extends Counter<string> {}

print(NumCounter.bump(), NumCounter.bump(), StrCounter.bump());
// CHECK-NEXT: 17 23 17
print(new Counter<number>(1).inst(), new Counter<string>('a').inst());
// CHECK-NEXT: 23 17
print(
  NumCounter.gen<boolean>(true),
  StrCounter.gen<boolean>(true),
  NumCounter.arrow(),
);
// CHECK-NEXT: 20 15 20
