/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen %s --match-full-lines

class A {
  static {
    let x;
    let x;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}static-initialization-block-error.js:13:9: error: Identifier 'x' is already declared
// CHECK-NEXT:    let x;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}static-initialization-block-error.js:12:9: note: previous declaration
// CHECK-NEXT:    let x;
// CHECK-NEXT:        ^
// CHECK-NEXT:Emitted 1 errors. exiting.
