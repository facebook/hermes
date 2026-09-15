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
    foo: A,
) {
  1 == 2;
  "a" != "b";
  n == s;
  s != n;
  a == b;
  a != b;
  foo == null;
  foo != undefined;
  n == null;
  n != undefined;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}equality-loose-error.js:20:3: error: ft: == cannot be applied to number and number (use === / !== for general comparisons)
// CHECK-NEXT:  1 == 2;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:21:3: error: ft: != cannot be applied to string and string (use === / !== for general comparisons)
// CHECK-NEXT:  "a" != "b";
// CHECK-NEXT:  ^~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:22:3: error: ft: == cannot be applied to number and string (use === / !== for general comparisons)
// CHECK-NEXT:  n == s;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:23:3: error: ft: != cannot be applied to string and number (use === / !== for general comparisons)
// CHECK-NEXT:  s != n;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:24:3: error: ft: == cannot be applied to class A and class B (use === / !== for general comparisons)
// CHECK-NEXT:  a == b;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:25:3: error: ft: != cannot be applied to class A and class B (use === / !== for general comparisons)
// CHECK-NEXT:  a != b;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:26:3: error: ft: == cannot be applied to class A and null (use === / !== for general comparisons)
// CHECK-NEXT:  foo == null;
// CHECK-NEXT:  ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:27:3: error: ft: != cannot be applied to class A and void (use === / !== for general comparisons)
// CHECK-NEXT:  foo != undefined;
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:28:3: error: ft: == cannot be applied to number and null (use === / !== for general comparisons)
// CHECK-NEXT:  n == null;
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}equality-loose-error.js:29:3: error: ft: != cannot be applied to number and void (use === / !== for general comparisons)
// CHECK-NEXT:  n != undefined;
// CHECK-NEXT:  ^~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 10 errors. exiting.
