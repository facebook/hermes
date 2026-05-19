/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class C {
  x: number = 0;
}
let c: C = new C();
// Spreading a non-exact-object typed value is an error.
let bad1: {x: number} = {...c};

let src: {x: string} = {x: "hi"};
// Spreading a field whose type is incompatible with the destination.
let bad2: {x: number} = {...src};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}exact-object-spread-error.js:15:5: error: ft: incompatible initialization type: cannot assign object to object
// CHECK-NEXT:let bad1: {x: number} = {...c};
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}exact-object-spread-error.js:15:26: error: ft: spread argument must be an exact object type
// CHECK-NEXT:let bad1: {x: number} = {...c};
// CHECK-NEXT:                         ^~~~
// CHECK-NEXT:{{.*}}exact-object-spread-error.js:19:5: error: ft: incompatible initialization type: cannot assign object to object
// CHECK-NEXT:let bad2: {x: number} = {...src};
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
