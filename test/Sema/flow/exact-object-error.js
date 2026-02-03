/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function foo(t: {x: number}): void {
  let a: {y: number} = t;
  let b: {x: string} = t;
  let c: {x: number, y: string} = t;
  let d: {x: number} = {x: "hi"};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}exact-object-error.js:11:7: error: ft: incompatible initialization type
// CHECK-NEXT:  let a: {y: number} = t;
// CHECK-NEXT:      ^~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}exact-object-error.js:12:7: error: ft: incompatible initialization type
// CHECK-NEXT:  let b: {x: string} = t;
// CHECK-NEXT:      ^~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}exact-object-error.js:13:7: error: ft: incompatible initialization type
// CHECK-NEXT:  let c: {x: number, y: string} = t;
// CHECK-NEXT:      ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}exact-object-error.js:14:28: error: ft: incompatible property type for 'x'
// CHECK-NEXT:  let d: {x: number} = {x: "hi"};
// CHECK-NEXT:                           ^~~~
// CHECK-NEXT:Emitted 4 errors. exiting.
