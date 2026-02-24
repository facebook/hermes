/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

let undefined;
let NaN;
let Infinity;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}restricted-global-error.js:10:5: error: Can't create duplicate variable that shadows a global property: 'undefined'
// CHECK-NEXT:let undefined;
// CHECK-NEXT:    ^~~~~~~~~
// CHECK-NEXT:{{.*}}restricted-global-error.js:11:5: error: Can't create duplicate variable that shadows a global property: 'NaN'
// CHECK-NEXT:let NaN;
// CHECK-NEXT:    ^~~
// CHECK-NEXT:{{.*}}restricted-global-error.js:12:5: error: Can't create duplicate variable that shadows a global property: 'Infinity'
// CHECK-NEXT:let Infinity;
// CHECK-NEXT:    ^~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
