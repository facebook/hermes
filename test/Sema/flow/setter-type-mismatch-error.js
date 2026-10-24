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
  @Hermes.final
  set x(v: string): void { this._x = 0; }
}

// A setter's return value is discarded, so it can only return void.
class Bar {
  @Hermes.final
  set x(v: number): number { return v; }
  @Hermes.final
  static set y(v: number): string { return 'a'; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}setter-type-mismatch-error.js:18:7: error: ft: setter parameter type must match getter return type
// CHECK-NEXT:  set x(v: string): void { this._x = 0; }
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}setter-type-mismatch-error.js:24:19: error: ft: setter return type must be void
// CHECK-NEXT:  set x(v: number): number { return v; }
// CHECK-NEXT:                  ^~~~~~~~
// CHECK-NEXT:{{.*}}setter-type-mismatch-error.js:26:26: error: ft: setter return type must be void
// CHECK-NEXT:  static set y(v: number): string { return 'a'; }
// CHECK-NEXT:                         ^~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
