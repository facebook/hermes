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

'use strict';

// In a static field initializer 'this' is the class itself.
class A {
  x: number = 3;
  static w: number = 7;
  static self: any = this;
  static viaThis: any = () => this.w;
  static inst: A = new A();
}

print(A.self === A, A.viaThis(), A.inst.x);
// CHECK: true 7 3

// Only arrows inherit the initializer's 'this'; a nested non-arrow function
// still takes it from its receiver.
class E {
  static fn: any = function () {
    return this;
  };
}

print(E.fn.call(42));
// CHECK-NEXT: 42

// new.target is undefined even when the class is evaluated inside a
// constructor with a non-undefined new.target.
class Outer {
  constructor() {
    class Inner {
      static directNT: any = new.target;
      static arrowNT: any = () => new.target;
    }
    print(Inner.directNT, Inner.arrowNT());
  }
}

new Outer();
// CHECK-NEXT: undefined undefined

// A static method's receiver is the class too, so its static members --
// including private ones and other static methods -- are reachable through
// 'this'.
class M {
  static w: number = 7;
  static #s: number = 3;
  static one(): number {
    return 1;
  }
  static all(): string {
    return `${this.w},${this.#s},${this.one()}`;
  }
  // Arrows in a static method body inherit that receiver.
  static viaArrow(): number {
    const f = (): number => this.w + this.#s;
    return f();
  }
  // Writes through the class name are visible through 'this'.
  static bump(): number {
    M.w++;
    return this.w;
  }
}

print(M.all(), M.viaArrow());
// CHECK-NEXT: 7,3,1 10
print(M.bump(), M.w);
// CHECK-NEXT: 8 8

// 'this' is the receiver of the call, so an inherited static method invoked on
// the subclass sees the subclass, matching ordinary JS static dispatch.
// Member resolution through 'this' is static, though: statics live in
// variables and are picked at compile time. 'new this()' would construct the
// declaring class rather than the receiver, so it is rejected; see
// class-static-receiver-error.js.
class Base {
  static who(): any {
    return this;
  }
  static name2(): string {
    return 'Base';
  }
  static describe(): string {
    return this.name2();
  }
}
class Sub extends Base {}

print(Base.who() === Base, Sub.who() === Sub);
// CHECK-NEXT: true true
print(Base.describe(), Sub.describe());
// CHECK-NEXT: Base Base
