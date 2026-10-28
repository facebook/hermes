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

// In ES a write through 'super' creates an own property on the receiver,
// leaving the superclass alone. Statics live in Variables, which the subclass
// shares with the superclass, so the write can't be given ES semantics and is
// rejected. Reads and accessor writes are unaffected.
class D {
  static f: number = 1;
  static _a: number = 2;
  @Hermes.final static get a(): number { return D._a; }
  @Hermes.final static set a(n: number) { D._a = n; }
}
class E extends D {
  static viaInit: number = ((): number => { super.f = 5; return 1; })();
  static viaMethod(): void { super.f = 6; }
  static viaCompound(): void { super.f += 1; }
  static viaUpdate(): void { super.f++; }
  // Allowed: the setter is called with the receiver as 'this', as in ES.
  static viaAccessor(): void { super.a = 7; }
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
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:44:51: error: ft: cannot write to static property f through 'super'
// CHECK-NEXT:  static viaInit: number = ((): number => { super.f = 5; return 1; })();
// CHECK-NEXT:                                                  ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:45:36: error: ft: cannot write to static property f through 'super'
// CHECK-NEXT:  static viaMethod(): void { super.f = 6; }
// CHECK-NEXT:                                   ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:46:38: error: ft: cannot write to static property f through 'super'
// CHECK-NEXT:  static viaCompound(): void { super.f += 1; }
// CHECK-NEXT:                                     ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:47:36: error: ft: cannot write to static property f through 'super'
// CHECK-NEXT:  static viaUpdate(): void { super.f++; }
// CHECK-NEXT:                                   ^
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:55:33: error: ft: 'new this()' is not supported in static members, use the class name
// CHECK-NEXT:  static make(): N { return new this(); }
// CHECK-NEXT:                                ^~~~
// CHECK-NEXT:{{.*}}class-static-receiver-error.js:56:49: error: ft: 'new this()' is not supported in static members, use the class name
// CHECK-NEXT:  static viaArrow(): N { const f = (): N => new this(); return f(); }
// CHECK-NEXT:                                                ^~~~
// CHECK-NEXT:Emitted 11 errors. exiting.
