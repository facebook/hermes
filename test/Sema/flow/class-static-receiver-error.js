/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A static member runs with the class as its receiver, so instance members are
// not reachable through 'this'. Arrows inherit that receiver.
class A {
  p: number = 1;
  static viaMethod(): number { return this.p; }
  static viaArrow(): number {
    const f = (): number => this.p;
    return f();
  }
  static viaInit: number = this.p;
}

// Likewise 'super' in a static context is the superclass constructor, so it
// resolves against the superclass's statics rather than its prototype.
class B {
  q: number = 3;
  m(): number { return 9; }
}
class C extends B {
  static viaInit: number = super.m();
  static viaMethod(): number { return super.q; }
}

// In ES a write to an inherited static data property creates an own property
// on the receiver, leaving the declaring class alone. Statics live in
// Variables, which subclasses share with the declaring class, so writes are
// only allowed through the declaring class name, where the receiver is exact.
// Reads and accessor writes are unaffected.
class D {
  static f: number = 1;
  static _a: number = 2;
  @Hermes.final static get a(): number { return D._a; }
  @Hermes.final static set a(n: number) { D._a = n; }
  static viaThis(): void { this.f = 3; }
  // Allowed: the receiver is exactly D.
  static viaName(): void { D.f = 4; }
}
class E extends D {
  static viaInit: number = ((): number => { super.f = 5; return 1; })();
  static viaMethod(): void { super.f = 6; }
  static viaCompound(): void { super.f += 1; }
  static viaUpdate(): void { super.f++; }
  static viaSubclassName(): void { E.f = 7; }
  // Allowed: the setter is called with the receiver as 'this', as in ES.
  static viaAccessor(): void { super.a = 7; }
}
E.f = 8;
// Only the delete error is reported, not the write rule as well.
delete E.f;

// ES checks that the receiver itself has a private static, so a subclass
// receiver throws. Private statics are therefore only reachable through the
// declaring class name.
class P {
  static #s: number = 1;
  static viaThis(): number { return this.#s; }
  static viaThisWrite(): void { this.#s = 2; }
  // Allowed.
  static viaName(): number { return P.#s; }
  static #m(): number { return 1; }
  @Hermes.final static get #a(): number { return 2; }
  static viaThisMethod(): number { return this.#m(); }
  static viaThisAccessor(): number { return this.#a; }
}

// 'new this()' in a static member would construct the declaring class even
// when 'this' is a subclass, so it is rejected. Instance members are fine.
class N {
  static make(): N { return new this(); }
  static viaArrow(): N { const f = (): N => new this(); return f(); }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-static-receiver-error.js:14:44: error: ft: static property p not defined in class A
// CHECK-NEXT:  static viaMethod(): number { return this.p; }
// CHECK-NEXT:                                           ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:16:34: error: ft: static property p not defined in class A
// CHECK-NEXT:    const f = (): number => this.p;
// CHECK-NEXT:                                 ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:19:33: error: ft: static property p not defined in class A
// CHECK-NEXT:  static viaInit: number = this.p;
// CHECK-NEXT:                                ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:29:34: error: ft: static property m not defined in class B
// CHECK-NEXT:  static viaInit: number = super.m();
// CHECK-NEXT:                                 ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:30:45: error: ft: static property q not defined in class B
// CHECK-NEXT:  static viaMethod(): number { return super.q; }
// CHECK-NEXT:                                            ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:43:33: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaThis(): void { this.f = 3; }
// CHECK-NEXT:                                ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:48:51: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaInit: number = ((): number => { super.f = 5; return 1; })();
// CHECK-NEXT:                                                  ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:49:36: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaMethod(): void { super.f = 6; }
// CHECK-NEXT:                                   ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:50:38: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaCompound(): void { super.f += 1; }
// CHECK-NEXT:                                     ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:51:36: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaUpdate(): void { super.f++; }
// CHECK-NEXT:                                   ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:52:38: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:  static viaSubclassName(): void { E.f = 7; }
// CHECK-NEXT:                                     ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:56:3: error: ft: static property f can only be written through the name of its declaring class
// CHECK-NEXT:E.f = 8;
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:58:1: error: ft: cannot delete property of typed object
// CHECK-NEXT:delete E.f;
// CHECK-NEXT:^~~~~~~~~~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:65:42: error: ft: private static #s can only be accessed through the name of its declaring class
// CHECK-NEXT:  static viaThis(): number { return this.#s; }
// CHECK-NEXT:                                         ^~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:66:38: error: ft: private static #s can only be accessed through the name of its declaring class
// CHECK-NEXT:  static viaThisWrite(): void { this.#s = 2; }
// CHECK-NEXT:                                     ^~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:71:48: error: ft: private static #m can only be accessed through the name of its declaring class
// CHECK-NEXT:  static viaThisMethod(): number { return this.#m(); }
// CHECK-NEXT:                                               ^~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:72:50: error: ft: private static #a can only be accessed through the name of its declaring class
// CHECK-NEXT:  static viaThisAccessor(): number { return this.#a; }
// CHECK-NEXT:                                                 ^~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:78:33: error: ft: 'new this()' is not supported in static members, use the class name
// CHECK-NEXT:  static make(): N { return new this(); }
// CHECK-NEXT:                                ^~~~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:79:49: error: ft: 'new this()' is not supported in static members, use the class name
// CHECK-NEXT:  static viaArrow(): N { const f = (): N => new this(); return f(); }
// CHECK-NEXT:                                                ^~~~
// CHECK-NEXT:Emitted 19 errors. exiting.
