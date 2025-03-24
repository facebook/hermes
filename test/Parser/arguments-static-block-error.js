/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function* foo() {
  class C {
    static {
      print(arguments.length);
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}arguments-static-block-error.js:12:12: error: class static blocks are not supported
// CHECK-NEXT:    static {
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}arguments-static-block-error.js:13:13: error: invalid use of 'arguments'
// CHECK-NEXT:      print(arguments.length);
// CHECK-NEXT:            ^~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
