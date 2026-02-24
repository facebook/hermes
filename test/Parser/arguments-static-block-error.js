/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

function* foo() {
  class C {
    static {
      print(arguments.length);
// CHECK:{{.*}}arguments-static-block-error.js:13:13: error: invalid use of 'arguments' as an identifier
// CHECK-NEXT:      print(arguments.length);
// CHECK-NEXT:            ^~~~~~~~~
    }
  }
}

class GlobalClass {
  static {
    print(arguments.length);
// CHECK:{{.*}}arguments-static-block-error.js:23:11: error: invalid use of 'arguments' as an identifier
// CHECK-NEXT:    print(arguments.length);
// CHECK-NEXT:          ^~~~~~~~~
  }
}
