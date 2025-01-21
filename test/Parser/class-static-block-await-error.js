/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

async function foo() {
  // This is fine.
  class C extends (await 1) {
    static {
      // This is not fine.
      await 1;
    }
  }
}

// CHECK:{{.*}}class-static-block-await-error.js:15:7: error: 'await' not in an async function
// CHECK-NEXT:    await 1;
// CHECK-NEXT:    ^~~~~~~
