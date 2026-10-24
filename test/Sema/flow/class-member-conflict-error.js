/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

// Fields, methods and accessors share a single name namespace. Fields live in
// instance slots and methods in home object slots, so allowing both would
// create two independent storage locations for the same name.

// Field then method.
class A1 {
  foo: number = 1;
  foo(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:17:3: error: ft: method foo already declared
// CHECK: {{.*}}class-member-conflict-error.js:16:3: note: ft: previous declaration of foo

// Method then field.
class A2 {
  foo(): number { return 2; }
  foo: number = 1;
}
// CHECK: {{.*}}class-member-conflict-error.js:25:3: error: ft: field foo already declared
// CHECK: {{.*}}class-member-conflict-error.js:24:3: note: ft: previous declaration of foo

// Field then accessor.
class A3 {
  x: number = 1;
  @Hermes.final
  get x(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:34:7: error: ft: method x already declared
// CHECK: {{.*}}class-member-conflict-error.js:32:3: note: ft: previous declaration of x

// Accessor then field.
class A4 {
  @Hermes.final
  get x(): number { return 2; }
  x: number = 1;
}
// CHECK: {{.*}}class-member-conflict-error.js:43:3: error: ft: field x already declared
// CHECK: {{.*}}class-member-conflict-error.js:41:3: note: ft: previous declaration of x

function g(): number { return 3; }

// Field shadowing an inherited method.
class B1 { foo(): number { return 1; } }
class C1 extends B1 { foo: () => number = g; }
// CHECK: {{.*}}class-member-conflict-error.js:52:23: error: ft: cannot override method foo with a field

// Field shadowing an inherited final method.
class B2 { @Hermes.final foo(): number { return 1; } }
class C2 extends B2 { foo: () => number = g; }
// CHECK: {{.*}}class-member-conflict-error.js:57:23: error: ft: cannot override method foo with a field

// Field shadowing an inherited accessor.
class B3 { @Hermes.final get x(): number { return 1; } }
class C3 extends B3 { x: number = 2; }
// CHECK: {{.*}}class-member-conflict-error.js:62:23: error: ft: cannot override method x with a field

// Method shadowing an inherited field.
class B4 { foo: number = 1; }
class C4 extends B4 { foo(): number { return 2; } }
// CHECK: {{.*}}class-member-conflict-error.js:67:23: error: ft: cannot override field foo with a method

// Accessor shadowing an inherited field.
class B5 { x: number = 1; }
class C5 extends B5 {
  @Hermes.final
  get x(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:74:7: error: ft: cannot override field x with a method

// Statics share a single namespace with each other (but not with instance
// members), so the same conflicts apply among them.

// Static field then static method.
class S1 {
  static foo: number = 1;
  static foo(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:84:10: error: ft: method foo already declared
// CHECK: {{.*}}class-member-conflict-error.js:83:3: note: ft: previous declaration of foo

// Static method then static field.
class S2 {
  static foo(): number { return 2; }
  static foo: number = 1;
}
// CHECK: {{.*}}class-member-conflict-error.js:92:10: error: ft: field foo already declared
// CHECK: {{.*}}class-member-conflict-error.js:91:3: note: ft: previous declaration of foo

// Static field then static accessor.
class S3 {
  static x: number = 1;
  @Hermes.final
  static get x(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:101:14: error: ft: method x already declared
// CHECK: {{.*}}class-member-conflict-error.js:99:3: note: ft: previous declaration of x

// Static accessor then static field.
class S4 {
  @Hermes.final
  static get x(): number { return 2; }
  static x: number = 1;
}
// CHECK: {{.*}}class-member-conflict-error.js:110:10: error: ft: field x already declared
// CHECK: {{.*}}class-member-conflict-error.js:108:3: note: ft: previous declaration of x

// Static field shadowing an inherited static method.
class D1 { static foo(): number { return 1; } }
class E1 extends D1 { static foo: () => number = g; }
// CHECK: {{.*}}class-member-conflict-error.js:117:30: error: ft: cannot override method foo with a field

// Static field shadowing an inherited static accessor.
class D2 { @Hermes.final static get x(): number { return 1; } }
class E2 extends D2 { static x: number = 2; }
// CHECK: {{.*}}class-member-conflict-error.js:122:30: error: ft: cannot override method x with a field

// Static field shadowing an inherited overloaded static method, whose Field
// has a null type.
class D3 {
  @Hermes.final @Hermes.overload static f(x: number): number { return x; }
  @Hermes.final @Hermes.overload static f(x: string): string { return x; }
}
class E3 extends D3 { static f: number = 1; }
// CHECK: {{.*}}class-member-conflict-error.js:131:30: error: ft: cannot override method f with a field

// Static method shadowing an inherited static field.
class D4 { static foo: number = 1; }
class E4 extends D4 { static foo(): number { return 2; } }
// CHECK: {{.*}}class-member-conflict-error.js:136:30: error: ft: cannot override field foo with a method

// Static accessor shadowing an inherited static field.
class D5 { static x: number = 1; }
class E5 extends D5 {
  @Hermes.final
  static get x(): number { return 2; }
}
// CHECK: {{.*}}class-member-conflict-error.js:143:14: error: ft: cannot override field x with a method

// A static field conflicting with an overloaded static method is a name
// conflict, not a problem with the @Hermes.overload decoration.
class F1 {
  static f: number = 1;
  @Hermes.final @Hermes.overload static f(x: number): number { return x; }
}
// CHECK: {{.*}}class-member-conflict-error.js:151:41: error: ft: method f already declared
// CHECK: {{.*}}class-member-conflict-error.js:150:3: note: ft: previous declaration of f

// An instance field override must keep the inherited field's type.
class D8 { x: number = 1; }
class E8 extends D8 { x: string = "a"; }
// CHECK: {{.*}}class-member-conflict-error.js:[[@LINE-1]]:23: error: ft: incompatible field type for override

// Anchor the total so unrelated diagnostics can't slip in unnoticed.
// CHECK: Emitted 20 errors. exiting.
