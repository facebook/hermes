/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

var x: [number, string] = [1, 'a'];
x.a;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}tuple-named-error.js:13:3: error: ft: unknown tuple property
// CHECK-NEXT:x.a;
// CHECK-NEXT:  ^
// CHECK-NEXT:Emitted 1 errors. exiting.
