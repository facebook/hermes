/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

let arr: Array<number> = [1, 2, 3];

// Element type from Array<T> must flow into the LHS slot.
let s: string = '';
[s] = arr;

// Rest binding type must accept Array<T>.
let badRest: Array<string> = [];
let n: number = 0;
[n, ...badRest] = arr;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}array-destr-assign-error.js:16:2: error: ft: incompatible array destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:[s] = arr;
// CHECK-NEXT: ^
// CHECK-NEXT:{{.*}}array-destr-assign-error.js:21:8: error: ft: incompatible array destructuring target type: cannot implicitly cast from class Array<number> to class Array<string>
// CHECK-NEXT:[n, ...badRest] = arr;
// CHECK-NEXT:       ^~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
