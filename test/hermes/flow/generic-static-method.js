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
// The statics are only reachable through 'this' inside the class or through a
// subclass of a specialization, since the generic class name is not a value.
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
