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

  get x(): number { return this._x; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}getter-no-final-error.js:16:3: error: ft: getters must be marked as @Hermes.final
// CHECK-NEXT:  get x(): number { return this._x; }
// CHECK-NEXT:  ^
// CHECK-NEXT:Emitted 1 errors. exiting.
