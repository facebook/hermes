/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

class Foo {
  _x: number;
  constructor() { this._x = 0; }

  @Hermes.final
  get x(): number { return this._x; }
}

let f = new Foo();

// Assignment to a getter property.
f.x = 5;

// Compound assignment to a getter property.
f.x += 1;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}getter-error.js:23:1: error: ft: cannot assign to getter-only property
// CHECK-NEXT:f.x = 5;
// CHECK-NEXT:^~~~~~~
// CHECK-NEXT:{{.*}}getter-error.js:26:1: error: ft: cannot assign to getter-only property
// CHECK-NEXT:f.x += 1;
// CHECK-NEXT:^~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
