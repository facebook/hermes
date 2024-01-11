/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

let tup: [number, boolean] = [1, true];
let anyVar: any = tup;

{
let [v1] = tup;
let [v2, v3, v4] = tup;
}
{
let a: number;
let b: string;
[a, b] = tup;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}tuple-destr-error.js:16:5: error: ft: cannot destructure tuple, expected 2 elements, found 1
// CHECK-NEXT:let [v1] = tup;
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}tuple-destr-error.js:17:5: error: ft: cannot destructure tuple, expected 2 elements, found 3
// CHECK-NEXT:let [v2, v3, v4] = tup;
// CHECK-NEXT:    ^~~~~~~~~~~~
// CHECK-NEXT:{{.*}}tuple-destr-error.js:22:1: error: ft: incompatible assignment types
// CHECK-NEXT:[a, b] = tup;
// CHECK-NEXT:^~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
