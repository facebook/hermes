/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// Members that legitimately share a name must keep working: getter/setter
// pairs, overload sets, and same-kind overrides.

class GetSet {
  n: number = 0;
  @Hermes.final
  get x(): number { return this.n; }
  @Hermes.final
  set x(v: number) { this.n = v; }
}
let gs = new GetSet();
gs.x = 5;
print(gs.x);
// CHECK: 5

class Overloads {
  @Hermes.final @Hermes.overload
  f(x: number): number { return x + 1; }
  @Hermes.final @Hermes.overload
  f(x: string): string { return x + "!"; }
}
let ov = new Overloads();
print(ov.f(1), ov.f("s"));
// CHECK-NEXT: 2 s!

class FieldBase { x: number = 1; }
class FieldDerived extends FieldBase { x: number = 2; }
let fd = new FieldDerived();
let fb: FieldBase = fd;
fb.x = 5;
print(fb.x, fd.x);
// CHECK-NEXT: 5 5

class MethodBase { foo(): number { return 1; } }
class MethodDerived extends MethodBase { foo(): number { return 2; } }
let md = new MethodDerived();
let mb: MethodBase = md;
print(mb.foo(), md.foo());
// CHECK-NEXT: 2 2

// Static and instance members are separate namespaces.
class Mixed {
  static foo: number = 1;
  foo(): number { return 2; }
}
print(Mixed.foo, new Mixed().foo());
// CHECK-NEXT: 1 2

// Private names are distinct from the identically spelled public ones, so
// they don't conflict with fields or methods.
class Priv {
  #foo: number;
  constructor() { this.#foo = 1; }
  foo(): number { return this.#foo; }
  static #sfoo: number = 10;
  static sfoo(): number { return Priv.#sfoo; }
}
print(new Priv().foo(), Priv.sfoo());
// CHECK-NEXT: 1 10

class StaticGetSet {
  static n: number = 0;
  @Hermes.final
  static get x(): number { return StaticGetSet.n; }
  @Hermes.final
  static set x(v: number) { StaticGetSet.n = v; }
}
StaticGetSet.x = 7;
print(StaticGetSet.x);
// CHECK-NEXT: 7
