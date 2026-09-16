/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

var n: number = 1;
n.nosuch();
n.length;
n[0];

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}number-property-error.js:13:3: error: ft: unknown number property
// CHECK-NEXT:n.nosuch();
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}number-property-error.js:14:3: error: ft: unknown number property
// CHECK-NEXT:n.length;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}number-property-error.js:15:3: error: ft: indexed access only allowed on array/tuple/string, found number
// CHECK-NEXT:n[0];
// CHECK-NEXT:  ^
// CHECK-NEXT:Emitted 3 errors. exiting.
