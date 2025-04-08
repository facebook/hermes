/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

class A {
  static #name = 12;
  static f1 = 13;
  static {
    debugger;
  }
  static f2 = 14;
  static {
    debugger;
  }
}
// CHECK: Break on 'debugger' statement in <A:static_block_0>: {{.*}}:15:5
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: undefined
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in <A:static_block_1>: {{.*}}:19:5
// CHECK-NEXT: 14
// CHECK-NEXT: Continuing execution
