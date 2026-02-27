/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheckOrRegen --match-full-lines %s

class A {
  x: string = 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}field-inits-error.js:11:3: error: ft: incompatible initialization type
// CHECK-NEXT:  x: string = 1;
// CHECK-NEXT:  ^~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
