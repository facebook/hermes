/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

var s: string = "hello";
s.foo;
s["bad"];

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}string-property-error.js:13:3: error: ft: unknown string property
// CHECK-NEXT:s.foo;
// CHECK-NEXT:  ^~~
// CHECK-NEXT:{{.*}}string-property-error.js:14:3: error: ft: string index must be a number
// CHECK-NEXT:s["bad"];
// CHECK-NEXT:  ^~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
