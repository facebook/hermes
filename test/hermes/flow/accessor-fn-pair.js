/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

// A getter/setter pair whose type is a function type: each annotation creates
// a distinct Type, so the pair must be matched structurally.
class C {
  _f: (x: number) => number;
  constructor() {
    this._f = (x: number): number => x;
  }
  @Hermes.final
  get f(): (x: number) => number {
    return this._f;
  }
  @Hermes.final
  set f(v: (x: number) => number) {
    this._f = v;
  }
}

const c = new C();
c.f = (x: number): number => x * 2;
print(c.f(21));
// CHECK: 42
