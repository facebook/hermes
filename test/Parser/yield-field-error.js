/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function* foo() {
  class C {
    x = yield;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}yield-field-error.js:12:9: error: invalid expression
// CHECK-NEXT:    x = yield;
// CHECK-NEXT:        ^
