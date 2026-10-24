/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Bounds resolve type parameters, not outer types of the same name.

type T = any;
type B = any;

class Cmp<T> {
  x: T | null = null;
}
function f<T: Cmp<T>>(x: T): void {}
f<number>(1);

function g<A: B, B>(x: A, y: B): void {}
g<number, string>(1, 'a');

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-outer-shadow-error.js:19:2: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:f<number>(1);
// CHECK-NEXT: ^~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-outer-shadow-error.js:22:2: error: ft: type argument for type parameter 'A' is incompatible with its bound
// CHECK-NEXT:g<number, string>(1, 'a');
// CHECK-NEXT: ^~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
