/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

async function foo() {
  class C {
    x = await 1;
  }

  class D {
    x = class { [await 1] = 1; };
  }
}


// CHECK:{{.*}}await-field-error.js:12:9: error: 'await' not in an async function
// CHECK-NEXT:    x = await 1;
// CHECK-NEXT:        ^~~~~~~
// CHECK-NEXT:{{.*}}await-field-error.js:16:18: error: 'await' not in an async function
// CHECK-NEXT:    x = class { [await 1] = 1; };
// CHECK-NEXT:                 ^~~~~~~
