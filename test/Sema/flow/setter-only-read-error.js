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
  set x(v: number): void { this._x = v; }
}
let f = new Foo();
let y: number = f.x;

// Compound assignment on setter-only also reads the property.
f.x += 1;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}setter-only-read-error.js:19:5: error: ft: incompatible initialization type: cannot assign void to number
// CHECK-NEXT:let y: number = f.x;
// CHECK-NEXT:    ^~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}setter-only-read-error.js:19:19: error: ft: cannot read setter-only property
// CHECK-NEXT:let y: number = f.x;
// CHECK-NEXT:                  ^
// CHECK-NEXT:{{.*}}setter-only-read-error.js:22:3: error: ft: cannot read setter-only property
// CHECK-NEXT:f.x += 1;
// CHECK-NEXT:  ^
// CHECK-NEXT:Emitted 3 errors. exiting.
