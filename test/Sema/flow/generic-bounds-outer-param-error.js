/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A generic method whose bound references the enclosing generic class's type
// parameter must resolve that bound per class instantiation. Outer<B>'s m has
// bound B, so passing A must be rejected even though Outer<A>'s m accepts A.
// This must hold regardless of the order the specializations are created.

class Base {}
class A extends Base {}
class B extends Base {}

class Outer<U: Base> {
  @Hermes.final
  m<T: U>(x: T): void {}
}

let oa: Outer<A> = new Outer<A>();
let ob: Outer<B> = new Outer<B>();

// Ok: A flows into U = A.
oa.m<A>(new A());
// Error: A does not flow into U = B.
ob.m<A>(new A());

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-outer-param-error.js:30:1: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:ob.m<A>(new A());
// CHECK-NEXT:^~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
