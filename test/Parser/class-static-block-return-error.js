/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

class C {
  static {
    return 1;
  }
}
function foo() {
  class C {
    static {
      return 1;
    }
  }
}

// CHECK:{{.*}}class-static-block-return-error.js:12:5: error: 'return' not in a function
// CHECK-NEXT:    return 1;
// CHECK-NEXT:    ^~~~~~
// CHECK:{{.*}}class-static-block-return-error.js:18:7: error: 'return' not in a function
// CHECK-NEXT:    return 1;
// CHECK-NEXT:    ^~~~~~
