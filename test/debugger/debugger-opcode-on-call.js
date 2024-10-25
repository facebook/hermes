/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function fn() {
  print('fn');
}

function test() {
  fn();
}

debugger;
test();

// CHECK: Break on 'debugger' statement in global: {{.*}}:19:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:16:5
// CHECK-NEXT: Continuing execution

// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: Continuing execution
