/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

function* foo() {
  class C {
    static {
      yield 1;
    }
  }
}

// CHECK:{{.*}}class-static-block-yield-error.js:13:7: error: invalid expression
// CHECK-NEXT:    yield 1;
// CHECK-NEXT:    ^
