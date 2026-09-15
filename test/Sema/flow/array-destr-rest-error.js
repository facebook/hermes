/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

let t: [number, string] = [1, 'x'];

// Rest is not allowed when destructuring a tuple at declaration.
function f1(): void {
  let [a, ...rest]: [number, string] = t;
}

// Same restriction must apply to assignment-form destructuring.
function f2(): void {
  let b: number = 0;
  let brest: any;
  [b, ...brest] = t;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}array-destr-rest-error.js:16:11: error: ft: rest element not allowed when destructuring a tuple
// CHECK-NEXT:  let [a, ...rest]: [number, string] = t;
// CHECK-NEXT:          ^~~~~~~
// CHECK-NEXT:{{.*}}array-destr-rest-error.js:23:7: error: ft: rest element not allowed when destructuring a tuple
// CHECK-NEXT:  [b, ...brest] = t;
// CHECK-NEXT:      ^~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
