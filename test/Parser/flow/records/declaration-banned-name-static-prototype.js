/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

// CHECK: {{.*}}declaration-banned-name-static-prototype.js:15:10: error: invalid record property name
// CHECK-NEXT:   static prototype: {} = {},
// CHECK-NEXT:          ^~~~~~~~~

record R {
  static prototype: {} = {},
}
