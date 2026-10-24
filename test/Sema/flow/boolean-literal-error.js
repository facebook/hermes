/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

const t: true = false;
const c = true;
const d: false = c;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}boolean-literal-error.js:12:7: error: ft: incompatible initialization type: cannot assign false to true
// CHECK-NEXT:const t: true = false;
// CHECK-NEXT:      ^~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}boolean-literal-error.js:14:7: error: ft: incompatible initialization type: cannot assign true to false
// CHECK-NEXT:const d: false = c;
// CHECK-NEXT:      ^~~~~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
