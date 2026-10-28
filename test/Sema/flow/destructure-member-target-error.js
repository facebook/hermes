/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Destructuring into a typed member expression is narrowed at runtime when the
// source type is unknown. A typed for-of source has a statically known element
// type, so reject a mismatch before runtime.

let nums: number[] = [1];
let so: {s: string} = {s: ''};
let src: any = null;
let records: {p: number}[] = [{p: 1}];
let optional: {p: string | void} = {p: undefined};

// Legal: the store narrows at runtime.
({p: so.s} = src);

({p: so.s} = {p: 1});
for (so.s of nums) {}
for ({p: so.s} of records) {}
for ({p: so.s = ''} of records) {}
// The default is stored into the target, so it is checked against it.
({p: so.s = 1} = optional);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}destructure-member-target-error.js:23:6: error: ft: incompatible object destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:({p: so.s} = {p: 1});
// CHECK-NEXT:     ^~~~
// CHECK-NEXT:{{.*}}destructure-member-target-error.js:24:6: error: ft: incompatible for-of element type: cannot implicitly cast from number to string
// CHECK-NEXT:for (so.s of nums) {}
// CHECK-NEXT:     ^~~~
// CHECK-NEXT:{{.*}}destructure-member-target-error.js:25:10: error: ft: incompatible object destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:for ({p: so.s} of records) {}
// CHECK-NEXT:         ^~~~
// CHECK-NEXT:{{.*}}destructure-member-target-error.js:26:10: error: ft: incompatible object destructuring target type: cannot implicitly cast from number to string
// CHECK-NEXT:for ({p: so.s = ''} of records) {}
// CHECK-NEXT:         ^~~~~~~~~
// CHECK-NEXT:{{.*}}destructure-member-target-error.js:28:13: error: ft: incompatible destructuring default type: cannot implicitly cast from 1 to string
// CHECK-NEXT:({p: so.s = 1} = optional);
// CHECK-NEXT:            ^
// CHECK-NEXT:Emitted 5 errors. exiting.
