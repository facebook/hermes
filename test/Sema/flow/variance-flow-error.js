/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// ReadOnly/WriteOnly cannot flow into invariant because that would
// require regaining a capability the source has given up.
function f2(a: {+x: number}): {x: number} {
  return a;
}
function f4(a: {-x: number}): {x: number} {
  return a;
}
// ReadOnly and WriteOnly are mutually incompatible.
function f5(a: {+x: number}): {-x: number} {
  return a;
}

// Wrong direction for covariant +: number-or-string cannot flow into number.
function f6(a: {+x: number | string}): {+x: number} {
  return a;
}
// Wrong direction for contravariant -: number cannot flow into number-or-string.
function f7(a: {-x: number}): {-x: number | string} {
  return a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}variance-flow-error.js:13:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return a;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}variance-flow-error.js:16:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return a;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}variance-flow-error.js:20:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return a;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}variance-flow-error.js:25:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return a;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}variance-flow-error.js:29:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return a;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:Emitted 5 errors. exiting.
