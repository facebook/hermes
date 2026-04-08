/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Assign to getter-only property.
class C {
  _x: number;
  constructor() { this._x = 0; }
  @Hermes.final
  get x(): number { return this._x; }
}
let cc = new C();
cc.x = 5;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}setter-error.js:20:1: error: ft: cannot assign to getter-only property
// CHECK-NEXT:cc.x = 5;
// CHECK-NEXT:^~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
