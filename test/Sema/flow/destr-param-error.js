/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Missing annotation on destructuring param - should error.
function bad({x}) {}

// Missing annotation on array destructuring param - should error.
function bad2([a, b]) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}destr-param-error.js:13:14: error: ft: destructuring parameters must have a type annotation
// CHECK-NEXT:function bad({x}) {}
// CHECK-NEXT:             ^~~
// CHECK-NEXT:{{.*}}destr-param-error.js:16:15: error: ft: destructuring parameters must have a type annotation
// CHECK-NEXT:function bad2([a, b]) {}
// CHECK-NEXT:              ^~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
