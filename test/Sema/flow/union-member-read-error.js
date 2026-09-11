/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// The field is not present in every arm.
function missing(v: {x: number} | {y: number}): number {
  return v.x;
}

// One of the arms is not an object type.
function nonObject(v: {tag: 'a'} | number): string {
  return v.tag;
}

// Writing through a union field is not supported.
function write(v: {tag: 'a'} | {tag: 'b'}): void {
  v.tag = 'a';
}

// Compound assignment is also a write and is rejected.
function compound(v: {n: number} | {n: number, extra: number}): void {
  v.n += 1;
}

// Increment/decrement is a write and is rejected.
function update(v: {n: number} | {n: number, extra: number}): void {
  v.n++;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}union-member-read-error.js:14:12: error: ft: property 'x' is not present in all arms of the union
// CHECK-NEXT:  return v.x;
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}union-member-read-error.js:19:12: error: ft: property 'tag' cannot be read on union with non-object arm number
// CHECK-NEXT:  return v.tag;
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}union-member-read-error.js:24:5: error: ft: cannot write to a property of a union
// CHECK-NEXT:  v.tag = 'a';
// CHECK-NEXT:    ^~~
// CHECK-NEXT:{{.*}}union-member-read-error.js:29:5: error: ft: cannot write to a property of a union
// CHECK-NEXT:  v.n += 1;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}union-member-read-error.js:34:5: error: ft: cannot write to a property of a union
// CHECK-NEXT:  v.n++;
// CHECK-NEXT:    ^
// CHECK-NEXT:Emitted 5 errors. exiting.
