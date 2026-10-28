/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheckOrRegen --match-full-lines %s

class A {
  x: string = 1;
  #y: number = 's';
  static #z: number = 's';
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}field-inits-error.js:11:3: error: ft: incompatible initialization type
// CHECK-NEXT:  x: string = 1;
// CHECK-NEXT:  ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}field-inits-error.js:12:3: error: ft: incompatible initialization type
// CHECK-NEXT:  #y: number = 's';
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}field-inits-error.js:13:10: error: ft: incompatible initialization type
// CHECK-NEXT:  static #z: number = 's';
// CHECK-NEXT:         ^~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
