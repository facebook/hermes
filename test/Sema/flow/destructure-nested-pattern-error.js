/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

let tup: [number, string] = [1, 'a'];
let arr: Array<number> = [1];
let obj: {p: number} = {p: 1};
let nested: {p: {q: number}} = {p: {q: 1}};
let optNested: {p: ?{q: number}} = {p: null};
let optQ: ?{q: number} = null;
let n: number = 0;
let s: string = '';

// Legal: the nested pattern matches the type of the element it receives.
let deep: [string, [number]] = ['a', [1]];
[s, [n]] = deep;
({p: {q: n}} = nested);

// Legal: a hole consumes a tuple element without binding anything.
[, s] = tup;

// The source type of a nested pattern must be destructurable by it.
[n, [s]] = tup;
({p: {q: n}} = obj);
[n, [s]] = arr;

// An optional source can only be narrowed for the outermost pattern, where the
// assignment casts its RHS. A nested one has no expression to cast.
({p: {q: n}} = optNested);

// A default replaces the source value, so it must be assignable to the target
// even when the target is a nested pattern.
({p: {q: n} = 5} = nested);

// A pattern target needs an exact flow: nothing narrows the default before it
// is destructured.
({p: {q: n} = optQ} = nested);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}destructure-nested-pattern-error.js:30:5: error: ft: incompatible type for array pattern, expected tuple or array
// CHECK-NEXT:[n, [s]] = tup;
// CHECK-NEXT:    ^~~
// CHECK-NEXT:{{.*}}destructure-nested-pattern-error.js:31:6: error: ft: incompatible type for object pattern, expected object type
// CHECK-NEXT:({p: {q: n}} = obj);
// CHECK-NEXT:     ^~~~~~
// CHECK-NEXT:{{.*}}destructure-nested-pattern-error.js:32:5: error: ft: incompatible type for array pattern, expected tuple or array
// CHECK-NEXT:[n, [s]] = arr;
// CHECK-NEXT:    ^~~
// CHECK-NEXT:{{.*}}destructure-nested-pattern-error.js:36:6: error: ft: incompatible type for object pattern, expected object type
// CHECK-NEXT:({p: {q: n}} = optNested);
// CHECK-NEXT:     ^~~~~~
// CHECK-NEXT:{{.*}}destructure-nested-pattern-error.js:40:15: error: ft: incompatible destructuring default type: cannot implicitly cast from 5 to object
// CHECK-NEXT:({p: {q: n} = 5} = nested);
// CHECK-NEXT:              ^
// CHECK-NEXT:{{.*}}destructure-nested-pattern-error.js:44:15: error: ft: incompatible destructuring default type: cannot implicitly cast from (void | null | object) to object
// CHECK-NEXT:({p: {q: n} = optQ} = nested);
// CHECK-NEXT:              ^~~~
// CHECK-NEXT:Emitted 6 errors. exiting.
