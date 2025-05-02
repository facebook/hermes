/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheck --match-full-lines %s

class C {
  f1 = arguments;
// CHECK:{{.*}}field-value-arguments-error.js:11:8: error: invalid use of 'arguments'
// CHECK-NEXT:  f1 = arguments;
// CHECK-NEXT:       ^~~~~~~~~
  f2 = () => arguments;
// CHECK-NEXT:{{.*}}field-value-arguments-error.js:15:14: error: invalid use of 'arguments'
// CHECK-NEXT:  f2 = () => arguments;
// CHECK-NEXT:             ^~~~~~~~~
  #f1 = arguments;
// CHECK-NEXT:{{.*}}field-value-arguments-error.js:19:9: error: invalid use of 'arguments'
// CHECK-NEXT:  #f1 = arguments;
// CHECK-NEXT:        ^~~~~~~~~
  #f2 = () => arguments;
// CHECK-NEXT:{{.*}}field-value-arguments-error.js:23:15: error: invalid use of 'arguments'
// CHECK-NEXT:  #f2 = () => arguments;
// CHECK-NEXT:              ^~~~~~~~~
}
