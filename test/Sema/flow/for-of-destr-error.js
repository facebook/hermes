/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A for-of declaration takes its type from the element type of the iterable,
// so mismatches are caught statically instead of at runtime.

let tuples: [number, string][] = [[1, 'a']];
let nums: number[] = [1];
let so: {s: string} = {s: ''};
let wrapped: {p: [number]}[] = [{p: [1]}];
let str: string = '';
let anys: any[] = [];

for (const [n] of tuples) {}
for (const {p} of tuples) {}
for (const n of nums) {
  let s: string = n;
}
// An annotation on the loop variable is checked against the element type too.
for (const s: string of nums) {}
// An annotated pattern is destructured straight out of the loop value, so there
// is nowhere to hang a narrowing cast: it has to fit exactly. A bare identifier
// narrows on store instead, so 'any' is fine there.
for (const [a, b]: [number, string] of anys) {}
for (const {p}: {p: string} of anys) {}
for (const a: number of anys) {}
// The same holds when the element type isn't statically known.
let anyVal: any = 0;
for (const [c]: [number] of str) {}
for (const {p}: {p: number} of anyVal) {}
for (const c: string of str) {}

// A tuple pattern in expression position has no enclosing assignment to check
// it, so each element has to be checked against the tuple's element type.
for ([str, str] of tuples) {}
for ([so.s, so.s] of tuples) {}
// Including when the tuple pattern is nested inside another pattern.
for ({p: [so.s]} of wrapped) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}for-of-destr-error.js:20:12: error: ft: cannot destructure tuple, expected 2 elements, found 1
// CHECK-NEXT:for (const [n] of tuples) {}
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:21:12: error: ft: incompatible type for object pattern, expected object type
// CHECK-NEXT:for (const {p} of tuples) {}
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:23:7: error: ft: incompatible initialization type: cannot assign number to string
// CHECK-NEXT:  let s: string = n;
// CHECK-NEXT:      ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:26:12: error: ft: incompatible for-of element type: cannot implicitly cast from number to string
// CHECK-NEXT:for (const s: string of nums) {}
// CHECK-NEXT:           ^~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:30:12: error: ft: incompatible for-of element type: cannot implicitly cast from any to [number, string]
// CHECK-NEXT:for (const [a, b]: [number, string] of anys) {}
// CHECK-NEXT:           ^~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:31:12: error: ft: incompatible for-of element type: cannot implicitly cast from any to object
// CHECK-NEXT:for (const {p}: {p: string} of anys) {}
// CHECK-NEXT:           ^~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:35:12: error: ft: incompatible for-of element type: cannot implicitly cast from any to [number]
// CHECK-NEXT:for (const [c]: [number] of str) {}
// CHECK-NEXT:           ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:36:12: error: ft: incompatible for-of element type: cannot implicitly cast from any to object
// CHECK-NEXT:for (const {p}: {p: number} of anyVal) {}
// CHECK-NEXT:           ^~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:41:7: error: ft: incompatible tuple destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:for ([str, str] of tuples) {}
// CHECK-NEXT:      ^~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:42:7: error: ft: incompatible tuple destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:for ([so.s, so.s] of tuples) {}
// CHECK-NEXT:      ^~~~
// CHECK-NEXT:{{.*}}for-of-destr-error.js:44:11: error: ft: incompatible tuple destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:for ({p: [so.s]} of wrapped) {}
// CHECK-NEXT:          ^~~~
// CHECK-NEXT:Emitted 11 errors. exiting.
