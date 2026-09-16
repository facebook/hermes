/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen %s --match-full-lines

// Mirror of destr-param-error.js for arrows: destructuring params require a type
// annotation when no constraint determines the parameter type (these consts
// have no contextual type). When a constraint is present, the annotation may be
// omitted; see destr-param-arrow-infer.js.

'use strict';

// Missing annotation on destructuring param - should error.
const bad = ({x}) => {};

// Missing annotation on array destructuring param - should error.
const bad2 = ([a, b]) => {};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}destr-param-arrow-error.js:18:14: error: ft: destructuring parameters must have a type annotation
// CHECK-NEXT:const bad = ({x}) => {};
// CHECK-NEXT:             ^~~
// CHECK-NEXT:{{.*}}destr-param-arrow-error.js:21:15: error: ft: destructuring parameters must have a type annotation
// CHECK-NEXT:const bad2 = ([a, b]) => {};
// CHECK-NEXT:              ^~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
