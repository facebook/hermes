/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Generic type parameter bounds that are violated by their type arguments.
// These are all reported during expression typechecking, so they appear
// together.

class Base {}

// Generic function: number is not a subtype of Base.
function f<T: Base>(x: T): void {}
f<number>(1);

// Generic class.
class Box<T: Base> {
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new Box<number>(1);

// Generic method.
class C {
  @Hermes.final
  m<T: Base>(x: T): void {}
}
new C().m<number>(1);

// Sibling reference: string does not satisfy the bound 'T' (= number here).
function g<T, U: T>(x: T, y: U): void {}
g<number, string>(1, 'a');

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-error.js:18:2: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:f<number>(1);
// CHECK-NEXT: ^~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-error.js:27:8: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:new Box<number>(1);
// CHECK-NEXT:       ^~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-error.js:34:1: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:new C().m<number>(1);
// CHECK-NEXT:^~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-error.js:38:2: error: ft: type argument for type parameter 'U' is incompatible with its bound
// CHECK-NEXT:g<number, string>(1, 'a');
// CHECK-NEXT: ^~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 4 errors. exiting.
