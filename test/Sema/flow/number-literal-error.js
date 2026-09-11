/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

type Circle = {tag: 1, radius: number};
type Square = {tag: 2, side: number};
type Shape = Circle | Square;

// A different numeric literal does not flow into the annotated literal type.
let bad: 1 = 2;

// 'number' does not flow into a numeric literal type.
let num: number = 5;
let lit: 5 = num;

// An object whose tag matches no arm does not flow into the union.
let shape: Shape = {tag: 3, radius: 1};

// ++/-- write the result back, so they are rejected on a literal-typed
// operand: the widened value would violate the literal type.
let n: 1 = 1;
n++;
let m: 2 = 2;
--m;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}number-literal-error.js:17:5: error: ft: incompatible initialization type: cannot assign 2 to 1
// CHECK-NEXT:let bad: 1 = 2;
// CHECK-NEXT:    ^~~~~~~~~~
// CHECK-NEXT:{{.*}}number-literal-error.js:21:5: error: ft: incompatible initialization type: cannot assign number to 5
// CHECK-NEXT:let lit: 5 = num;
// CHECK-NEXT:    ^~~~~~~~~~~~
// CHECK-NEXT:{{.*}}number-literal-error.js:24:5: error: ft: incompatible initialization type: cannot assign object to union Shape
// CHECK-NEXT:let shape: Shape = {tag: 3, radius: 1};
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}number-literal-error.js:29:1: error: ft: update expression must be number or bigint
// CHECK-NEXT:n++;
// CHECK-NEXT:^~~
// CHECK-NEXT:{{.*}}number-literal-error.js:31:1: error: ft: update expression must be number or bigint
// CHECK-NEXT:--m;
// CHECK-NEXT:^~~
// CHECK-NEXT:Emitted 5 errors. exiting.
