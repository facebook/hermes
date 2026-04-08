/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

class Foo {
  _x: number;
  constructor() { this._x = 0; }
  @Hermes.final
  get x(): number { return this._x; }
  @Hermes.final
  set x(v: string): void { this._x = 0; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}setter-type-mismatch-error.js:18:7: error: ft: setter parameter type must match getter return type
// CHECK-NEXT:  set x(v: string): void { this._x = 0; }
// CHECK-NEXT:      ^
// CHECK-NEXT:Emitted 1 errors. exiting.
