/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A multi-level generic alias chain must resolve to its concrete type, both when
// used as a variable type and as a bound. If it silently resolved to 'any'
// (which the populateTypeAlias fix guards against) these uses would be wrongly
// accepted, so the errors here guard that unsoundness.

class Base {}
class Other {}
class Derived extends Base {}

type Id<X> = X;
type Wrap<Y> = Id<Y>;
type Wrap2<Z> = Wrap<Z>;

// Wrap2<Base> is Base, so assigning Other must fail.
let c: Wrap2<Base> = new Other();

// The chain used as a bound is Base, so Other must fail and Derived must pass.
function f<T: Wrap2<Base>>(x: T): void {}
f<Other>(new Other());
f<Derived>(new Derived());

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-alias-chain-error.js:24:5: error: ft: incompatible initialization type: cannot assign class Other to class
// CHECK-NEXT:let c: Wrap2<Base> = new Other();
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-alias-chain-error.js:28:2: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:f<Other>(new Other());
// CHECK-NEXT: ^~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
