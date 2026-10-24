/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -typed -dump-sema %s > /dev/null

// An alias whose RHS is another alias instantiation forms a "chain". Each link
// must resolve down to the final concrete type, not an intermediate generic.

class Base {}
class Derived extends Base {
  x: number = 1;
}

// One-level chain (baseline): Id<Base> resolves to Base.
type Id<X> = X;
let a: Id<Base> = new Derived();

// Two-level chain: Wrap<Base> resolves through Id<Base> to Base.
type Wrap<Y> = Id<Y>;
let b: Wrap<Base> = new Derived();

// Three-level chain, to confirm arbitrary depth resolves.
type Wrap2<Z> = Wrap<Z>;
let c: Wrap2<Base> = new Derived();
