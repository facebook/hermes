/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A generic function can't be re-declared.

'use strict';

function g<T>(x: T): T {
  return x;
}
function g(x: number): number {
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}func-decl-generic-redecl-error.js:17:10: error: ft: redeclaration of g
// CHECK-NEXT:function g(x: number): number {
// CHECK-NEXT:         ^
// CHECK-NEXT:{{.*}}func-decl-generic-redecl-error.js:14:1: note: ft: first declaration of g
// CHECK-NEXT:function g<T>(x: T): T {
// CHECK-NEXT:^~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
