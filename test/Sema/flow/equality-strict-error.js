/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class A {}
class B {}

function main(
    n: number,
    s: string,
    a: A,
    b: B,
) {
  "a" === 1;
  1 !== "a";
  n === s;
  s !== n;
  a === b;
  a !== b;
  a === undefined;
  a !== null;
  n === null;
  n === undefined;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}equality-strict-error.js:19:3: error: ft: === cannot be applied to string and number
// CHECK-NEXT:  "a" === 1;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:20:3: error: ft: !== cannot be applied to number and string
// CHECK-NEXT:  1 !== "a";
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:21:3: error: ft: === cannot be applied to number and string
// CHECK-NEXT:  n === s;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:22:3: error: ft: !== cannot be applied to string and number
// CHECK-NEXT:  s !== n;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:23:3: error: ft: === cannot be applied to class A and class B
// CHECK-NEXT:  a === b;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:24:3: error: ft: !== cannot be applied to class A and class B
// CHECK-NEXT:  a !== b;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:25:3: error: ft: === cannot be applied to class A and void
// CHECK-NEXT:  a === undefined;
// CHECK-NEXT:  ^~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:26:3: error: ft: !== cannot be applied to class A and null
// CHECK-NEXT:  a !== null;
// CHECK-NEXT:  ^~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:27:3: error: ft: === cannot be applied to number and null
// CHECK-NEXT:  n === null;
// CHECK-NEXT:  ^~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-strict-error.js:28:3: error: ft: === cannot be applied to number and void
// CHECK-NEXT:  n === undefined;
// CHECK-NEXT:  ^~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 10 errors. exiting.
