/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A hoisted function signature doesn't hide a var re-declaration.

'use strict';

function other(n: number): number {
  return n;
}
var f = other;
function f(n: number): number {
  return n;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}func-decl-hoist-redecl-error.js:17:5: error: ft: redeclaration of f
// CHECK-NEXT:var f = other;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}func-decl-hoist-redecl-error.js:18:1: note: ft: first declaration of f
// CHECK-NEXT:function f(n: number): number {
// CHECK-NEXT:^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
