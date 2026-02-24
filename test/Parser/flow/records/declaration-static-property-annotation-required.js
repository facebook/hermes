/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

// CHECK: {{.*}}declaration-static-property-annotation-required.js:15:13: error: expected ':' for property, '(' for method, or '<' for method with type parameters
// CHECK-NEXT:   static foo,
// CHECK-NEXT:             ^

record R {
  static foo,
}
