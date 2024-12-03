/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function* foo() {
  class C {
    x = () => arguments;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}arguments-field-error.js:12:15: error: invalid use of 'arguments'
// CHECK-NEXT:    x = () => arguments;
// CHECK-NEXT:              ^~~~~~~~~
