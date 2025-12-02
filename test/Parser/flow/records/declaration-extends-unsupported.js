/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

// CHECK: {{.*}}declaration-extends-unsupported.js:14:10: error: '{' expected in record declaration
// CHECK-NEXT: record R extends C {
// CHECK-NEXT:          ~~~~~~~~~^

record R extends C {
  a: number,
}
