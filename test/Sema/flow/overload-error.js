/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// @Hermes.overload without @Hermes.final.
class A {
  @Hermes.overload
  foo(x: number): number { return x; }
}

// @Hermes.overload on a constructor.
class B {
  @Hermes.overload
  constructor() {}
}

// @Hermes.overload on a getter.
class D {
  @Hermes.final @Hermes.overload
  get x(): number { return 0; }
}

// @Hermes.overload on a setter.
class E {
  @Hermes.final @Hermes.overload
  set x(v: number) {}
}

// Partial @Hermes.overload: one method has it, the other does not.
class G {
  @Hermes.final @Hermes.overload
  baz(x: number): number { return x; }
  @Hermes.final
  baz(x: string): string { return x; }
}

// Subclass shadowing a parent's overloaded final method with a single
// non-overloaded method. Caught by the final-override check before the
// override-type check (which would null-deref on parent.foo->type, since
// overloaded fields have type==nullptr).
class HParent {
  @Hermes.final @Hermes.overload
  foo(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  foo(x: string): string { return x; }
}
class HChild extends HParent {
  foo(x: boolean): boolean { return x; }
}

// Subclass adding its own overload with the same name as a parent's
// overloaded final method. Same reason — caught by final-override check.
class IParent {
  @Hermes.final @Hermes.overload
  foo(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  foo(x: string): string { return x; }
}
class IChild extends IParent {
  @Hermes.final @Hermes.overload
  foo(x: boolean): boolean { return x; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}overload-error.js:12:3: error: ft: @Hermes.overload requires @Hermes.final
// CHECK-NEXT:  @Hermes.overload
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}overload-error.js:18:3: error: ft: @Hermes.overload cannot be applied to a constructor
// CHECK-NEXT:  @Hermes.overload
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}overload-error.js:24:3: error: ft: @Hermes.overload cannot be applied to getters/setters
// CHECK-NEXT:  @Hermes.final @Hermes.overload
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}overload-error.js:30:3: error: ft: @Hermes.overload cannot be applied to getters/setters
// CHECK-NEXT:  @Hermes.final @Hermes.overload
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}overload-error.js:39:3: error: ft: all overloads of baz must be decorated with @Hermes.overload
// CHECK-NEXT:  baz(x: string): string { return x; }
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}overload-error.js:53:3: error: ft: cannot override final method
// CHECK-NEXT:  foo(x: boolean): boolean { return x; }
// CHECK-NEXT:  ^~~
// CHECK-NEXT:{{.*}}overload-error.js:66:3: error: ft: cannot override final method
// CHECK-NEXT:  foo(x: boolean): boolean { return x; }
// CHECK-NEXT:  ^~~
// CHECK-NEXT:Emitted 7 errors. exiting.
