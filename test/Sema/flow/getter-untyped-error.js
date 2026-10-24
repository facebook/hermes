/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// An untyped getter has type 'any', which doesn't match a typed setter.
class Foo {
  @Hermes.final
  set x(v: number) {}
  @Hermes.final
  get x() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}getter-untyped-error.js:17:7: error: ft: getter return type must match setter parameter type
// CHECK-NEXT:  get x() {}
// CHECK-NEXT:      ^
// CHECK-NEXT:Emitted 1 errors. exiting.
