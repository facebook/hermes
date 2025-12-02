/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

// CHECK: {{.*}}declaration-invalid-async-property.js:15:9: error: invalid async/generator modifier for record property, expected a method definition
// CHECK-NEXT:   async foo: string,
// CHECK-NEXT:         ^~~

record R {
  async foo: string,
}
