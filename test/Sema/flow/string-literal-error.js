/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

type Circle = {tag: 'circle', radius: number};
type Square = {tag: 'square', side: number};
type Shape = Circle | Square;

// A different string literal does not flow into the annotated literal type.
let bad: 'a' = 'b';

// 'string' does not flow into a string literal type.
let str: string = 'x';
let lit: 'x' = str;

// An object whose tag matches no arm does not flow into the union.
let shape: Shape = {tag: 'triangle', radius: 1};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}string-literal-error.js:17:5: error: ft: incompatible initialization type: cannot assign "b" to "a"
// CHECK-NEXT:let bad: 'a' = 'b';
// CHECK-NEXT:    ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}string-literal-error.js:21:5: error: ft: incompatible initialization type: cannot assign string to "x"
// CHECK-NEXT:let lit: 'x' = str;
// CHECK-NEXT:    ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}string-literal-error.js:24:5: error: ft: incompatible initialization type: cannot assign object to union Shape
// CHECK-NEXT:let shape: Shape = {tag: 'triangle', radius: 1};
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
