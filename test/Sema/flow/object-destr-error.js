/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

let obj: {x: number, y: string} = {x: 1, y: 'hello'};

// Property not found
{
let {z} = obj;
}

// Incompatible type for object pattern
{
let tup: [number, string] = [1, 'hi'];
let {x} = tup;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-destr-error.js:16:6: error: ft: property 'z' not found in object type
// CHECK-NEXT:let {z} = obj;
// CHECK-NEXT:     ^
// CHECK-NEXT:{{.*}}object-destr-error.js:22:5: error: ft: incompatible type for object pattern, expected object type
// CHECK-NEXT:let {x} = tup;
// CHECK-NEXT:    ^~~
// CHECK-NEXT:Emitted 2 errors. exiting.
